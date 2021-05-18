#include "qlistviewglossarydemodel.h"
#include "glossaryde.h"

QListViewGlossaryDeModel::QListViewGlossaryDeModel(GlossaryDe &aGlossary, QObject *parent)
    :QAbstractListModel(parent), glossary(&aGlossary)
{

}

int QListViewGlossaryDeModel::rowCount(const QModelIndex &) const
{
    return this->glossary->size();
}

QVariant QListViewGlossaryDeModel::data(const QModelIndex &index, int role) const
{
    QVariant value;

    switch ( role )
    {
        case Qt::DisplayRole: //string
        {
            const WortDe &wd = this->glossary->at(index.row());
            const std::string prefix = wd.prefix();
            const std::string wort = prefix.empty() ? wd.wort() : prefix + ' ' + wd.wort();
            value = QString::fromStdString(wort + " - " + wd.translation());
            break;
        }
        case Qt::UserRole: //data
        {
            const WortDe &wd = this->glossary->at(index.row());
            const std::string prefix = wd.prefix();
            const std::string wort = prefix.empty() ? wd.wort() : prefix + ' ' + wd.wort();
            value = QString::fromStdString(wort + " - " + wd.translation());
            break;
        }
        default:
            break;
    }

    return value;
}

void QListViewGlossaryDeModel::upDate(size_t idxBegin, size_t idxLast)
{
    if (idxBegin >= glossary->size())
        idxBegin = glossary->size() > 0 ? glossary->size() - 1 : 0;
    if (idxLast >= glossary->size())
        idxLast = glossary->size() > 0 ? glossary->size() - 1 : 0;
    this->dataChanged(QAbstractItemModel::createIndex(idxBegin, 0), QAbstractItemModel::createIndex(idxLast, 0) );
}

void QListViewGlossaryDeModel::upDate()
{
    upDate(0, glossary->size());
}

QModelIndex QListViewGlossaryDeModel::creatNewIndex(size_t idx)
{
    QModelIndex index = createIndex(idx, 0);
    return index;
}
