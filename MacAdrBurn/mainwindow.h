#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

typedef enum Device_State{
    BURNED = 0,
    NOT_FIND_DEVICE,
    NOT_BURNED
}Device_State;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    bool check_input_legal(const char *input, int maxlen);

public slots:
    void burnMacAdr();
    Device_State getMacAdr();
    void finishedSlot(QNetworkReply *reply);
    void timerProcess();
   // void autoScroll();
private:
  //  QTimer *mCheckTimer;
    Ui::MainWindow *ui;
    QNetworkAccessManager *mNetManager;
    bool mIsFind;
    char mMacAdr[18];
};

#endif // MAINWINDOW_H
