#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QPersistentModelIndex>

class TableWidget : public QWidget
{
    Q_OBJECT

public:
    TableWidget(QWidget *parent = nullptr);

private slots:
    void customMenuRequested(const QPoint &pos);
    bool insertColumn();
    void insertRow();
    bool removeColumn();
    void removeRow();
    void copyElements();
    void pasteElements();

signals:
    void copyAble(bool);

private:
    QTableView *m_tableView;
    QList<QPersistentModelIndex> m_copyList;

};

#endif // TABLEWIDGET_H
