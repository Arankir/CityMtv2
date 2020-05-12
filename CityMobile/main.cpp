#include <QCoreApplication>
#include <citymobileapi.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
void SmashData(QString a_data, int *a_sideAdress, int *a_nozzle, int *a_trkFuelCode, int *a_trkVCode){
    int data=a_data.toInt();
    *a_sideAdress=data/100000; data%=100000;
    *a_nozzle=data/10000; data%=10000;
    *a_trkFuelCode=data/1000; data%=1000;
    *a_trkVCode=data;
}

void GetColumnsFuelsData(QJsonArray *a_fuels, QJsonObject *a_columns, QSqlDatabase a_db, QString a_vCodeAgzs){
    bool FuelsInc[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
    QSqlQuery* q2 = new QSqlQuery(a_db);
    q2->exec("SELECT d.AGZSName, d.VCode, c.ColumnNumber, c.diesel, c.diesel_premium, c.a80, c.a92, c.a92_premium, c.a95, c.a95_premium, c.a98, c.a98_premium, "
             "c.a100, c.a100_premium, c.propane, c.metan, c.TrkVCode"
             "FROM [agzs].[dbo].[PR_AGZSColumnsData] c INNER JOIN [agzs].[dbo].PR_AGZSData d ON d.VCode = c.Link WHERE (((d.VCode)="+a_vCodeAgzs+")) order by c.ColumnNumber asc");
    while (q2->next()) {
        QJsonObject Column;
        QJsonArray Fuels;
        if(q2->value(3).toInt()>0){
            Fuels.append("diesel");
            if(!FuelsInc[0]){
                QJsonObject FuelName;
                FuelName["Fuel"]="diesel";
                FuelName["FuelName"]="ДТ";
                a_fuels->append(FuelName);
                FuelsInc[0]=true;
            }
        }
        if(q2->value(4).toInt()>0){
            Fuels.append("diesel_premium");
            if(!FuelsInc[1]){
                QJsonObject FuelName;
                FuelName["Fuel"]="diesel_premium";
                FuelName["FuelName"]="ДТ Премиум";
                a_fuels->append(FuelName);
                FuelsInc[1]=true;
            }
        }
        if(q2->value(5).toInt()>0){
            Fuels.append("a80");
            if(!FuelsInc[2]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a80";
                FuelName["FuelName"]="АИ-80";
                a_fuels->append(FuelName);
                FuelsInc[2]=true;
            }
        }
        if(q2->value(6).toInt()>0){
            Fuels.append("a92");
            if(!FuelsInc[3]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a92";
                FuelName["FuelName"]="АИ-92";
                a_fuels->append(FuelName);
                FuelsInc[3]=true;
            }
        }
        if(q2->value(7).toInt()>0){
            Fuels.append("a92_premium");
            if(!FuelsInc[4]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a92_premium";
                FuelName["FuelName"]="АИ-92 Премиум";
                a_fuels->append(FuelName);
                FuelsInc[4]=true;
            }
        }
        if(q2->value(8).toInt()>0){
            Fuels.append("a95");
            if(!FuelsInc[5]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a95";
                FuelName["FuelName"]="АИ-95";
                a_fuels->append(FuelName);
                FuelsInc[5]=true;
            }
        }
        if(q2->value(9).toInt()>0){
            Fuels.append("a95_premium");
            if(!FuelsInc[6]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a95_premium";
                FuelName["FuelName"]="АИ-95 Премиум";
                a_fuels->append(FuelName);
                FuelsInc[6]=true;
            }
        }
        if(q2->value(10).toInt()>0){
            Fuels.append("a98");
            if(!FuelsInc[7]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a98";
                FuelName["FuelName"]="АИ-98";
                a_fuels->append(FuelName);
                FuelsInc[7]=true;
            }
        }
        if(q2->value(11).toInt()>0){
            Fuels.append("a98_premium");
            if(!FuelsInc[8]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a98_premium";
                FuelName["FuelName"]="АИ-98 Премиум";
                a_fuels->append(FuelName);
                FuelsInc[8]=true;
            }
        }
        if(q2->value(12).toInt()>0){
            Fuels.append("a100");
            if(!FuelsInc[9]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a100";
                FuelName["FuelName"]="АИ-100";
                a_fuels->append(FuelName);
                FuelsInc[9]=true;
            }
        }
        if(q2->value(13).toInt()>0){
            Fuels.append("a100_premium");
            if(!FuelsInc[10]){
                QJsonObject FuelName;
                FuelName["Fuel"]="a100_premium";
                FuelName["FuelName"]="АИ-100 Премиум";
                a_fuels->append(FuelName);
                FuelsInc[10]=true;
            }
        }
        if(q2->value(14).toInt()>0){
            Fuels.append("propane");
            if(!FuelsInc[11]){
                QJsonObject FuelName;
                FuelName["Fuel"]="propane";
                FuelName["FuelName"]="Сжиженный газ";
                a_fuels->append(FuelName);
                FuelsInc[11]=true;
            }
        }
        if(q2->value(15).toInt()>0){
            Fuels.append("metan");
            if(!FuelsInc[12]){
                QJsonObject FuelName;
                FuelName["Fuel"]="metan";
                FuelName["FuelName"]="Метан";
                a_fuels->append(FuelName);
                FuelsInc[12]=true;
            }
        }
        (*a_columns)[q2->value(16).toString()]=Column;
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
            GetColumnsFuelsData(&FuelNames, &Columns, a_db, q->value(2).toString());

            AGZS["Columns"]=Columns;
            AGZS["FuelNames"]=FuelNames;
            AGZSs.append(AGZS);
        }
    }
    QJsonDocument doc;
    doc.setArray(AGZSs);
    delete q;
    CityMobileAPI::SetAZSData(a_api_key,doc);
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
    CityMobileAPI::SetPriceList(a_api_key,prices.join("&").replace(",","."));
}

void GetRequests(QString a_api_key){
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
}

void SetRequestStateAccept(QString a_api_key, QString a_orderId){

}
void SetRequestStateFueling(QString a_api_key, QString a_orderId){

}
void SetRequestStateCanceled(QString a_api_key, QString a_orderId, QString a_reason, QString a_extendedOrderId, QDateTime a_extendedDate){

}
void SetRequestStateCompleted(QString a_api_key, QString a_orderId, double a_litre, QString a_extendedOrderId, QDateTime a_extendedDate){

}
void SetRequestStateFuelNow(QString a_api_key, QString a_orderId, double a_litre){

}
void SetRequestMessage(QString a_api_key, QString a_orderId, QString a_msg){

}

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

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
//        _timer.setInterval(60000);
//        QObject::connect(&_timer,SIGNAL(timeout()),this,SLOT(SendFuel()));
//        _timer.start();
    }
    else {
        qDebug() << "db close";
    }

    return a.exec();
}
