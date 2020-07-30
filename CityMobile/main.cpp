#include <QCoreApplication>
#include <QObject>
#include <citymobileapi.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QTimer>
#include <QEventLoop>
#include <QTextCodec>


#define GetDataStart {
#define FuelStart {
QString GetFuelAPIName(int aFuelVCode) {
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

int GetFuelID(QString aFuelIdApi) {
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

QString GetFullFuelName(QSqlDatabase aDb, int aFuelId){
    QSqlQuery *qFuels = new QSqlQuery(aDb);
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

int GetCashBoxIndex(QSqlDatabase aDb) {
    QSqlQuery *qCashBox = new QSqlQuery(aDb);
    qCashBox->exec("SELECT TOP 1 CashBoxIndex, PayIndex, PayWay, AutoCheck "
                   "FROM [agzs].[dbo].[ARM_CashBoxesSemaphor] "
                   "WHERE PayIndex=10 and PayWay='СИТИМОБИЛ' and AutoCheck=0 "
                   "ORDER BY CDate DESC");
    qCashBox->next();
    int last = qCashBox->value(0).toInt();
    delete qCashBox;
    return last;
}

QString GetSmena(QSqlDatabase aDb) {
    QSqlQuery *qSmena = new QSqlQuery(aDb);
    qSmena->exec("SELECT TOP 1 VCode "
                 "FROM [agzs].[dbo].[ARM_Smena] "
                 "ORDER BY CDate DESC");
    QString smena;
    if (qSmena->next()) {
        smena = qSmena->value(0).toString();
    }
    return smena;
}

int GetLastAPI(QSqlDatabase aDb) {
    QSqlQuery *qApiTransactionsLast = new QSqlQuery(aDb);
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
#define GetDataEnd }

#define SecondRequests {
void VerifyRequests(CityMobileAPI &aCityMobil, QSqlDatabase aDb) {
    //AdditionalTransVCode = VCode другой записи - идет заправка
    //если там статус 'Выдача', iState=4, (LitersCountBeforeDB, MoneyCountBeforeDB, TransCountBefore)!=0, Transnum!='' - наливают
    //если заполнено все и статус 'Завершение выдачи' - заправили
    QSqlQuery* qRequests = new QSqlQuery(aDb);
    qRequests->exec("SELECT api.AGZSName, api.AGZS, api.APIID, api.APIColumnId, api.FuelId, api.localState, api.VCode, h.VCode, b.VCode, "
                    "b.[State], b.iState, b.TrkTotalPriceDB, b.TrkVolumeDb, b.TrkUnitPriceDB, b.DateOpen, b.DateClose "
                    "FROM [agzs].[dbo].[PR_APITransaction] api "
                        "INNER JOIN [agzs].[dbo].[ADAST_TRKTransaction] h "
                        "ON api.Link = h.VCode "
                        "INNER JOIN [agzs].[dbo].[ADAST_TRKTransaction] b "
                        "ON h.AditionalTransVCode = b.VCode "
                    "WHERE api.localState IN ('0','1','2','3','4','')");
    while (qRequests->next()) {
        if (qRequests->value(10).toInt() != qRequests->value(5).toInt()) {
            QSqlQuery *qApiUpdate = new QSqlQuery(aDb);
            switch (qRequests->value(10).toInt()) {
            case 0: {//Формирование цен   Не сформирована

                break;
            }
            case 1 ... 3: {//Цена установлена
                qApiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] "
                                    "SET localState = :localState, DateClose = :date "
                                    "WHERE VCode = :VCode");
                qApiUpdate->bindValue(":localState",qRequests->value(10).toInt());
                qApiUpdate->bindValue(":date",qRequests->value(15).toDateTime());
                qApiUpdate->bindValue(":VCode",qRequests->value(6).toInt());
                qApiUpdate->exec();
                break;
            }
            case 4: {//Выдача
                qApiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] "
                                    "SET localState = :localState, DateClose = :date "
                                    "WHERE VCode = :VCode");
                qApiUpdate->bindValue(":localState",qRequests->value(10).toInt());
                qApiUpdate->bindValue(":date",qRequests->value(15).toDateTime());
                qApiUpdate->bindValue(":VCode",qRequests->value(6).toInt());
                qApiUpdate->exec();
                aCityMobil.SetRequestStateFueling(qRequests->value(2).toString());
                break;
            }
            case 5 ... 8 :{//Выдано //Подтверждение   Завершение выдачи //Обновление счетчиков //Завершение выдачи
                QSqlQuery *qPayOperation = new QSqlQuery(aDb);
                qPayOperation->exec("SELECT [AmountDB], [PriceDB], [VolumeDB] "
                                    "FROM [agzs].[dbo].[ARM_PayOperation] "
                                    "WHERE Link = "+qRequests->value(7).toString());
                if (qPayOperation->size() > 0) {
                    qApiUpdate->prepare("UPDATE [agzs].[dbo].[PR_APITransaction] "
                                        "SET localState = :localState, [Price] = :price, [Litre] = :litre, [Sum] = :sum, [DateOpen] = :dateOpen, DateClose = :dateClose "
                                        "WHERE VCode = :VCode");
                    qApiUpdate->bindValue(":localState",5);
                    qApiUpdate->bindValue(":price",qRequests->value(13).toInt());
                    qApiUpdate->bindValue(":litre",qRequests->value(12).toInt());
                    qApiUpdate->bindValue(":sum",qRequests->value(11).toInt());
                    qApiUpdate->bindValue(":dateOpen",qRequests->value(14).toDateTime());
                    qApiUpdate->bindValue(":dateClose",qRequests->value(15).toDateTime());
                    qApiUpdate->bindValue(":VCode",qRequests->value(6).toInt());
                    qApiUpdate->exec();
                    aCityMobil.SetRequestStateCompleted(qRequests->value(2).toString(), qRequests->value(12).toDouble(), qRequests->value(6).toString(), qRequests->value(15).toDateTime());
                }
                delete qPayOperation;
                break;
            }
            default:{

            }
            }
            delete qApiUpdate;
        }
    }
    delete qRequests;
}

