#include "customtablemodel.h"
#include <QtCore/QVector>
#include <QtCore/QRandomGenerator>
#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QMimeData>
#include <QIODevice>
#include <QDataStream>

struct ItemMime
{
    ItemMime() {}
    ItemMime(const QVariant &data, int row, int column)
    {
        this->data = data;
        this->row = row;
        this->column = column;
    }
    int row;
    int column;
    QVariant data;
};

QDataStream &operator<<(QDataStream &out, const ItemMime &item)
{
    out << item.row
        << item.column
        << item.data;
    return out;
}

QDataStream &operator>>(QDataStream &in, ItemMime &item) {
    QVariant var;
    in >> item.row >> item.column >> item.data;
    return in;
}

CustomTableModel::CustomTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    m_columnCount = 4;
    m_rowCount = 15;

    // m_data
    for (int i = 0; i < m_rowCount; i++) {
        QVector<QVariant>* dataVec = new QVector<QVariant>(m_columnCount);
        for (int k = 0; k < dataVec->size(); k++) {
            if (k % 2 == 0)
                dataVec->replace(k, i * 50 + QRandomGenerator::global()->bounded(20));
            else
                dataVec->replace(k, QRandomGenerator::global()->bounded(100));
        }
        m_data.append(dataVec);
    }
}

int CustomTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.count();
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_columnCount;
}

QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    if (role != Qt::DisplayRole)
        return QVariant();

    return QString("%1").arg(section + 1);
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_data[index.row()]->at(index.column());
    } else if (role == Qt::EditRole) {
        return m_data[index.row()]->at(index.column());
    }
    return QVariant();
}

bool CustomTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()]->replace(index.column(), value.toDouble());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CustomTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    m_rowCount++;
    for (auto i = 0; i < count ; ++i) {
        m_data.insert(row, new QVector<QVariant>(m_rowCount, QVariant(0)));
    }
    endInsertRows();
    return true;
}

bool CustomTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    m_rowCount--;
    for (auto i = 0; i < count; ++i) {
        m_data.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool CustomTableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    m_columnCount++;
    for (auto i : m_data) {
        for (auto j = 0; j < count; ++j) {
            i->insert(column, QVariant(0));
        }
    }
    endInsertColumns();
    return true;
}

bool CustomTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    m_columnCount++;
    for (auto i : m_data) {
        for (auto j = 0; j < count; ++j) {
            i->remove(column, count);
        }
    }
    endRemoveColumns();
    return true;
}

bool CustomTableModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    bool b = true;
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
        b = b && setData(index, it.value(), it.key());
    return b;
}

QStringList CustomTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/custom.variant.list";
    return types;
}

QMimeData *CustomTableModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            ItemMime item(data(index, Qt::DisplayRole), index.row(), index.column());
            stream << item;
        }
    }

    mimeData->setData("application/custom.variant.list", encodedData);
    return mimeData;
}

bool CustomTableModel::dropMimeData(const QMimeData *data,
     Qt::DropAction action, int row, int column, const QModelIndex &parent)
 {
     if (action == Qt::IgnoreAction)
         return true;

     if (!data->hasFormat("application/custom.variant.list"))
         return false;

     if (column > m_columnCount)
         return false;

     int beginRow;

     if (row != -1)
         beginRow = row;
     else if (parent.isValid())
         beginRow = parent.row();
     else
         beginRow = rowCount(QModelIndex());

     QByteArray encodedData = data->data("application/custom.variant.list");
         QDataStream stream(&encodedData, QIODevice::ReadOnly);
         QMultiMap<int, ItemMime> newItems;

         while (!stream.atEnd()) {
             ItemMime item;
             stream >> item;
             newItems.insert(item.row, item);
         }
         // we could insert new rows and fill them up
//         insertRows(beginRow, newItemss.uniqueKeys().count(), QModelIndex());

         // keep row and column order of elements, few modifications
     QMapIterator<int, ItemMime> i(newItems);
     int key = newItems.firstKey();
     while (i.hasNext()) {
         i.next();
         if (i.key() > key) {
             ++beginRow;
             key = i.key();
         }
         QModelIndex idx = index(beginRow, i.value().column, QModelIndex());
         setData(idx, i.value().data);
     }

     return true;
}

Qt::DropActions CustomTableModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
