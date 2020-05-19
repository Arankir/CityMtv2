#include <QCoreApplication>
#include <QObject>
#include <citymobileapi.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QTimer>
#include <QEventLoop>
#include <QTextCodec>

QString ToUTF8(QString a_text){
    QByteArray ba = a_text.toUtf8();
    QTextCodec *codec_1251 = QTextCodec::codecForName("Windows-1251");
    QString string = codec_1251->toUnicode(ba);

    QTextCodec *codec_utf8 = QTextCodec::codecForName("UTF-8");
    QByteArray encodedString_utf8 = codec_utf8->fromUnicode(string);
    return QString(encodedString_utf8);
}
QString GetFuelAPIName(QString a_fuelFullName){
    if(a_fuelFullName=="Дизельное топливо")
        return "diesel";
//    if(a_fuelFullName=="diesel_premium")
//        return "diesel_premium";
//    if(a_fuelFullName=="a80")
//        return "a80";
    if(a_fuelFullName=="Бензин АИ-92")
        return "a92";
//    if(a_fuelFullName=="a92_premium")
//        return "a92_premium";
//    if(a_fuelFullName=="a95")
//        return "a95";
//    if(a_fuelFullName=="a95_premium")
//        return "a95_premium";
//    if(a_fuelFullName=="a98")
//        return "a98";
//    if(a_fuelFullName=="a98_premium")
//        return "a98_premium";
//    if(a_fuelFullName=="a100")
//        return "a100";
//    if(a_fuelFullName=="a100_premium")
//        return "a100_premium";
    if(a_fuelFullName=="Сжиженный газ")
        return "propane";
//    if(a_fuelFullName=="metan")
//        return "metan";
    return 0;
}
int GetFuelID(QString a_fuelIdAPI){
    if(a_fuelIdAPI=="diesel")
        return 32;
    if(a_fuelIdAPI=="diesel_premium")
        return 0;
    if(a_fuelIdAPI=="a80")
        return 0;
    if(a_fuelIdAPI=="a92")
        return 3;
    if(a_fuelIdAPI=="a92_premium")
        return 0;
    if(a_fuelIdAPI=="a95")
        return 8;
    if(a_fuelIdAPI=="a95_premium")
        return 0;
    if(a_fuelIdAPI=="a98")
        return 0;
    if(a_fuelIdAPI=="a98_premium")
        return 0;
    if(a_fuelIdAPI=="a100")
        return 0;
    if(a_fuelIdAPI=="a100_premium")
        return 0;
    if(a_fuelIdAPI=="propane")
        return 14;
    if(a_fuelIdAPI=="metan")
        return 0;
    return 0;
}
QString GetFullFuelName(QSqlDatabase a_db, int a_fuelID){
    QSqlQuery *q_Fuels = new QSqlQuery(a_db);
    q_Fuels->exec("SELECT TOP 1 [DBVCode], [DBName] FROM [agzs].[dbo].[FuelMassLink] where [DBVCode]="+QString::number(a_fuelID));
    q_Fuels->next();
    QString fullName=q_Fuels->value(1).toString();
    delete q_Fuels;
    return fullName;
}
int GetCashBoxIndex(QSqlDatabase a_db){
    QSqlQuery *q_CashBox = new QSqlQuery(a_db);
    q_CashBox->exec("SELECT TOP 1 CashBoxIndex, PayIndex, PayWay, AutoCheck FROM [agzs].[dbo].[ARM_CashBoxesSemaphor] where PayIndex=10 and PayWay='СИТИМОБИЛ' and AutoCheck=0 order by CDate");
    q_CashBox->next();
    int last=q_CashBox->value(0).toInt();
    delete q_CashBox;
    return last;
}
int CheckError(QString a_api_key, QSqlDatabase a_db, QString a_stationID, QString a_columnID, QString a_fuelID, QString a_priceFuel, QString a_id, QString a_lastVCode, QDateTime now){
    QSqlQuery *q_Check = new QSqlQuery(a_db);
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, d.Adress, d.Location_x, d.Location_y, d.ColumnsCount, d.AGZSL, d.AGZSP "
                ",d.[diesel_price] ,d.[diesel_premium_price] ,d.[a80_price] ,d.[a92_price] ,d.[a92_premium_price] ,d.[a95_price] "
                ",d.[a95_premium_price] ,d.[a98_price] ,d.[a98_premium_price] ,d.[a100_price] ,d.[a100_premium_price] ,d.[propane_price] "
                ",d.[metan_price], c.[DeviceName], c.[Serial], c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], c.[TrkFuelCode], c.[TrkVCode] "
                "FROM [agzs].[dbo].PR_AGZSData d INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c ON [agzs].[dbo].PR_AGZSData.VCode = [agzs].[dbo].PR_AGZSColumnsData.Link "
                "where d.Id='"+a_stationID+"' and c.[TrkVCode]="+QString::number(a_columnID.toInt()/1000)+" and c.SideAdress="+QString::number(a_columnID.toInt()%1000)+" order by CDate desc");
    if(q_Check->size()==0){
        CityMobileAPI *api = new CityMobileAPI();
        api->SetRequestStateCanceled(a_api_key,a_id,"Указанная колонка не найдена.",a_lastVCode,now);
        delete q_Check;
        return 1;
    }
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, d.Adress, d.Location_x, d.Location_y, d.ColumnsCount, d.AGZSL, d.AGZSP "
                ",d.[diesel_price] ,d.[diesel_premium_price] ,d.[a80_price] ,d.[a92_price] ,d.[a92_premium_price] ,d.[a95_price] "
                ",d.[a95_premium_price] ,d.[a98_price] ,d.[a98_premium_price] ,d.[a100_price] ,d.[a100_premium_price] ,d.[propane_price] "
                ",d.[metan_price], c.[DeviceName], c.[Serial], c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], c.[TrkFuelCode], c.[TrkVCode] "
                "FROM [agzs].[dbo].PR_AGZSData d INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c ON [agzs].[dbo].PR_AGZSData.VCode = [agzs].[dbo].PR_AGZSColumnsData.Link "
                "where d.Id='"+a_stationID+"' and d.["+a_fuelID+"]>0 order by CDate desc");
    if(q_Check->size()==0){
        CityMobileAPI *api = new CityMobileAPI();
        api->SetRequestStateCanceled(a_api_key,a_id,"Не обнаружено указанное топливо.",a_lastVCode,now);
        delete q_Check;
        return 2;
    }
    q_Check->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, d.Adress, d.Location_x, d.Location_y, d.ColumnsCount, d.AGZSL, d.AGZSP "
                ",d.[diesel_price] ,d.[diesel_premium_price] ,d.[a80_price] ,d.[a92_price] ,d.[a92_premium_price] ,d.[a95_price] "
                ",d.[a95_premium_price] ,d.[a98_price] ,d.[a98_premium_price] ,d.[a100_price] ,d.[a100_premium_price] ,d.[propane_price] "
                ",d.[metan_price], c.[DeviceName], c.[Serial], c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], c.[TrkFuelCode], c.[TrkVCode] "
                "FROM [agzs].[dbo].PR_AGZSData d INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c ON [agzs].[dbo].PR_AGZSData.VCode = [agzs].[dbo].PR_AGZSColumnsData.Link "
                "where d.Id='"+a_stationID+"' and d.["+a_fuelID+"]="+a_priceFuel+" order by CDate desc");
    if(q_Check->size()==0){
        CityMobileAPI *api = new CityMobileAPI();
        api->SetRequestStateCanceled(a_api_key,a_id,"Цена на выбранный вид топлива отличается от фактической цены.",a_lastVCode,now);
        delete q_Check;
        return 3;
    }
    delete q_Check;
    return 0;
}
int GetLastAPI(QSqlDatabase a_db){
    QSqlQuery *q_APIRequestsLast = new QSqlQuery(a_db);
    q_APIRequestsLast->exec("SELECT TOP 1 VCode FROM [agzs].[dbo].[PR_APITransaction] order by CDate");
    q_APIRequestsLast->next();
    int last=q_APIRequestsLast->value(0).toInt();
    delete q_APIRequestsLast;
    return last;
}
void MoneyData(QJsonObject a_data, int *a_requestTotalPriceDB, int *a_requestVolumeDB, int *a_requestUnitPriceDB, int *a_moneyTakenDB, int *a_fullTankDB){
    if(a_data.value("OrderType").toString()=="Money"){
        *a_requestTotalPriceDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestVolumeDB=*a_requestTotalPriceDB / *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=0;
    } else if(a_data.value("OrderType").toString()=="Liters"){
        *a_requestVolumeDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestTotalPriceDB=*a_requestVolumeDB * *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=0;
    } else if(a_data.value("OrderType").toString()=="FullTank"){
        *a_requestTotalPriceDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestVolumeDB=*a_requestTotalPriceDB / *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=1;
    }
}
QString GetSmena(QSqlDatabase a_db){
    QSqlQuery *q_Smena = new QSqlQuery(a_db);
    q_Smena->exec("SELECT top 1 VCode FROM [agzs_test].[dbo].[ARM_Smena] order by cdate desc");
    q_Smena->next();
    return q_Smena->value(0).toString();
}
void InsertNewRequest(QString a_api_key, QJsonObject a_request, QSqlDatabase a_db){
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
    int lastAPIVCode = GetLastAPI(a_db);
    int error = CheckError(a_api_key,a_db,a_request.value("StationId").toString(),a_request.value("ColumnId").toString(),a_request.value("FuelId").toString(),QString::number(a_request.value("PriceFuel").toInt()),a_request.value("Id").toString(),QString::number(lastAPIVCode+1),now);
    QString fuelFullName=GetFullFuelName(a_db,GetFuelID(a_request.value("FuelId").toString()));
    QSqlQuery *q_AGZSColumn = new QSqlQuery(a_db);
    q_AGZSColumn->exec("SELECT c.AGZSName, c.AGZS, d.VCode, d.Id, d.Adress, d.Location_x, d.Location_y, d.ColumnsCount, d.AGZSL, d.AGZSP "
                ",d.[diesel_price] ,d.[diesel_premium_price] ,d.[a80_price] ,d.[a92_price] ,d.[a92_premium_price] ,d.[a95_price] "
                ",d.[a95_premium_price] ,d.[a98_price] ,d.[a98_premium_price] ,d.[a100_price] ,d.[a100_premium_price] ,d.[propane_price] "
                ",d.[metan_price], c.TrkType, c.[DeviceName], c.[Serial], c.[FuelName], c.[FuelShortName], c.[Side], c.[SideAdress], c.[Nozzle], c.[TrkFuelCode], c.[TrkVCode] "
                "FROM [agzs].[dbo].PR_AGZSData d INNER JOIN [agzs].[dbo].PR_AGZSColumnsData c ON [agzs].[dbo].PR_AGZSData.VCode = [agzs].[dbo].PR_AGZSColumnsData.Link "
                "where d.Id='"+a_request.value("StationId").toString()+"' and c.[TrkVCode]="+QString::number(a_request.value("ColumnId").toInt()/1000)+" and "
                "c.SideAdress="+QString::number(a_request.value("ColumnId").toInt()%1000)+" and c.FuelName='"+fuelFullName+"' "
                "d.["+a_request.value("FuelId").toString()+"]="+QString::number(a_request.value("PriceFuel").toInt())+" and d. order by CDate desc");
    q_AGZSColumn->next();
    int transactionVCode=-1;
    if(error!=0){
        int requestTotalPriceDB=-1,	requestVolumeDB=-1,	requestUnitPriceDB=-1, moneyTakenDB=-1, fullTankDB=-1;
        MoneyData(a_request, &requestTotalPriceDB, &requestVolumeDB, &requestUnitPriceDB, &moneyTakenDB, &fullTankDB);
        QSqlQuery *q_Transaction = new QSqlQuery(a_db);
        q_Transaction->exec("SELECT TOP 1 Value FROM [agzs].[dbo].[LxKeysOfCodes] where Key='ADAST_TRKTransaction'");
        q_Transaction->next();
        transactionVCode=q_Transaction->value(0).toInt()+1;
        q_Transaction->exec("Update [agzs].[dbo].[LxKeysOfCodes] set Value="+QString::number(transactionVCode)+" where Key='ADAST_TRKTransaction'");
        q_Transaction->prepare("INSERT INTO [agzs].[dbo].[ADAST_TRKTransaction] (AGZSName, LocalVCode, TrkType, DeviceName,	Serial,	FuelName, FuelShortName, Side, SideAddress, Nozzle, "
                       "TrkFuelCode, TransNum, TrkTotalPriceDB, TrkVolumeDB, TrkUnitPriceDB, RequestTotalPriceDB, RequestVolumeDB, RequestUnitPriceDB, RequestField, State, iState, "
                       "TrkTransType, LitersCountBeforeDB, MoneyCountBeforeDB, TransCountBefore, LitersCountAfterDB, MoneyCountAfterDB, TransCountAfter, Result, DateOpen, DateClose, "
                       "TemperatureDB, PayOperationVCode, PayWay, PrePostPay, WUser, WDate, CUser, CDate, CHost, WHost, VCode, AddedForTransVCode, AditionalTransVCode, ActiveDB, "
                       "MassDB, Smena, TrkVcode, CapacityVcode, PumpPlace, MoneyTakenDB, iPayWay, AutoCheckDB, Closed, FullTankDB, AGZS, FuelVCode, rowguid, Propan) VALUES("
                       ":AGZSName, :LocalVCode, :TrkType, :DeviceName, :Serial,	:FuelName, :FuelShortName, :Side, :SideAddress, :Nozzle, :TrkFuelCode, :TransNum, :TrkTotalPriceDB, "
                       ":TrkVolumeDB, :TrkUnitPriceDB, :RequestTotalPriceDB, :RequestVolumeDB, :RequestUnitPriceDB, :RequestField, :State, :iState, :TrkTransType, :LitersCountBeforeDB, "
                       ":MoneyCountBeforeDB, :TransCountBefore, :LitersCountAfterDB, :MoneyCountAfterDB, :TransCountAfter, :Result, :DateOpen, :DateClose, :TemperatureDB, "
                       ":PayOperationVCode, :PayWay, :PrePostPay, :WUser, :WDate, :CUser, :CDate, :CHost, :WHost, :VCode, :AddedForTransVCode, :AditionalTransVCode, :ActiveDB, :MassDB, "
                       ":Smena, :TrkVcode, :CapacityVcode, :PumpPlace, :MoneyTakenDB, :iPayWay, :AutoCheckDB, :Closed, :FullTankDB, :AGZS, :FuelVCode, :rowguid, :Propan)");
        q_Transaction->bindValue(":AGZSName",q_AGZSColumn->value(0).toString());
        q_Transaction->bindValue(":LocalVCode",QString::number(transactionVCode));
        q_Transaction->bindValue(":TrkType",q_AGZSColumn->value(23).toString());
        q_Transaction->bindValue(":DeviceName",q_AGZSColumn->value(24).toString());
        q_Transaction->bindValue(":Serial",q_AGZSColumn->value(25).toString());
        q_Transaction->bindValue(":FuelName",q_AGZSColumn->value(26).toString());
        q_Transaction->bindValue(":FuelShortName",q_AGZSColumn->value(27).toString());
        q_Transaction->bindValue(":Side",q_AGZSColumn->value(28).toString());
        q_Transaction->bindValue(":SideAddress",q_AGZSColumn->value(29).toString());
        q_Transaction->bindValue(":Nozzle",q_AGZSColumn->value(30).toString());
        q_Transaction->bindValue(":TrkFuelCode",q_AGZSColumn->value(31).toString());
        q_Transaction->bindValue(":TransNum","");
        q_Transaction->bindValue(":TrkTotalPriceDB",0);
        q_Transaction->bindValue(":TrkVolumeDB",0);
        q_Transaction->bindValue(":TrkUnitPriceDB",0);
        q_Transaction->bindValue(":RequestTotalPriceDB",requestTotalPriceDB);
        q_Transaction->bindValue(":RequestVolumeDB",requestVolumeDB);
        q_Transaction->bindValue(":RequestUnitPriceDB",requestUnitPriceDB);
        q_Transaction->bindValue(":RequestField","V");
        q_Transaction->bindValue(":State","Завершение выдачи");
        q_Transaction->bindValue(":iState",8);
        q_Transaction->bindValue(":TrkTransType","");
        q_Transaction->bindValue(":LitersCountBeforeDB",0);
        q_Transaction->bindValue(":MoneyCountBeforeDB",0);
        q_Transaction->bindValue(":TransCountBefore",0);
        q_Transaction->bindValue(":LitersCountAfterDB",0);
        q_Transaction->bindValue(":MoneyCountAfterDB",0);
        q_Transaction->bindValue(":TransCountAfter",0);
        q_Transaction->bindValue(":Result","Выдача завершена: 1");
        q_Transaction->bindValue(":DateOpen",now);
        q_Transaction->bindValue(":DateClose",now);
        q_Transaction->bindValue(":TemperatureDB",-100);
        q_Transaction->bindValue(":PayOperationVCode",0);
        q_Transaction->bindValue(":PayWay","СИТИМОБИЛ");
        q_Transaction->bindValue(":PrePostPay",0);
        q_Transaction->bindValue(":WUser","SERVER");
        q_Transaction->bindValue(":WDate",now);
        q_Transaction->bindValue(":CUser","SERVER");
        q_Transaction->bindValue(":CDate",now);
        q_Transaction->bindValue(":CHost","SERVER");
        q_Transaction->bindValue(":WHost","SERVER");
        q_Transaction->bindValue(":VCode",transactionVCode);
        q_Transaction->bindValue(":AddedForTransVCode",0);
        q_Transaction->bindValue(":AditionalTransVCode",0);
        q_Transaction->bindValue(":ActiveDB",0);
        q_Transaction->bindValue(":MassDB",0);
        q_Transaction->bindValue(":Smena",GetSmena(a_db));
        q_Transaction->bindValue(":TrkVcode",q_AGZSColumn->value(32).toString());
        q_Transaction->bindValue(":CapacityVcode",GetCashBoxIndex(a_db));
        q_Transaction->bindValue(":PumpPlace",q_AGZSColumn->value(29).toString());
        q_Transaction->bindValue(":MoneyTakenDB",moneyTakenDB);
        q_Transaction->bindValue(":iPayWay",10);
        q_Transaction->bindValue(":AutoCheckDB",0);
        q_Transaction->bindValue(":Closed",0);
        q_Transaction->bindValue(":FullTankDB",fullTankDB);
        q_Transaction->bindValue(":AGZS",q_AGZSColumn->value(1).toString());
        q_Transaction->bindValue(":FuelVCode",GetFuelID(a_request.value("FuelId").toString()));
        q_Transaction->bindValue(":rowguid","DEFAULT");
        q_Transaction->bindValue(":Propan",0);
        q_Transaction->exec();
        delete q_Transaction;
        CityMobileAPI *api = new CityMobileAPI();
        api->SetRequestStateAccept(a_api_key,a_request.value("Id").toString());
    }
    QSqlQuery *q_APIRequests = new QSqlQuery(a_db);
    q_APIRequests->prepare("INSERT INTO [agzs].[dbo].[PR_APITransaction] ([AGZSName],[AGZS],[CDate],[VCode],[APIID],[APIStationExtendedId],[APIColumnId],[APIFuelId],[FuelId],"
                "[APIPriceFuel],[APILitre],[APISum],[APIStatus],[APIContractId],[agent],[localState],[Price],[Sum],[DateOpen],[DateClose],[Link]) VALUES (:AGZSName, :AGZS, :CDate, "
                ":VCode, :APIID, :APIStationExtendedId, :APIColumnId, :APIFuelId, :FuelId, :APIPriceFuel, :APILitre, :APISum, :APIStatus, :APIContractId, :agent, :localState, :Price, "
                ":Sum, :DateOpen, :DateClose, :Link)");
    q_APIRequests->bindValue(":AGZSName",q_AGZSColumn->value(0).toString());
    q_APIRequests->bindValue(":AGZS",q_AGZSColumn->value(1).toString());
    q_APIRequests->bindValue(":CDate",QDateTime::fromString(a_request.value("DateCreate").toString(),"yyyy-MM-ddThh:mm:ss.zzzZ").toString("yyyy-MM-dd hh:mm:ss.zzz"));
    q_APIRequests->bindValue(":VCode",lastAPIVCode+1);
    q_APIRequests->bindValue(":APIID",a_request.value("Id").toString());
    q_APIRequests->bindValue(":APIStationExtendedId",a_request.value("StationExtendedId").toString());
    q_APIRequests->bindValue(":APIColumnId",a_request.value("ColumnId").toString());
    q_APIRequests->bindValue(":APIFuelId",a_request.value("FuelId").toString());
    q_APIRequests->bindValue(":FuelId",GetFuelID(a_request.value("FuelId").toString()));
    q_APIRequests->bindValue(":APIPriceFuel",a_request.value("PriceFuel").toString());
    q_APIRequests->bindValue(":APILitre",a_request.value("Litre").toString());
    q_APIRequests->bindValue(":APISum",a_request.value("Sum").toString());
    q_APIRequests->bindValue(":APIStatus",a_request.value("Status").toString());
    q_APIRequests->bindValue(":APIContractId",a_request.value("ContractId").toString());
    q_APIRequests->bindValue(":agent","CityMobile");
    q_APIRequests->bindValue(":localState",error!=0?"Error: "+QString::number(error):"0");
    q_APIRequests->bindValue(":Price",0);
    q_APIRequests->bindValue(":Sum",0);
    q_APIRequests->bindValue(":DateOpen",now);
    q_APIRequests->bindValue(":DateClose","DEFAULT");
    q_APIRequests->bindValue(":Link",transactionVCode);
    q_APIRequests->exec();
}

