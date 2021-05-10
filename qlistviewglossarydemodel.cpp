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
            value = QString::fromStdString(wort);
            break;
        }
        case Qt::UserRole: //data
        {
            const WortDe &wd = this->glossary->at(index.row());
            const std::string prefix = wd.prefix();
            const std::string wort = prefix.empty() ? wd.wort() : prefix + ' ' + wd.wort();
            value = QString::fromStdString(wort);
            break;
        }
        default:
            break;
    }

    return value;
}
