#ifndef CITYMOBILEAPI_H
#define CITYMOBILEAPI_H

#include <QObject>
#include <requestdata.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>

class CityMobileAPI : public QObject {
    Q_OBJECT
public:
    explicit CityMobileAPI(QString bazeUrl, QString apiKey, QSqlDatabase db, QObject *parent = nullptr);
    ~CityMobileAPI();
    void SetAZSData(QJsonDocument post);
    void SetPriceList(QString post);
    void GetRequests();
    void SetRequestStateAccept(QString orderId);
    void SetRequestStateFueling(QString orderId);
    void SetRequestStateCanceled(QString orderId, QString reason, QString extendedOrderId, QDateTime extendedDate);
    void SetRequestStateCompleted(QString orderId, double litre, QString extendedOrderId, QDateTime extendedDate);
    void SetRequestStateFuelNow(QString orderId, double litre);
    void SetRequestMessage(QString orderId, QString msg);
    CityMobileAPI(const CityMobileAPI &achievement);
    CityMobileAPI &operator=(const CityMobileAPI&);

signals:
    void s_finished(RequestData *request);

private slots:
    QNetworkRequest createRequest(QString url, QString contentType, bool auth);

    void checkOrders(RequestData *aRequest);
    void insertNewRequest(QJsonObject aRequest);
    int getCashBoxIndex();
    QString getSmena();
    int getLastAPI();
    int checkError(QString aStationID, QString aColumnID, QString aFuelID, QString aPriceFuel, QString aId, QString aLastVCode, QDateTime aNow);
    void getMoneyData(QJsonObject aData, double &aRequestTotalPriceDB, double &aRequestVolumeDB, double &aRequestUnitPriceDB, double &aMoneyTakenDB, int &aFullTankDB);
    QString getFuelAPIName(int aFuelVCode);
    int getFuelID(QString aFuelIdApi);
    QString getFullFuelName(int aFuelId);
private:
    QString _bazeUrl;
    RequestData *_request;
    QString _apiKey;
    QSqlDatabase _db;
};

#endif // CITYMOBILEAPI_H
