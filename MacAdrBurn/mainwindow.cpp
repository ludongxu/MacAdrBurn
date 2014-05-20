#include <stdio.h>
#include <errno.h>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    connect(ui->pushButton_burn, SIGNAL(clicked()), this, SLOT(burnMacAdr()));
    connect(ui->pushButton_get, SIGNAL(clicked()), this, SLOT(getMacAdr()));

    //init network
    mNetManager = new QNetworkAccessManager(this);
    QObject::connect(mNetManager, SIGNAL(finished(QNetworkReply*)),
                 this, SLOT(finishedSlot(QNetworkReply*)));


    mIsFind = false;

  //  mCheckTimer = new QTimer(this);
  //  connect(mCheckTimer, SIGNAL(timeout()), this, SLOT(timerProcess()) );
  //  mCheckTimer->start(3000);

   //autoScroll
   // connect(this, SIGNAL(textChanged()), this, SLOT(autoScroll()));


}

MainWindow::~MainWindow()
{
    delete ui;
    delete mNetManager;
   // delete mCheckTimer;
}

bool MainWindow::check_input_legal(const char *input, int maxlen)
{
    int len;
    bool islegal = input;
    len = strlen(input);

    if (len != maxlen) {
        return false;
    }

    for (int i = 0; i < len; i++) {
     //   qDebug("mac: i: %d, %c", i, input[i]);
        if (!isxdigit(input[i])) {
            islegal = false;
            break;
        }
    }
    return islegal;
}

#if 0
bool
MainWindow::check_mac_legal(const char *mac)
{
    int len;
    bool islegal = true;
    len = strlen(mac);

    if (len != 12) {
        return false;
    }

    for (int i = 0; i < len; i++) {
        qDebug("mac: i: %d, %c", i, mac[i]);
        if (!isxdigit(mac[i])) {
            islegal = false;
            break;
        }
    }
    return islegal;
}

bool MainWindow::check_pid_legal(const char *pid)
{
    int len = strlen(pid);
    if (len == 0) {
        return false;
    }

    return true;
}

#endif



void
MainWindow::burnMacAdr()
{
    char buff[1024];
    char mac_addr[18];
    QString  macstr, pidstr;
    const char *p_mac = NULL;
    const char *p_pid = NULL;
    QByteArray mac_array, pid_array;
    qDebug("start burn\n");
    ui->textEdit->append("start burn");

    Device_State state = getMacAdr();

    if (state != NOT_BURNED)
        return;
    qDebug("count: %d\n", ui->lineEdit_mac->text().count());
    qDebug("count: %d\n", ui->lineEdit_pid->text().count());
    macstr = ui->lineEdit_mac->text();
    mac_array = macstr.toLocal8Bit();
    p_mac = mac_array.constData();

    pidstr = ui->lineEdit_pid->text();
    pid_array = pidstr.toLocal8Bit();
    p_pid = pid_array.constData();

    bool mac_legal = check_input_legal(p_mac, 12);
    if (mac_legal == false) {
        ui->textEdit->setTextColor(QColor(255, 0, 0));

        ui->textEdit->append("error mac address");
        ui->textEdit->setTextColor(QColor(0, 0, 255));
        return;
    }

    bool pid_legal = check_input_legal(p_pid, 2);
    if (pid_legal == false) {
        ui->textEdit->setTextColor(QColor(255, 0, 0));

        ui->textEdit->append("error pid number");
        ui->textEdit->setTextColor(QColor(0, 0, 0));
        return;
    }
    qDebug("mac_legal: %d\n", mac_legal);

    for (int i = 0, j = 0; i < 12; i += 2, j+=3) {
        qDebug("mac_addr: %d, %c, %c\n", i,  p_mac[i], p_mac[i+1]);

        mac_addr[j] = p_mac[i];
        mac_addr[j + 1] = p_mac[i + 1];
        if (i == 10)
            continue;
        mac_addr[j + 2] = ':';

    }


    mac_addr[17] = '\0';

    qDebug("mac addr: %s\n", mac_addr);

    sprintf(buff, "http://115.29.198.3:8080/szbox/service!submitMac.htm?id=%s&mac=%s", p_pid, mac_addr);
    qDebug("url: %s\n", buff);
    memcpy(mMacAdr, mac_addr, sizeof(mac_addr));

    QUrl url(buff);
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(url));

}