void GetColumnsFuelsData(QJsonArray &aFuels, QJsonObject &aColumns, QSqlDatabase aDb, QString aVCodeAgzs) {
    QSqlQuery *qColumns = new QSqlQuery(aDb);
    qColumns->exec("SELECT d.AGZSName, d.VCode, c.FuelVCode, c.TrkVCode, c.SideAdress "
             "FROM [agzs].[dbo].[PR_AGZSColumnsData] c "
                "INNER JOIN [agzs].[dbo].PR_AGZSData d "
                "ON d.VCode = c.Link "
             "WHERE d.VCode = "+aVCodeAgzs+" "
             "ORDER BY c.SideAdress ASC");
    while (qColumns->next()) {
        QString fuelAPIName = GetFuelAPIName(qColumns->value(2).toInt());
        QJsonObject fuel;
        fuel["Fuel"] = fuelAPIName;
        fuel["FuelName"] = qColumns->value(2).toString();
        bool insert = std::move(true);
        for (auto fuelN: aFuels) {
            if (fuelN.toObject().value("Fuel") == fuel.value("Fuel")) {
                insert = std::move(false);
                break;
            }
        }
        if (insert) {
            aFuels.append(std::move(fuel));
        }
        QString columnNumber = QString::number(qColumns->value(4).toInt());
        if (aColumns.value(columnNumber).toString() == "") {
            QJsonObject column;
            column["Fuels"] = std::move(QJsonArray());
            aColumns[columnNumber] = column;
        }
        QJsonObject value;
        QJsonArray fuels = aColumns.value(columnNumber).toObject().value("Fuels").toArray();
        fuels.append(fuelAPIName);
        value["Fuels"] = fuels;
        aColumns[columnNumber] = value;
    }
    delete qColumns;
}
#define SecondRequestsEnd }

#define StandartRequests {
void SetAZSData(CityMobileAPI &aCityMobil, QSqlDatabase aDb) {
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
    QJsonObject AGZS;
    QSqlQuery* qAgzsData = new QSqlQuery(aDb);
    if (qAgzsData->exec("SELECT AGZSName, AGZS, VCode, Id, Adress, Location_x, Location_y, ColumnsCount, AGZSL, AGZSP "
                        "FROM [agzs].[dbo].PR_AGZSData "
                        "WHERE AGZS = ("
                                    "SELECT TOP 1 AGZS "
                                    "FROM [agzs].[dbo].[Identification])")) {
        while (qAgzsData->next()) {
            //QJsonObject AGZS;
            AGZS["StationExtendedId"] = qAgzsData->value(3).toString();
            AGZS["Enable"] = true;
            AGZS["Name"] = qAgzsData->value(0).toString();
            AGZS["Adress"] = qAgzsData->value(4).toString();

            QJsonObject Location;
            Location["Lon"] = qAgzsData->value(6).toString().replace(",",".");
            Location["Lat"] = qAgzsData->value(5).toString().replace(",",".");
            AGZS["Location"] = Location;
            //Columns
            QJsonObject Columns;
            //FuelNames
            QJsonArray FuelNames;
            GetColumnsFuelsData(FuelNames, Columns, aDb, qAgzsData->value(2).toString());

            AGZS["Columns"] = Columns;
            AGZS["FuelNames"] = FuelNames;
            //AGZSs.append(AGZS);
        }
    }
    QJsonDocument doc;
    doc.setObject(AGZS);
    delete qAgzsData;
    aCityMobil.SetAZSData(doc);
}

