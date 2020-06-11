#include "citymobileapi.h"
const QString c_baseUrl="https://terminal.api.dev.fuelup.ru/"; //https://terminal.api.fuelup.ru/

CityMobileAPI::CityMobileAPI(QString aBazeUrl, QString aApiKey, QSqlDatabase aDb, QObject *parent) : QObject(parent), _bazeUrl(aBazeUrl), _request(new RequestData()), _apiKey(aApiKey), _db(aDb) {

}

CityMobileAPI::~CityMobileAPI() {
    delete _request;
}

void CityMobileAPI::SetAZSData(QJsonDocument aPost) {
    qDebug()<<"Data"<<aPost;
    QNetworkRequest request = createRequest("api/station", "application/json", true);
    _request->post(request,QString(aPost.toJson(QJsonDocument::Compact)).toUtf8());
}

void CityMobileAPI::SetPriceList(QString aPost) {
    qDebug()<<"Price"<<aPost;
    QNetworkRequest request = createRequest("api/price", "application/x-www-form-urlencoded", true);
    _request->post(request,aPost.toUtf8());
}

void CityMobileAPI::GetRequests() {
    QNetworkRequest request = createRequest("api/orders/items", "application/x-www-form-urlencoded", true);
    _request->get(request);
    connect(_request, &RequestData::s_finished, this, &CityMobileAPI::checkOrders);
}

void CityMobileAPI::SetRequestStateAccept(QString aOrderId) {
    QNetworkRequest request = createRequest("api/orders/accept?orderId=" + aOrderId, "application/x-www-form-urlencoded", true);
    _request->get(request);
}

void CityMobileAPI::SetRequestStateFueling(QString aOrderId) {
    QNetworkRequest request = createRequest("api/orders/fueling?orderId=" + aOrderId, "application/x-www-form-urlencoded", true);
    _request->get(request);
}

void CityMobileAPI::SetRequestStateCanceled(QString aOrderId, QString aReason, QString aExtendedOrderId, QDateTime aExtendedDate) {
    QNetworkRequest request = createRequest(QString("api/orders/canceled?orderId=%&reason=%2&extendedOrderId=%3&extendedDate=%4").arg(
                                                aOrderId,
                                                aReason,
                                                aExtendedOrderId,
                                                aExtendedDate.toString("dd.MM.yyyy HH:mm:ss")), "application/x-www-form-urlencoded", true);
    _request->get(request);
}

void CityMobileAPI::SetRequestStateCompleted(QString aOrderId, double aLitre, QString aExtendedOrderId, QDateTime aExtendedDate) {
    QNetworkRequest request = createRequest(QString("api/orders/completed?orderId=%1&litre=%2&extendedOrderId=%3&extendedDate=%4").arg(
                                                aOrderId,
                                                QString::number(aLitre).replace(",","."),
                                                aExtendedOrderId,
                                                aExtendedDate.toString("dd.MM.yyyy HH:mm:ss")), "application/x-www-form-urlencoded", true);
    _request->get(request);
}

void CityMobileAPI::SetRequestStateFuelNow(QString aOrderId, double aLitre) {
    QNetworkRequest request = createRequest("api/orders/volume", "application/x-www-form-urlencoded", true);
    _request->post(request,QString("orderId=" + aOrderId + "&litre=" + QString::number(aLitre).replace(",",".")).toUtf8());
}

void CityMobileAPI::SetRequestMessage(QString aOrderId, QString aMessage) {
    QNetworkRequest request = createRequest("api/orders/message", "application/x-www-form-urlencoded", true);
    _request->post(request,QString("orderId=" + aOrderId + "&msg=" + aMessage).toUtf8());
}

CityMobileAPI::CityMobileAPI(const CityMobileAPI &aApi): QObject(aApi.parent()), _bazeUrl(aApi._bazeUrl), _request(new RequestData()), _apiKey(aApi._apiKey) {

}

CityMobileAPI &CityMobileAPI::operator=(const CityMobileAPI&) {
    return *this;
}

QNetworkRequest CityMobileAPI::createRequest(QString aUrl, QString aContentType, bool aAuth) {
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl + aUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader,aContentType);
    if (aAuth) {
        request.setRawHeader("Authorization",_apiKey.toUtf8());
    }
    return request;
}


