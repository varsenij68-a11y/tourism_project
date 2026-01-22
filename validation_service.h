#pragma once

#include <QRegularExpression>
#include <QString>

#include "address.h"

class ValidationService {
public:
    static QRegularExpression nameRegex();
    static QRegularExpression addressTextRegex();
    static QRegularExpression houseRegex();
    static QRegularExpression postalCodeRegex();

    static bool validateNamePart(const QString& value, QString* err = nullptr);
    static bool validateOptionalNamePart(const QString& value, QString* err = nullptr);
    static bool validateAddress(const Address& address, QString* err = nullptr);
};
