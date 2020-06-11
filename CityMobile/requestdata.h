#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCodec>
#include <QTcpSocket>
#include <QEventLoop>
#include <QDir>
#include <QHttpMultiPart>
#include <QUrl>
#include <QUrlQuery>

class RequestData : public QObject {
    Q_OBJECT
    enum class RequestType {
        get,
        post
    };

public:
    explicit RequestData(QString url, bool parallel=false, QObject *parent = nullptr);
    RequestData(QObject *parent = nullptr);
    ~RequestData();
    void get(QString str, bool parallel = false);
    void get(QNetworkRequest request, bool parallel = false);
    void post(QNetworkRequest request, QString post, bool parallel = false);
    QByteArray getAnswer();
    QByteArray getAuthorization();
    int getCode();

signals:
    void s_finished(RequestData *request);

private slots:
    void completeRequest(RequestType type, QNetworkRequest request, QString post, bool parallel);
    void onResult(QNetworkReply *reply);

private:
    QNetworkAccessManager *_manager;
    QByteArray _answer;
    int _code;
    QByteArray _authorization;
    QString _url;
    bool _parallel;

};

#endif // IMAGEREQUEST_H
