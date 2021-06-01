#ifndef QLISTVIEWGLOSSARYDEMODEL_H
#define QLISTVIEWGLOSSARYDEMODEL_H

#include <QModelIndex>
#include <vector>
#include <string>
#include "glossaryde.h"

class GlossaryDe;

class QListViewGlossaryDeModel: public QAbstractListModel
{
public:
    QListViewGlossaryDeModel(GlossaryDe &aGlossary, QObject *parent=nullptr);
    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;

    void upDate();
    void upDate(size_t idxBegin, size_t idxLast);
    QModelIndex creatNewIndex(size_t idx);

private:
    GlossaryDe *glossary = nullptr;
};

class QListViewGlossaryTemaModel: public QAbstractListModel
{
public:
    QListViewGlossaryTemaModel(GlossaryDe::SelectSettings &aSelectThemes, QObject *parent=nullptr);
    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    GlossaryDe::SelectSettings &selectThemes;
};

#endif // QLISTVIEWGLOSSARYDEMODEL_H