void VerifyRequests(QString a_api_key, QSqlDatabase a_db){
    //AdditionalTransVCode = VCode другой записи - идет заправка
    //если там статус 'Выдача', iState=4, (LitersCountBeforeDB, MoneyCountBeforeDB, TransCountBefore)!=0, Transnum!='' - наливают
    //если заполнено все и статус 'Завершение выдачи' - заправили
    QSqlQuery* q_requests = new QSqlQuery(a_db);
    q_requests->exec("SELECT api.AGZSName, api.AGZS, api.APIID, api.APIColumnId, api.FuelId, api.localState, api.VCode, h.VCode, b.VCode, b.[State], b.iState, b.TrkTotalPriceDB, b.TrkVolumeDb, b.TrkUnitPriceDB, b.DateOpen, b.DateClose "
             "FROM [agzs].[dbo].[PR_APITransaction] api "
             "INNER JOIN [agzs].[dbo].[ADAST_TRKTransaction] h ON api.Link = h.VCode "
             "INNER JOIN [agzs].[dbo].[ADAST_TRKTransaction] b ON h.AditionalTransVCode = b.VCode "
             "WHERE api.localState in ('0','1','2','3','4','')");
    while (q_requests->next()) {
        if(q_requests->value(10).toInt()!=q_requests->value(5).toInt()){
            QSqlQuery *q_apiUpdate = new QSqlQuery(a_db);
            switch (q_requests->value(10).toInt()) {
            case 0:{//Формирование цен   Не сформирована

                break;
            }
            case 1:{//Цена установлена
                q_apiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] set localState = :localState, DateClose = :date where VCode = :VCode");
                q_apiUpdate->bindValue(":localState",q_requests->value(10).toInt());
                q_apiUpdate->bindValue(":date",q_requests->value(15).toDateTime());
                q_apiUpdate->bindValue(":VCode",q_requests->value(6).toInt());
                q_apiUpdate->exec();
                break;
            }
            case 2:{//Запрос счетчиков
                q_apiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] set localState = :localState, DateClose = :date where VCode = :VCode");
                q_apiUpdate->bindValue(":localState",q_requests->value(10).toInt());
                q_apiUpdate->bindValue(":date",q_requests->value(15).toDateTime());
                q_apiUpdate->bindValue(":VCode",q_requests->value(6).toInt());
                q_apiUpdate->exec();
                break;
            }
            case 3:{//Запрос выдачи
                q_apiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] set localState = :localState, DateClose = :date where VCode = :VCode");
                q_apiUpdate->bindValue(":localState",q_requests->value(10).toInt());
                q_apiUpdate->bindValue(":date",q_requests->value(15).toDateTime());
                q_apiUpdate->bindValue(":VCode",q_requests->value(6).toInt());
                q_apiUpdate->exec();
                break;
            }
            case 4:{//Выдача
                q_apiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] set localState = :localState, DateClose = :date where VCode = :VCode");
                q_apiUpdate->bindValue(":localState",q_requests->value(10).toInt());
                q_apiUpdate->bindValue(":date",q_requests->value(15).toDateTime());
                q_apiUpdate->bindValue(":VCode",q_requests->value(6).toInt());
                q_apiUpdate->exec();
                CityMobileAPI *api = new CityMobileAPI();
                api->SetRequestStateFueling(a_api_key,q_requests->value(2).toString());
                break;
            }
            case 5 ... 8 :{//Выдано //Подтверждение   Завершение выдачи //Обновление счетчиков //Завершение выдачи
                QSqlQuery *q_PayOperation = new QSqlQuery(a_db);
                q_PayOperation->exec("SELECT [AmountDB], [PriceDB], [VolumeDB] FROM [agzs].[dbo].[ARM_PayOperation] where Link="+q_requests->value(7).toString());
                if(q_PayOperation->size()>0){
                    q_apiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] set localState = :localState, [Price] = :price, ,[Litre] = :litre, [Sum] = :sum, [DateOpen] = :dateOpen, DateClose = :dateClose where VCode = :VCode");
                    q_apiUpdate->bindValue(":localState",5);
                    q_apiUpdate->bindValue(":price",q_requests->value(13).toInt());
                    q_apiUpdate->bindValue(":litre",q_requests->value(12).toInt());
                    q_apiUpdate->bindValue(":sum",q_requests->value(11).toInt());
                    q_apiUpdate->bindValue(":dateOpen",q_requests->value(14).toDateTime());
                    q_apiUpdate->bindValue(":dateClose",q_requests->value(15).toDateTime());
                    q_apiUpdate->bindValue(":VCode",q_requests->value(6).toInt());
                    q_apiUpdate->exec();
                    CityMobileAPI *api = new CityMobileAPI();
                    api->SetRequestStateCompleted(a_api_key,q_requests->value(2).toString(),q_requests->value(12).toDouble(),q_requests->value(6).toString(),q_requests->value(15).toDateTime());
                }
                delete q_PayOperation;
                break;
            }
            default:{

            }
            }
            delete q_apiUpdate;
        }
    }
}

