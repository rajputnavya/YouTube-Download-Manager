#include "widget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QNetworkReply>
#include <QJsonParseError>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("YouTube Download Manager", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:28px; color: red");
    layout->addWidget(titleLabel);

    QLabel *instructionLabel = new QLabel("Please Input YouTube Video Link to Download", this);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size:20px; color: green");
    layout->addWidget(instructionLabel);

    QTextEdit *linkInput = new QTextEdit(this);
    linkInput->setFixedSize(500,45);
    linkInput->setStyleSheet("background-color:lightgray; font-size: 18px");
    linkInput->setAcceptRichText(false);
    layout->addWidget(linkInput,0,Qt::AlignCenter);

    QPushButton *getLinkButton = new QPushButton("Get Link", this);
    getLinkButton->setFixedSize(150,50);
    getLinkButton->setStyleSheet("background-color: blue; color: white; font-size: 20px");
    layout->addWidget(getLinkButton,0,Qt::AlignCenter);

    videoInfoLabel = new QLabel("", this);
    videoInfoLabel->setAlignment(Qt::AlignCenter);
    videoInfoLabel->setStyleSheet("font-size:18px; color: green");
    layout->addWidget(videoInfoLabel);

    downloadButton = new QPushButton("Download", this);
    downloadButton->setFixedSize(150,50);
    downloadButton->setStyleSheet("background-color: red; color: white; font-size: 20px");
    downloadButton->hide();
    layout->addWidget(downloadButton,0,Qt::AlignCenter);

    //Initialisation of QNetworkAccessManager
    manager = new QNetworkAccessManager(this);

    //Connecting both Buttons to Slots using Lambda Functions
    QObject::connect(getLinkButton, &QPushButton::clicked,[linkInput, this](){

        QString videoLink = linkInput->toPlainText();
        fetchVideoInfo(videoLink, this);

    });

    QObject::connect(downloadButton, &QPushButton::clicked,[linkInput, this](){

        QString videoLink = linkInput->toPlainText();

        QWidget *downloadWindow = new QWidget(nullptr); //
        downloadWindow->setWindowTitle("Downloading...");
        downloadWindow->resize(400,200);

        QVBoxLayout *downloadLayout = new QVBoxLayout(downloadWindow);

        QLabel *downloadingLabel = new QLabel("Downloading...", downloadWindow);
        downloadingLabel->setAlignment(Qt::AlignCenter);
        downloadingLabel->setStyleSheet("font-size:18px; color: red");
        downloadLayout->addWidget(downloadingLabel);

        QLabel *progressLabel = new QLabel("", downloadWindow);
        progressLabel->setAlignment(Qt::AlignCenter);
        progressLabel->setStyleSheet("font-size:18; color: green");
        downloadLayout->addWidget(progressLabel);

        downloadWindow->show();
        downloadButton->hide();

        downloadVideo(videoLink, downloadWindow, progressLabel);
    });
}

Widget::~Widget()
{

}

void Widget::fetchVideoInfo(const QString &videoLink, QWidget *window){
    QUrl url("http://127.0.0.1:5000/get_video_info");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QJsonObject json;
    json["video_link"] = videoLink;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, window, this](){

        if(reply->error()==QNetworkReply::NoError){
            QByteArray responseDate = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseDate);
            QJsonObject jsonObject = jsonDoc.object();
            QString videoTitle = jsonObject["title"].toString();
            videoInfoLabel->setText("Title: "+videoTitle);

            downloadButton->show();

        }
        else{
            qDebug()  << "Error Fetching video Info: "<<reply->errorString();
        }
        reply->deleteLater();

    });

}

void Widget::downloadVideo(const QString &videoLink,QWidget *window, QLabel *progressLabel){

    QUrl url("http://127.0.0.1:5000/download_video");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QJsonObject json;
    json["video_link"] = videoLink;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    QObject::connect(reply, &QNetworkReply::readyRead,[reply,window,progressLabel, this](){

        while(reply->canReadLine()){
            QString line = reply->readLine();
            if(line.startsWith("data:")){
                QString jsonText = line.mid(5).trimmed();
                QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonText.toUtf8());
                QJsonObject jsonObject = jsonDoc.object();

                if(jsonObject.contains("error")){
                    qDebug() << "Error Downloading Video: "<< jsonObject["error"].toString();
                }
                else{
                    double downloaded = jsonObject["downloaded"].toDouble();
                    double total = jsonObject["total"].toDouble();
                    double speed = jsonObject["speed"].toDouble();
                    int eta = jsonObject["eta"].toInt();

                    QString percent = jsonObject["percent"].toString();

                    QString progressText = QString("%1 of %2 MiB at %3 KiB/s ETA %4s")
                                               .arg(downloaded, 0, 'f',2)
                                               .arg(total, 0, 'f',2)
                                               .arg(speed, 0, 'f',2)
                                               .arg(eta);

                    progressLabel->setText(progressText);
                }
            }
        }

    });
    QObject::connect(reply,&QNetworkReply::finished,[reply,window](){
        if(reply->error()==QNetworkReply::NoError){
            QMessageBox::information(window,"Download Complete","Video Downloaded Successfully");
        }
        else{
            qDebug() << "Error Downloading Video: Try Again - " <<reply->errorString();
        }

        reply->deleteLater();
    });

}

