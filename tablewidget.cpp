#include "tablewidget.h"
#include "customtablemodel.h"
#include <QtWidgets/QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QVXYModelMapper>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QAction>

TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent)
{
    // create table m_tableView and add model to it
    m_tableView = new QTableView(this);
    // create simple model for storing data
    CustomTableModel *model = new CustomTableModel(m_tableView);
    m_tableView->setModel(model);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->setDragEnabled(true);
    m_tableView->setAcceptDrops(true);
    m_tableView->setDropIndicatorShown(true);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // fill in actions
    m_tableView->addAction(new QAction(tr("Add row"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::insertRow);
    m_tableView->addAction(new QAction(tr("Remove row"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::removeRow);
    m_tableView->addAction(new QAction(tr("Add column"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::insertColumn);
    m_tableView->addAction(new QAction(tr("Remove column"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::removeColumn);
    m_tableView->addAction(new QAction(tr("Copy"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::copyElements);
    m_tableView->addAction(new QAction(tr("Paste"), m_tableView));
    connect(m_tableView->actions().last(), &QAction::triggered, this, &TableWidget::pasteElements);
    m_tableView->actions().last()->setEnabled(false);
    connect(this, &TableWidget::copyAble, m_tableView->actions().last(), &QAction::setEnabled);

    // custom context menu
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView, &QTableView::customContextMenuRequested, this, &TableWidget::customMenuRequested);

    // create main layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(m_tableView, 1, 0);
    setLayout(mainLayout);
}

void TableWidget::customMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    for (auto action : m_tableView->actions()) {
        menu->addAction(action);
    }
    menu->popup(m_tableView->viewport()->mapToGlobal(pos));
}


bool TableWidget::insertColumn()
{
    QAbstractItemModel *model = m_tableView->model();
    int column = m_tableView->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
    bool changed = model->insertColumn(column + 1);
//    if (changed)
//        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

//    for (int row = 0; row < model->rowCount(index.parent()); ++row) {
//        QModelIndex child = model->index(row, index.column()+1, index.parent());
//        model->setData(child, QVariant(0), Qt::EditRole);
//    }

    return changed;
}

void TableWidget::insertRow()
{
    QModelIndex index = m_tableView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_tableView->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

//    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
//        QModelIndex child = model->index(index.row()+1, column, index.parent());
//        model->setData(child, QVariant(0), Qt::EditRole);
//    }
}

bool TableWidget::removeColumn()
{
    QAbstractItemModel *model = m_tableView->model();
    int column = m_tableView->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    bool changed = model->removeColumn(column);
    return changed;
}

void TableWidget::removeRow()
{
    QModelIndex index = m_tableView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_tableView->model();
    model->removeRow(index.row(), index.parent());
}

void TableWidget::copyElements()
{
    QModelIndexList indexList = m_tableView->selectionModel()->selectedIndexes();
    for (auto item : indexList) {
        m_copyList.push_back(QPersistentModelIndex(item));
    }

    emit copyAble(true);
}

void TableWidget::pasteElements()
{
    QModelIndex index = m_tableView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_tableView->model();

    for (int i = 0; i < m_copyList.size(); i++) {
            int currentColumn = (index.column() + i) % (model->columnCount());
            int currentRow = (index.row()) + (index.column() + i) / (model->columnCount());
            QModelIndex child = model->index(currentRow, currentColumn, index.parent());
            QPersistentModelIndex dataIndex = m_copyList.value(i);
            if (dataIndex.isValid()) {
                QVariant data = dataIndex.data(Qt::EditRole);
                model->setData(child, data, Qt::EditRole);
            }
        }
}