void GetColumnsFuelsData(QJsonArray &a_fuels, QJsonObject &a_columns, QSqlDatabase a_db, QString a_vCodeAgzs){
    QSqlQuery* q2 = new QSqlQuery(a_db);
    q2->exec("SELECT d.AGZSName, d.VCode, c.FuelName, c.TrkVCode, c.SideAdress "
             "FROM [agzs].[dbo].[PR_AGZSColumnsData] c INNER JOIN [agzs].[dbo].PR_AGZSData d ON d.VCode = c.Link WHERE (((d.VCode)="+a_vCodeAgzs+")) order by c.TrkVCode, c.SideAdress asc");
    while (q2->next()) {
        QString fuelAPIName=GetFuelAPIName(q2->value(2).toString());
        QJsonObject fuel;
        fuel["Fuel"]=fuelAPIName;
        fuel["FuelName"]=q2->value(2).toString();
        bool insert=std::move(true);
        for(auto fuelN: a_fuels){
            if(fuelN.toObject().value("Fuel")==fuel.value("Fuel")){
                insert=std::move(false);
                break;
            }
        }
        if(insert){
            a_fuels.append(std::move(fuel));
        }
        QString columnNumber = QString::number(q2->value(4).toInt()*1000+q2->value(3).toInt());
        if(a_columns.value(columnNumber).toString()==""){
            QJsonObject column;
            column["Fuels"]=std::move(QJsonArray());
            a_columns[columnNumber]=column;
        }
        QJsonObject value;
        QJsonArray fuels = a_columns.value(columnNumber).toObject().value("Fuels").toArray();
        fuels.append(fuelAPIName);
        value["Fuels"]=fuels;
        a_columns[columnNumber] = value;
    }
    delete q2;
}