#define SecondRequests {
void CityMobileAPI::checkOrders(RequestData*) {
    disconnect(_request, &RequestData::s_finished, this, &CityMobileAPI::checkOrders);
    QJsonArray orders = QJsonDocument::fromJson(_request->getAnswer()).object().value("Orders").toArray();
    qDebug()<<"Requests"<<orders;
    for(auto order: orders) {
        QSqlQuery *qRequests = new QSqlQuery(_db);
        qRequests->exec("SELECT ApiId "
                        "FROM [agzs].[dbo].[PR_APITransaction] "
                        "WHERE ApiId = '"+order.toObject().value("Id").toString()+"'");
        if (qRequests->size() == 0) {
            insertNewRequest(order.toObject());
        }
        delete qRequests;
    }
}

void CityMobileAPI::insertNewRequest(QJsonObject aRequest) {
//            {
//            "Id": "088ccc98-73e3-4b4a-aaa9-79dddd02720a",
//            "DateCreate" :  "2019-11-20T17:28:33.994Z",
//            "OrderType":  "Litre",
//            "OrderVolume": "10",
//            "StationId":  "1234",
//            "StationExtendedId":  "1234",
//            "ColumnId":  "1",
//            "FuelId":  "a92",
//            "PriceFuel":  "42.50",
//            "Litre":  "10",
//            "Sum":  "425.00",
//            "Status":  "OrderCreated",
//            "ContractId":  "32135"
//            }

//    1. ColumnId – в случае, если идентификатор колонки не найден, либо колонка отключена, АСУ АЗС должна сообщить статут Canceled с указанием соответствующей причины отмены.
//    2. FuelId – в случае, если на указанной колонке заданное в заказе топливо недоступно, АСУ АЗС должна сообщить статут Canceled с указанием соответствующей причины отмены.
//    3. PriceFuel – в случае, если стоимость топлива в интегрируемой систем отличается от присланной, АСУ АЗС должна сообщить статут Canceled с указанием соответствующей причины отмены.
    QDateTime now = QDateTime::currentDateTime();
    int lastAPIVCode = getLastAPI();
    if (lastAPIVCode > 0) {
        int error = checkError(aRequest.value("StationId").toString(),
                               aRequest.value("ColumnId").toString(),
                               aRequest.value("FuelId").toString(),
                               QString::number(aRequest.value("PriceFuel").toInt()),
                               aRequest.value("Id").toString(),
                               QString::number(lastAPIVCode+1),
                               now);
        QString fuelFullName = getFullFuelName(getFuelID(aRequest.value("FuelId").toString()));

        QSqlQuery *qAgzsColumn = new QSqlQuery(_db);
        qAgzsColumn->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id,  c.TrkType, c.[DeviceName], c.[Serial], c.[FuelName], c.[FuelShortName], "
                            "c.[Side], c.[SideAdress], c.[Nozzle], c.[TrkFuelCode], c.[TrkVCode] "
                           "FROM [agzs].[dbo].PR_AGZSData d "
                                "INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c "
                                "ON d.VCode = c.Link "
                                "INNER JOIN [agzs].[dbo].pr_agzsPrice p "
                                "ON d.VCode = p.Link "
                           "WHERE d.Id=" + aRequest.value("StationId").toString() + " and c.SideAdress=" + aRequest.value("ColumnId").toString() +
                            " and c.FuelVCode=" + QString::number(getFuelID(aRequest.value("FuelId").toString())) +
                            " and p.[" + aRequest.value("FuelId").toString() + "]=" + QString::number(aRequest.value("PriceFuel").toInt()) + " "
                           "ORDER BY d.CDate DESC");
        if (qAgzsColumn->next()) {
            int transactionVCode = -1;
            if (error == 0) {
                double requestTotalPriceDB=-1,	requestVolumeDB=-1,	requestUnitPriceDB=-1, moneyTakenDB=-1;
                int fullTankDB=-1;
                getMoneyData(aRequest, requestTotalPriceDB, requestVolumeDB, requestUnitPriceDB, moneyTakenDB, fullTankDB);
                QSqlQuery *qTransaction = new QSqlQuery(_db);
                qTransaction->exec("SELECT TOP 1 Value "
                                    "FROM [agzs].[dbo].[LxKeysOfCodes] "
                                    "WHERE [Key] = 'ADAST_TRKTransaction'");
                if (qTransaction->next()) {
                    transactionVCode = qTransaction->value(0).toInt() + 1;
                    qTransaction->exec("UPDATE [agzs].[dbo].[LxKeysOfCodes] "
                                        "SET Value = " + QString::number(transactionVCode)+" "
                                        "WHERE [Key] = 'ADAST_TRKTransaction'");
                    qTransaction->prepare("INSERT INTO [agzs].[dbo].[ADAST_TRKTransaction] (AGZSName, LocalVCode, TrkType, DeviceName,	Serial,	FuelName, FuelShortName, Side, SideAddress, Nozzle, "
                                   "TrkFuelCode, TransNum, TrkTotalPriceDB, TrkVolumeDB, TrkUnitPriceDB, RequestTotalPriceDB, RequestVolumeDB, RequestUnitPriceDB, RequestField, State, iState, "
                                   "TrkTransType, LitersCountBeforeDB, MoneyCountBeforeDB, TransCountBefore, LitersCountAfterDB, MoneyCountAfterDB, TransCountAfter, Result, DateOpen, DateClose, "
                                   "TemperatureDB, PayOperationVCode, PayWay, PrePostPay, WUser, WDate, CUser, CDate, CHost, WHost, VCode, AddedForTransVCode, AditionalTransVCode, ActiveDB, "
                                   "MassDB, Smena, TrkVcode, CapacityVcode, PumpPlace, MoneyTakenDB, iPayWay, AutoCheckDB, Closed, FullTankDB, AGZS, FuelVCode, rowguid, Propan) "
                                   "VALUES(:AGZSName, :LocalVCode, :TrkType, :DeviceName, :Serial, :FuelName, :FuelShortName, :Side, :SideAddress, :Nozzle, :TrkFuelCode, :TransNum, "
                                   ":TrkTotalPriceDB, :TrkVolumeDB, :TrkUnitPriceDB, :RequestTotalPriceDB, :RequestVolumeDB, :RequestUnitPriceDB, :RequestField, :State, :iState, :TrkTransType, "
                                   ":LitersCountBeforeDB, :MoneyCountBeforeDB, :TransCountBefore, :LitersCountAfterDB, :MoneyCountAfterDB, :TransCountAfter, :Result, :DateOpen, :DateClose, "
                                   ":TemperatureDB, :PayOperationVCode, :PayWay, :PrePostPay, :WUser, :WDate, :CUser, :CDate, :CHost, :WHost, :VCode, :AddedForTransVCode, :AditionalTransVCode, "
                                   ":ActiveDB, :MassDB, :Smena, :TrkVcode, :CapacityVcode, :PumpPlace, :MoneyTakenDB, :iPayWay, :AutoCheckDB, :Closed, :FullTankDB, :AGZS, :FuelVCode, DEFAULT, "
                                   ":Propan)");
                    qTransaction->bindValue(":AGZSName", qAgzsColumn->value(0).toString());
                    qTransaction->bindValue(":LocalVCode", QString::number(transactionVCode));
                    qTransaction->bindValue(":TrkType", qAgzsColumn->value(4).toString());
                    qTransaction->bindValue(":DeviceName", qAgzsColumn->value(5).toString());
                    qTransaction->bindValue(":Serial", qAgzsColumn->value(6).toString());
                    qTransaction->bindValue(":FuelName", qAgzsColumn->value(7).toString());
                    qTransaction->bindValue(":FuelShortName", qAgzsColumn->value(8).toString());
                    qTransaction->bindValue(":Side", qAgzsColumn->value(9).toString());
                    qTransaction->bindValue(":SideAddress", qAgzsColumn->value(10).toString());
                    qTransaction->bindValue(":Nozzle", qAgzsColumn->value(11).toString());
                    qTransaction->bindValue(":TrkFuelCode", qAgzsColumn->value(12).toString());
                    qTransaction->bindValue(":TransNum", "");
                    qTransaction->bindValue(":TrkTotalPriceDB", 0);
                    qTransaction->bindValue(":TrkVolumeDB", 0);
                    qTransaction->bindValue(":TrkUnitPriceDB", 0);
                    qTransaction->bindValue(":RequestTotalPriceDB", requestTotalPriceDB);
                    qTransaction->bindValue(":RequestVolumeDB", requestVolumeDB);
                    qTransaction->bindValue(":RequestUnitPriceDB", requestUnitPriceDB);
                    qTransaction->bindValue(":RequestField", "V");
                    qTransaction->bindValue(":State", "Завершение выдачи");
                    qTransaction->bindValue(":iState", 8);
                    qTransaction->bindValue(":TrkTransType", "P");
                    qTransaction->bindValue(":LitersCountBeforeDB", 0);
                    qTransaction->bindValue(":MoneyCountBeforeDB", 0);
                    qTransaction->bindValue(":TransCountBefore", 0);
                    qTransaction->bindValue(":LitersCountAfterDB", 0);
                    qTransaction->bindValue(":MoneyCountAfterDB", 0);
                    qTransaction->bindValue(":TransCountAfter", 0);
                    qTransaction->bindValue(":Result", "Выдача завершена: 1");
                    qTransaction->bindValue(":DateOpen", now.toString("yyyyMMdd hh:mm:ss.zzz"));
                    qTransaction->bindValue(":DateClose", now.toString("yyyyMMdd hh:mm:ss.zzz"));
                    qTransaction->bindValue(":TemperatureDB", -100);
                    qTransaction->bindValue(":PayOperationVCode", 0);
                    qTransaction->bindValue(":PayWay", "СИТИМОБИЛ");
                    qTransaction->bindValue(":PrePostPay", 0);
                    qTransaction->bindValue(":WUser", "SERVER");
                    qTransaction->bindValue(":WDate", now.toString("yyyyMMdd hh:mm:ss.zzz"));
                    qTransaction->bindValue(":CUser", "SERVER");
                    qTransaction->bindValue(":CDate", now.toString("yyyyMMdd hh:mm:ss.zzz"));
                    qTransaction->bindValue(":CHost", "SERVER");
                    qTransaction->bindValue(":WHost", "SERVER");
                    qTransaction->bindValue(":VCode", transactionVCode);
                    qTransaction->bindValue(":AddedForTransVCode", 0);
                    qTransaction->bindValue(":AditionalTransVCode", 0);
                    qTransaction->bindValue(":ActiveDB", 0);
                    qTransaction->bindValue(":MassDB", 0);
                    qTransaction->bindValue(":Smena", getSmena());
                    qTransaction->bindValue(":TrkVcode", qAgzsColumn->value(13).toString());
                    qTransaction->bindValue(":CapacityVcode", getCashBoxIndex());
                    qTransaction->bindValue(":PumpPlace", qAgzsColumn->value(10).toString());
                    qTransaction->bindValue(":MoneyTakenDB", moneyTakenDB);
                    qTransaction->bindValue(":iPayWay", 10);
                    qTransaction->bindValue(":AutoCheckDB", 0);
                    qTransaction->bindValue(":Closed", 0);
                    qTransaction->bindValue(":FullTankDB", fullTankDB);
                    qTransaction->bindValue(":AGZS", qAgzsColumn->value(1).toString());
                    qTransaction->bindValue(":FuelVCode", getFuelID(aRequest.value("FuelId").toString()));
                    qTransaction->bindValue(":Propan", 0);
                    qTransaction->exec();
                    if(qTransaction->next()) {
                        SetRequestStateAccept(aRequest.value("Id").toString());
                    }
                }
                delete qTransaction;
            }
            QSqlQuery *qApiRequests = new QSqlQuery(_db);
            qApiRequests->prepare("INSERT INTO [agzs].[dbo].[PR_APITransaction] ([AGZSName],[AGZS],[CDate],[VCode],[APIID],[APIStationExtendedId],[APIColumnId],[APIFuelId],[FuelId],"
                        "[APIPriceFuel],[APILitre],[APISum],[APIStatus],[APIContractId],[agent],[localState],[Price],[Sum],[DateOpen],[DateClose],[Link]) "
                        "VALUES (:AGZSName, :AGZS, :CDate, :VCode, :APIID, :APIStationExtendedId, :APIColumnId, :APIFuelId, :FuelId, :APIPriceFuel, :APILitre, :APISum, "
                        ":APIStatus, :APIContractId, :agent, :localState, :Price, :Sum, :DateOpen, DEFAULT, :Link)");
            qApiRequests->bindValue(":AGZSName", qAgzsColumn->value(0).toString());
            qApiRequests->bindValue(":AGZS", qAgzsColumn->value(1).toString());
            qApiRequests->bindValue(":CDate", QDateTime::fromString(aRequest.value("DateCreate").toString(),"yyyy-MM-ddThh:mm:ss.zzzZ").toString("yyyy-MM-dd hh:mm:ss.zzz"));
            qApiRequests->bindValue(":VCode", lastAPIVCode + 1);
            qApiRequests->bindValue(":APIID", aRequest.value("Id").toString());
            qApiRequests->bindValue(":APIStationExtendedId", aRequest.value("StationExtendedId").toString());
            qApiRequests->bindValue(":APIColumnId", aRequest.value("ColumnId").toString());
            qApiRequests->bindValue(":APIFuelId", aRequest.value("FuelId").toString());
            qApiRequests->bindValue(":FuelId", getFuelID(aRequest.value("FuelId").toString()));
            qApiRequests->bindValue(":APIPriceFuel", aRequest.value("PriceFuel").toString());
            qApiRequests->bindValue(":APILitre", aRequest.value("Litre").toString());
            qApiRequests->bindValue(":APISum", aRequest.value("Sum").toString());
            qApiRequests->bindValue(":APIStatus", aRequest.value("Status").toString());
            qApiRequests->bindValue(":APIContractId", aRequest.value("ContractId").toString());
            qApiRequests->bindValue(":agent", "CityMobile");
            qApiRequests->bindValue(":localState", error != 0 ? "Error: " + QString::number(error) : "0");
            qApiRequests->bindValue(":Price", 0);
            qApiRequests->bindValue(":Sum", 0);
            qApiRequests->bindValue(":DateOpen", now);
            //qApiRequests->bindValue(":DateClose", "DEFAULT");
            qApiRequests->bindValue(":Link", transactionVCode);
            qApiRequests->exec();
            delete qApiRequests;
        }
        delete qAgzsColumn;
    }
}

