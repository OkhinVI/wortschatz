#ifndef UTILQTTYPES_H
#define UTILQTTYPES_H

#include "string_utf8.h"
#include <QString>
#include <QLineEdit>

//class QLineEdit;

namespace utilQt
{

QString strToQt(const std::string &str);

std::string strToStd(const QString &strQt);

std::string lineEditToStdStr(const QLineEdit *edtQt);

void strToLineEdit(QLineEdit *edtQt, const std::string &str);

} // namespace utilQt


#endif // UTILQTTYPES_H