void SetAZSData(QString a_api_key, QSqlDatabase a_db){
//    {
//        "StationExtendedId": "1234",
//        "Columns": {
//            "1": {
//                "Fuels": ["a92", "a95", "a95_premium", "diesel_premium"]
//            },
//            "2": {
//                "Fuels": ["a92", "a95", "a95_premium", "a100_premium"]
//            }
//        },
//        "Enable": true,
//        "Name": "АЗС \"На Мандариновой\" №10",
//        "Address": "г. Энск, ул. Мандариновая, 4",
//        "Location": { "Lon": 43.5432, "Lat": 34.1234 },
//        "FuelNames": [
//        {
//            "Fuel": "a92",
//            "FuelName": "АИ-92"
//        },
//        {
//            "Fuel": "a95",
//            "FuelName": "АИ-95"
//        },
//        {
//            "Fuel": "a95_premium",
//            "FuelName": "G-95"
//        },
//        {
//            "Fuel": "a100_premium",
//            "FuelName": "G-100"
//        },
//        {
//            "Fuel": "diesel_premium",
//            "FuelName": "ДТ Опти"
//        }
//        ]
//    }
    QJsonArray AGZSs;
    QSqlQuery* q = new QSqlQuery(a_db);
    if (q->exec("SELECT AGZSName, AGZS, VCode, Id, Adress, Location_x, Location_y, ColumnsCount, AGZSL, AGZSP FROM [agzs].[dbo].PR_AGZSData where AGZS=(SELECT TOP 1 AGZS FROM [agzs].[dbo].[Identification])")) {
        while (q->next()) {
            QJsonObject AGZS;
            AGZS["StationExtendedId"] = q->value(3).toString();
            AGZS["Enable"] = true;
            AGZS["Name"] = q->value(0).toString();
            AGZS["Adress"] = q->value(4).toString();

            QJsonObject Location;
            Location["Lon"] = q->value(6).toString().replace(",",".");
            Location["Lat"] = q->value(5).toString().replace(",",".");
            AGZS["Location"] = Location;
            //Columns
            QJsonObject Columns;
            //FuelNames
            QJsonArray FuelNames;
            GetColumnsFuelsData(FuelNames, Columns, a_db, q->value(2).toString());

            AGZS["Columns"]=Columns;
            AGZS["FuelNames"]=FuelNames;
            AGZSs.append(AGZS);
        }
    }
    QJsonDocument doc;
    doc.setArray(AGZSs);
    delete q;
    CityMobileAPI *api = new CityMobileAPI();
    api->SetAZSData(a_api_key,doc);
}

