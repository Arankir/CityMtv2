#include "citymobileapi.h"
const QString c_baseUrl="https://terminal.api.dev.fuelup.ru/"; //https://terminal.api.fuelup.ru/

CityMobileAPI::CityMobileAPI(QObject *parent) : QObject(parent), _manager(new QNetworkAccessManager){}
CityMobileAPI::~CityMobileAPI(){
    delete _manager;
}

void CityMobileAPI::SetAZSData(QString a_api_key, QJsonDocument a_post){
    qDebug()<<"Data"<<a_post;
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/station"));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->post(request,QString(a_post.toJson(QJsonDocument::Compact)).toUtf8());
    this->deleteLater();
}

void CityMobileAPI::SetPriceList(QString a_api_key, QString a_post){
    qDebug()<<"Price"<<a_post;
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/price"));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->post(request,a_post.toUtf8());
    this->deleteLater();
}

void CityMobileAPI::GetRequests(QString a_api_key){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/items"));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->get(request);
    connect(_manager,&QNetworkAccessManager::finished,this,[=](QNetworkReply *reply){
        emit s_finished(reply);
        this->deleteLater();
    }
    );
}

void CityMobileAPI::SetRequestStateAccept(QString a_api_key, QString a_orderId){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/accept?orderId="+a_orderId));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->get(request);
    this->deleteLater();
}
void CityMobileAPI::SetRequestStateFueling(QString a_api_key, QString a_orderId){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/fueling?orderId="+a_orderId));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->get(request);
    this->deleteLater();
}
void CityMobileAPI::SetRequestStateCanceled(QString a_api_key, QString a_orderId, QString a_reason, QString a_extendedOrderId, QDateTime a_extendedDate){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/canceled?orderId="+a_orderId+"&reason="+a_reason+"&extendedOrderId="+a_extendedOrderId+"&extendedDate="+a_extendedDate.toString("dd.MM.yyyy HH:mm:ss")));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->get(request);
    this->deleteLater();
}
void CityMobileAPI::SetRequestStateCompleted(QString a_api_key, QString a_orderId, double a_litre, QString a_extendedOrderId, QDateTime a_extendedDate){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/completed?orderId="+a_orderId+"&litre="+QString::number(a_litre).replace(",",".")+"&extendedOrderId="+a_extendedOrderId+"&extendedDate="+a_extendedDate.toString("dd.MM.yyyy HH:mm:ss")));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->get(request);
    this->deleteLater();
}
void CityMobileAPI::SetRequestStateFuelNow(QString a_api_key, QString a_orderId, double a_litre){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/volume"));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->post(request,QString("orderId="+a_orderId+"&litre="+QString::number(a_litre).replace(",",".")).toUtf8());
    this->deleteLater();
}
void CityMobileAPI::SetRequestMessage(QString a_api_key, QString a_orderId, QString a_msg){
    QNetworkRequest request;
    request.setUrl(QUrl(c_baseUrl+"api/orders/message"));
    request.setRawHeader("Authorization",a_api_key.toUtf8());
    _manager->post(request,QString("orderId="+a_orderId+"&msg="+a_msg).toUtf8());
    this->deleteLater();
}
