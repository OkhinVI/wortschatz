#ifndef QLISTVIEWGLOSSARYDEMODEL_H
#define QLISTVIEWGLOSSARYDEMODEL_H

#include <QModelIndex>

class GlossaryDe;

class QListViewGlossaryDeModel: public QAbstractListModel
{
public:
    QListViewGlossaryDeModel(GlossaryDe &aGlossary, QObject *parent=nullptr);
    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;

    void upDate();
    void upDate(size_t idxBegin, size_t idxLast);

private:
    GlossaryDe *glossary = nullptr;
};

#endif // QLISTVIEWGLOSSARYDEMODEL_H