void SetPriceList(QString a_api_key, QSqlDatabase a_db){
//    key1=value1&key2=value2&...
    QStringList prices;
    QSqlQuery* q = new QSqlQuery(a_db);
    if (q->exec("SELECT AGZSName,Id,VCode,ColumnsCount,[diesel_price],[diesel_premium_price],[a80_price],[a92_price],[a92_premium_price],[a95_price],[a95_premium_price]"
                ",[a98_price],[a98_premium_price],[a100_price],[a100_premium_price],[propane_price],[metan_price] FROM [agzs].[dbo].PR_AGZSData where AGZS=(SELECT TOP 1 AGZS FROM [agzs].[dbo].[Identification])")) {
        while (q->next()) {
            if(q->value(4).toInt()>0){
                prices.append("diesel="+QString::number(q->value(4).toDouble()));
            }
            if(q->value(5).toInt()>0){
                prices.append("diesel_premium="+QString::number(q->value(5).toDouble()));
            }
            if(q->value(6).toInt()>0){
                prices.append("a80="+QString::number(q->value(6).toDouble()));
            }
            if(q->value(7).toInt()>0){
                prices.append("a92="+QString::number(q->value(7).toDouble()));
            }
            if(q->value(8).toInt()>0){
                prices.append("a92_premium="+QString::number(q->value(8).toDouble()));
            }
            if(q->value(9).toInt()>0){
                prices.append("a95="+QString::number(q->value(9).toDouble()));
            }
            if(q->value(10).toInt()>0){
                prices.append("a95_premium="+QString::number(q->value(10).toDouble()));
            }
            if(q->value(11).toInt()>0){
                prices.append("a98="+QString::number(q->value(11).toDouble()));
            }
            if(q->value(12).toInt()>0){
                prices.append("a98_premium="+QString::number(q->value(12).toDouble()));
            }
            if(q->value(13).toInt()>0){
                prices.append("a100="+QString::number(q->value(13).toDouble()));
            }
            if(q->value(14).toInt()>0){
                prices.append("a100_premium="+QString::number(q->value(14).toDouble()));
            }
            if(q->value(15).toInt()>0){
                prices.append("propane="+QString::number(q->value(15).toDouble()));
            }
            if(q->value(16).toInt()>0){
                prices.append("metan="+QString::number(q->value(16).toDouble()));
            }
        }
    }
    delete q;
    CityMobileAPI *api = new CityMobileAPI();
    api->SetPriceList(a_api_key,prices.join("&").replace(",","."));
}

