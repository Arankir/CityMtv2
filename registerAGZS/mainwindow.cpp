#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), _manager(new QNetworkAccessManager())
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete _manager;
    delete ui;
}


void MainWindow::on_pushButton_clicked(){
    QEventLoop loop;
    connect(_manager,&QNetworkAccessManager::finished,&loop,&QEventLoop::quit);
    QNetworkReply *reply = _manager->get(QNetworkRequest(QUrl("https://terminal.api.fuelup.ru/api/auth?login="+ui->lineEditUUID->text())));
    qDebug()<<"https://terminal.api.fuelup.ru/api/auth?login="+ui->lineEditUUID->text();
    loop.exec();
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    qDebug()<<1<<statusCode.toInt();
    if (statusCode.toInt() == 200) {
        reply = _manager->post(QNetworkRequest(QUrl("https://terminal.api.fuelup.ru/api/auth?login="+ui->lineEditUUID->text()+"&code="+ui->lineEditAPI_KEY->text())), QByteArray());
        loop.exec();
        statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
        qDebug()<<2<<statusCode.toInt();
        qDebug()<<reply->attribute( QNetworkRequest::HttpStatusCodeAttribute )<<reply->rawHeaderList()<<reply->rawHeaderPairs()<<reply->rawHeader("Authorization")<<reply->readAll();
        if (statusCode.toInt() == 200) {
            statusCode = reply->rawHeader("Authorization");
            ui->lineEditResult->setText(statusCode.toString());
        } else {
            ui->lineEditResult->setText("Ошибка пароля " + statusCode.toString());
            QMessageBox::warning(0,"Ошибка!","Не правильно указан api_key");
        }
    } else {
        ui->lineEditResult->setText("Ошибка логина " + statusCode.toString());
        QMessageBox::warning(0,"Ошибка!","Не найден uuid");
    }
    delete reply;
}
