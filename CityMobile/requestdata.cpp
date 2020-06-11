#include "requestdata.h"

RequestData::RequestData(QString aUrl, bool aParallel, QObject *aParent): QObject(aParent), _manager(new QNetworkAccessManager()), _answer(""),
_authorization(""), _url(aUrl), _parallel(aParallel) {
    connect(_manager,&QNetworkAccessManager::finished,this,&RequestData::onResult);
    setlocale(LC_ALL, "");
}

RequestData::RequestData(QObject *aParent): RequestData("", false, aParent) {

}

void RequestData::get(QString aUrl, bool aParallel) {
    get(QNetworkRequest(QUrl(aUrl)), aParallel);
}

void RequestData::get(QNetworkRequest aRequest, bool aParallel) {
    completeRequest(RequestType::get, aRequest, "", aParallel);
}

void RequestData::post(QNetworkRequest aRequest, QString aPost, bool aParallel) {
    completeRequest(RequestType::post, aRequest, aPost, aParallel);
}

QByteArray RequestData::getAnswer() {
    return _answer;
}

QByteArray RequestData::getAuthorization() {
    return _authorization;
}

int RequestData::getCode() {
    return _code;
}

void RequestData::completeRequest(RequestData::RequestType aType, QNetworkRequest aRequest, QString aPost, bool aParallel) {
    qDebug() << "Запрос:" << aRequest.url() << aPost.toUtf8();
    switch (aType) {
    case RequestType::get: {
        _manager->get(aRequest);
        break;
    }
    case RequestType::post: {
        _manager->post(aRequest, aPost.toUtf8());
        break;
    }
    }
    if (!aParallel) {
        QEventLoop loop;
        connect(_manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
        loop.exec();
        disconnect(_manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    }
}

void RequestData::onResult(QNetworkReply *aReply) {
    _code=aReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"Код:"<< _code;
    _answer=aReply->readAll();
    if (aReply->rawHeaderList().indexOf("Authorization") > -1) {
        _authorization=aReply->rawHeader("Authorization");
    }
    emit s_finished(this);
}

RequestData::~RequestData(){
    disconnect(_manager);
    delete _manager;
}