int CityMobileAPI::checkError(QString aStationID, QString aColumnID, QString aFuelID, QString aPriceFuel, QString aId, QString aLastVCode, QDateTime aNow) {
    QSqlQuery *q_Check = new QSqlQuery(_db);
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], "
                    "c.[TrkFuelCode], c.[TrkVCode] "
                  "FROM [agzs].[dbo].PR_AGZSData d "
                    "INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c "
                    "ON d.VCode = c.Link "
                  "WHERE d.AGZS = "+aStationID+" and c.SideAdress = "+aColumnID+" "
                  "ORDER BY d.CDate DESC");
    if (q_Check->size() == 0) {
        SetRequestStateCanceled(aId,"Указанная колонка не найдена.",aLastVCode,aNow);
        delete q_Check;
        qDebug()<<"Error1";
        return 1;
    }
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], "
                    "c.[TrkFuelCode], c.[TrkVCode] "
                  "FROM [agzs].[dbo].PR_AGZSData d "
                    "INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c "
                    "ON d.VCode = c.Link "
                  "WHERE d.Id='"+aStationID+"' and c.SideAdress = "+aColumnID+" and c.FuelVCode = "+QString::number(getFuelID(aFuelID))+" "
                  "ORDER BY d.CDate DESC");
    if (q_Check->size() == 0) {
        SetRequestStateCanceled(aId,"Не обнаружено указанное топливо.",aLastVCode,aNow);
        delete q_Check;
        qDebug()<<"Error2";
        return 2;
    }
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], "
                    "c.[TrkFuelCode], c.[TrkVCode], p.["+aFuelID+"] "
                "FROM [agzs].[dbo].PR_AGZSData d "
                     "INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c "
                     "ON d.VCode = c.Link "
                     "INNER JOIN [agzs].[dbo].PR_AGZSPrice p "
                     "ON d.VCode = p.Link "
                "WHERE d.Id='"+aStationID+"' and c.SideAdress = "+aColumnID+" and c.FuelVCode = "+QString::number(getFuelID(aFuelID))+
                    " and p.["+aFuelID+"]="+aPriceFuel+" "
                "ORDER BY d.CDate DESC");
    if (q_Check->size() == 0) {
        SetRequestStateCanceled(aId,"Цена на выбранный вид топлива отличается от фактической цены.",aLastVCode,aNow);
        delete q_Check;
        qDebug()<<"Error3";
        return 3;
    }
    return 0;
}

