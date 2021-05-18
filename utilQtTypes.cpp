#include "utilQtTypes.h"
#include "utility.h"
#include <QLineEdit>

QString utilQt::strToQt(const std::string &str)
{
    return QString::fromStdString(str);
}

std::string utilQt::strToStd(const QString &strQt)
{
    return strQt.toUtf8().toStdString();
}

std::string utilQt::lineEditToStdStr(const QLineEdit *edtQt)
{
    if (edtQt == nullptr)
        return std::string();
    std::string str = edtQt->text().toUtf8().toStdString();
    util::replaceEndLines(str);
    return AreaUtf8(str).trim().toString();
}

void utilQt::strToLineEdit(QLineEdit *edtQt, const std::string &str)
{
    if (edtQt == nullptr)
        return;

    edtQt->setText(QString::fromStdString(str));
}
