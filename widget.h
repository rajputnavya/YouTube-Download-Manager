#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    QLabel *videoInfoLabel;
    QPushButton *downloadButton;


private:
    Ui::Widget *ui;
    //+
    QNetworkAccessManager *manager;

    void fetchVideoInfo(const QString &videoLink, QWidget *window);
    void downloadVideo(const QString &videoLink,QWidget *window,QLabel *progressLabel);

};
#endif // WIDGET_H