void CityMobileAPI::getMoneyData(QJsonObject aData, double &aRequestTotalPriceDB, double &aRequestVolumeDB, double &aRequestUnitPriceDB, double &aMoneyTakenDB, int &aFullTankDB) {
    if(aData.value("OrderType").toString() == "Money"){
        aRequestTotalPriceDB = aData.value("OrderVolume").toDouble();
        aRequestUnitPriceDB = aData.value("PriceFuel").toDouble();
        aRequestVolumeDB = aRequestTotalPriceDB / aRequestUnitPriceDB;
        aMoneyTakenDB = aData.value("OrderVolume").toDouble();
        aFullTankDB = 0;
    } else if (aData.value("OrderType").toString() == "Liters") {
        aRequestVolumeDB = aData.value("OrderVolume").toDouble();
        aRequestUnitPriceDB = aData.value("PriceFuel").toDouble();
        aRequestTotalPriceDB = aRequestVolumeDB * aRequestUnitPriceDB;
        aMoneyTakenDB = aData.value("OrderVolume").toDouble();
        aFullTankDB = 0;
    } else if (aData.value("OrderType").toString() == "FullTank") {
        aRequestTotalPriceDB = aData.value("OrderVolume").toDouble();
        aRequestUnitPriceDB = aData.value("PriceFuel").toDouble();
        aRequestVolumeDB = aRequestTotalPriceDB / aRequestUnitPriceDB;
        aMoneyTakenDB = aData.value("OrderVolume").toDouble();
        aFullTankDB = 1;
    }
}
#define SecondRequestsEnd }

