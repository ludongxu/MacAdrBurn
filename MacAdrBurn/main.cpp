#include "mainwindow.h"
#include <QApplication>
 #include <QTextCodec>
//#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   MainWindow w;

   QTextCodec::setCodecForTr(QTextCodec::codecForName("GB2312"));
   QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
   QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    w.show();
    return a.exec();
}
