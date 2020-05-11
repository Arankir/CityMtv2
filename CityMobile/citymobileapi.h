#ifndef CITYMOBILEAPI_H
#define CITYMOBILEAPI_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class CityMobileAPI : public QObject
{
    Q_OBJECT
public:
    explicit CityMobileAPI(QObject *parent = nullptr);
    ~CityMobileAPI();
    static void SetAZSData(QString api_key, QJsonDocument post);
    static void SetPriceList(QString api_key, QString post);
    void GetRequests(QString api_key);
    static void SetRequestStateAccept(QString api_key, QString orderId);
    static void SetRequestStateFueling(QString api_key, QString orderId);
    static void SetRequestStateCanceled(QString api_key, QString orderId, QString reason, QString extendedOrderId, QDateTime extendedDate);
    static void SetRequestStateCompleted(QString api_key, QString orderId, double litre, QString extendedOrderId, QDateTime extendedDate);
    static void SetRequestStateFuelNow(QString api_key, QString orderId, double litre);
    static void SetRequestMessage(QString api_key, QString orderId, QString msg);

signals:
    void s_finished(QNetworkReply *reply);
private:
    QNetworkAccessManager *_manager;
    QString _baseUrl="https://terminal.api.dev.fuelup.ru/"; //https://terminal.api.fuelup.ru/
};

#endif // CITYMOBILEAPI_H