#define GetDataStart {
int CityMobileAPI::getCashBoxIndex() {
    QSqlQuery *qCashBox = new QSqlQuery(_db);
    qCashBox->exec("SELECT TOP 1 CashBoxIndex, PayIndex, PayWay, AutoCheck "
                   "FROM [agzs].[dbo].[ARM_CashBoxesSemaphor] "
                   "WHERE PayIndex=10 and PayWay='СИТИМОБИЛ' and AutoCheck=0 "
                   "ORDER BY CDate DESC");
    qCashBox->next();
    int last = qCashBox->value(0).toInt();
    delete qCashBox;
    return last;
}

QString CityMobileAPI::getSmena() {
    QSqlQuery *qSmena = new QSqlQuery(_db);
    qSmena->exec("SELECT TOP 1 VCode "
                 "FROM [agzs].[dbo].[ARM_Smena] "
                 "ORDER BY CDate DESC");
    QString smena;
    if (qSmena->next()) {
        smena = qSmena->value(0).toString();
    }
    return smena;
}

int CityMobileAPI::getLastAPI() {
    QSqlQuery *qApiTransactionsLast = new QSqlQuery(_db);
    qApiTransactionsLast->exec("SELECT TOP 1 VCode "
                               "FROM [agzs].[dbo].[PR_APITransaction] "
                               "ORDER BY CDate DESC");
    int last = 0;
    if (qApiTransactionsLast->next()) {
        last = qApiTransactionsLast->value(0).toInt();
    }
    delete qApiTransactionsLast;
    return last;
}