void SetPriceList(CityMobileAPI &aCityMobil, QSqlDatabase aDb) {
//    key1=value1&key2=value2&...
    QStringList prices;
    QSqlQuery* qPrices = new QSqlQuery(aDb);
    qPrices->exec("SELECT TOP 1 [AGZSName], [AGZS], [VCode], [Link], [CDate], [Partner], [iPartner], "
            "[diesel], [diesel_premium], [a80], [a92], [a92_premium], [a95], [a95_premium], "
            "[a98],[a98_premium], [a100], [a100_premium], [propane], [metan] "
            "FROM [agzs].[dbo].PR_AGZSPrice "
            "WHERE Link = (SELECT VCode "
                          "FROM [agzs].[dbo].PR_AGZSData "
                          "WHERE AGZS = (SELECT TOP 1 AGZS "
                                        "FROM [agzs].[dbo].[Identification])) "
            "and iPartner = 10");
    while (qPrices->next()) {
        if(qPrices->value(7).toInt() > 0) {
            prices.append("diesel=" + QString::number(qPrices->value(7).toDouble()));
        }
        if(qPrices->value(8).toInt() > 0) {
            prices.append("diesel_premium=" + QString::number(qPrices->value(8).toDouble()));
        }
        if(qPrices->value(9).toInt() > 0) {
            prices.append("a80=" + QString::number(qPrices->value(9).toDouble()));
        }
        if(qPrices->value(10).toInt() > 0) {
            prices.append("a92=" + QString::number(qPrices->value(10).toDouble()));
        }
        if(qPrices->value(11).toInt() > 0) {
            prices.append("a92_premium=" + QString::number(qPrices->value(11).toDouble()));
        }
        if(qPrices->value(12).toInt() > 0) {
            prices.append("a95=" + QString::number(qPrices->value(12).toDouble()));
        }
        if(qPrices->value(13).toInt() > 0) {
            prices.append("a95_premium=" + QString::number(qPrices->value(13).toDouble()));
        }
        if(qPrices->value(14).toInt() > 0) {
            prices.append("a98=" + QString::number(qPrices->value(14).toDouble()));
        }
        if(qPrices->value(15).toInt() > 0) {
            prices.append("a98_premium=" + QString::number(qPrices->value(15).toDouble()));
        }
        if(qPrices->value(16).toInt() > 0) {
            prices.append("a100=" + QString::number(qPrices->value(16).toDouble()));
        }
        if(qPrices->value(17).toInt() > 0) {
            prices.append("a100_premium=" + QString::number(qPrices->value(17).toDouble()));
        }
        if(qPrices->value(18).toInt() > 0) {
            prices.append("propane=" + QString::number(qPrices->value(18).toDouble()));
        }
        if(qPrices->value(19).toInt() > 0) {
            prices.append("metan=" + QString::number(qPrices->value(19).toDouble()));
        }
    }
    delete qPrices;
    aCityMobil.SetPriceList(prices.join("&").replace(",","."));
}

void GetRequests(CityMobileAPI &aCityMobil, QSqlDatabase aDb) {
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

    aCityMobil.GetRequests();
    VerifyRequests(aCityMobil, aDb);
}
#define StandartRequestsEnd }

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    setlocale(LC_ALL, "");
    QSqlDatabase _db = QSqlDatabase::addDatabase("QODBC3");
    QStringList setting;
    if (QFile::exists("Setting.txt")) {
        QFile settings("Setting.txt");
        if (settings.open(QIODevice::ReadOnly)) {
            while (!settings.atEnd()) {
                setting << QString::fromLocal8Bit(settings.readLine()).remove("\r\n").remove("\n");
            }
            settings.close();
        } else
            qDebug()<<"Error: setting file is already open";
    } else
        qDebug()<<"Error: setting file not found";
    _db.setDatabaseName(QString("DRIVER={SQL Server};"
                                "SERVER=%1;"
                                "DATABASE=agzs;"
                                "Persist Security Info=true;"
                                "uid=sa;"
                                "pwd=Gfhf743Djpbr").arg(setting[0]));
    CityMobileAPI _cityMobile("https://terminal.api.dev.fuelup.ru/",setting[1], _db);
    if (_db.open()) {
        qDebug() << "db open" << setting[1];
        QTimer timer;
        QEventLoop loop;
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(10000);
        while (true) {
            loop.exec();
            SetAZSData(_cityMobile, _db);
            SetPriceList(_cityMobile, _db);
            GetRequests(_cityMobile, _db);
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