void GetRequests(QString a_api_key, QSqlDatabase a_db){
// проверять новые заказы

//    {
//        "nextRetryMs": 2000,
//        "Orders": [
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
//        ]
//    }

    CityMobileAPI *api = new CityMobileAPI();
    api->GetRequests(a_api_key);
    QObject::connect(api,&CityMobileAPI::s_finished,[=](QNetworkReply *reply){
        QJsonArray orders=QJsonDocument::fromJson(reply->readAll()).object().value("Orders").toArray();
        qDebug()<<"Requests"<<orders;
        for(auto order: orders){
            QSqlQuery *q_requests = new QSqlQuery(a_db);
            q_requests->exec("SELECT APIID FROM [agzs].[dbo].[PR_APITransaction] where APIID='"+order.toObject().value("Id").toString()+"'");
            if(q_requests->size()==0)
                InsertNewRequest(a_api_key,order.toObject(),a_db);
            delete q_requests;
        }
    });
    VerifyRequests(a_api_key, a_db);
}

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    setlocale(LC_ALL, "");
    QSqlDatabase _db = QSqlDatabase::addDatabase("QODBC3");
    QStringList setting;
    if(QFile::exists("Setting.txt")){
        QFile settings("Setting.txt");
        if (settings.open(QIODevice::ReadOnly)){
            while(!settings.atEnd()){
                setting << QString::fromLocal8Bit(settings.readLine()).remove("\r\n").remove("\n");
            }
            settings.close();
        } else
            qDebug()<<"Error: setting file is already open";
    } else
        qDebug()<<"Error: setting file not found";
    _db.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=agzs;Persist Security Info=true;uid=sa;pwd=Gfhf743Djpbr").arg(setting[0]));
    //DB_.setHostName("127.0.0.1");
    //DB_.setUserName("root");
    //DB_.setPassword("1423");
    if(_db.open()){
//        _manager = new QNetworkAccessManager;
        qDebug() << "db open";
        QTimer timer;
        QEventLoop loop;
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(10000);
        while(true){
            loop.exec();
            SetAZSData(setting[1],_db);
            SetPriceList(setting[1],_db);
            GetRequests(setting[1],_db);
        }
//        _timer.setInterval(60000);
//        QObject::connect(&_timer,SIGNAL(timeout()),this,SLOT(SendFuel()));
//        _timer.start();
    }
    else {
        qDebug() << "db close";
    }

    return a.exec();
}