Device_State
MainWindow::getMacAdr()
{
    char buff[1024];
    qDebug("get mac addr");
   ui->textEdit->append("get mac");
    FILE *stream =  popen("CommandUSB.exe -r ETHMAC", "r");
    if (NULL == stream) {
        perror("popen");
    } else {
        memset(buff, 0, sizeof(buff));
        fread(buff, sizeof(buff), 1, stream);
        ui->textEdit->append(buff);
        pclose(stream);
        if (strstr(buff, "ETHMAC")) {
            qDebug("length: %d\n", strlen(buff));
            if (strlen(buff) < 19) {
                ui->textEdit->append("no burn");
                return NOT_BURNED;
            } else {
                ui->textEdit->append("already burn");
                QMessageBox::about(NULL, "ERROR", "About this <font color='red'>already burn</font>");
                return BURNED;
            }
        } else {
            ui->textEdit->append("did not find device");
            QMessageBox::about(NULL, "ERROR", "About this <font color='red'>did not find device</font>");
            return NOT_FIND_DEVICE;
        }
    }
}


void
MainWindow::finishedSlot(QNetworkReply *reply)
{
     char buff[1024];
     char cmd[1024];
     // Reading attributes of the reply
     QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
     // Or the target URL if it was a redirect:
     QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

     if (reply->error() == QNetworkReply::NoError) {
         // read data from QNetworkReply here
         QByteArray bytes = reply->readAll();  // bytes
         QString string = QString::fromUtf8(bytes);
         if (string.contains("true")) {
             ui->textEdit->setTextColor(QColor(0, 0, 255));

             ui->textEdit->append("mac address start burn ...");
             ui->textEdit->setTextColor(QColor(0, 0, 0));
             sprintf(cmd, "CommandUSB.exe -w -s=0 ETHMAC=%s", mMacAdr);
             FILE *stream =  popen(cmd, "r");
             if (NULL == stream) {
                 ui->textEdit->append(("burn error\n"));
                 perror("popen");
                // qDebug("run cmd error: %s\n", );
             } else {
                 memset(buff, 0, sizeof(buff));
                 fread(buff, sizeof(buff), 1, stream);

                 ui->textEdit->append(buff);
                 ui->textEdit->append("burn sucess\n");
                 pclose(stream);
                 mIsFind = false;
             }

         } else {
             ui->textEdit->setTextColor(QColor(0, 255, 0));

             ui->textEdit->append("mac address already used");
             ui->textEdit->setTextColor(QColor(0, 0, 0));
         }
         qDebug("resp: %s\n", string.toLatin1().data());
     } else {
         // handle errors here
     }

     // We receive ownership of the reply object
     // and therefore need to handle deletion.
     reply->deleteLater();
}

void MainWindow::timerProcess()
{
    qDebug("timer....\n");
    char buff[1024];

    if (mIsFind == true)
        return;

    FILE *stream =  popen("CommandUSB.exe -r ETHMAC", "r");
    if (NULL == stream) {
        perror("popen");
        mIsFind = false;
       // qDebug("run cmd error: %s\n", );
    } else {
        if (mIsFind == false) {
            memset(buff, 0, sizeof(buff));
            fread(buff, sizeof(buff), 1, stream);
            ui->textEdit->append(buff);
            ui->textEdit->append("find a device");
            pclose(stream);
            mIsFind = true;
        }
    }

}

#if 0
void MainWindow::autoScroll()
{
    qDebug("autoScroll\n");
}
#endif
