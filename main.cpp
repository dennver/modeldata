#include <QtWidgets/QApplication>
#include "tablewidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TableWidget w;
    w.resize(800, 600);
    w.show();
    return a.exec();
}