#define FuelStart {
QString CityMobileAPI::getFuelAPIName(int aFuelVCode) {
    switch(aFuelVCode) {
    case 3: {
        return "a92";
        break;
    }
    case 14: {
        return "propane";
        break;
    }
    case 32: {
        return "diesel";
        break;
    }
    default: {
        return "";
    }
    }
}

int CityMobileAPI::getFuelID(QString aFuelIdApi) {
    if (aFuelIdApi == "diesel")
        return 32;
    if (aFuelIdApi == "diesel_premium")
        return 0;
    if (aFuelIdApi == "a80")
        return 0;
    if (aFuelIdApi == "a92")
        return 3;
    if (aFuelIdApi == "a92_premium")
        return 0;
    if (aFuelIdApi == "a95")
        return 8;
    if (aFuelIdApi == "a95_premium")
        return 0;
    if (aFuelIdApi == "a98")
        return 0;
    if (aFuelIdApi == "a98_premium")
        return 0;
    if (aFuelIdApi == "a100")
        return 0;
    if (aFuelIdApi == "a100_premium")
        return 0;
    if (aFuelIdApi == "propane")
        return 14;
    if (aFuelIdApi == "metan")
        return 0;
    return 0;
}

QString CityMobileAPI::getFullFuelName(int aFuelId) {
    QSqlQuery *qFuels = new QSqlQuery(_db);
    qFuels->exec("SELECT TOP 1 [DBVCode], [DBName] "
                  "FROM [agzs].[dbo].[FuelMassLink] "
                  "WHERE [DBVCode] = "+QString::number(aFuelId));
    QString fullName;
    if (qFuels->next()) {
        fullName = qFuels->value(1).toString();
    }
    delete qFuels;
    return fullName;
}
#define FuelEnd }
#define GetDataEnd }
