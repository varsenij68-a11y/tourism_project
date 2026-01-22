#include "validation_service.h"

QRegularExpression ValidationService::nameRegex() {
    return QRegularExpression(QStringLiteral("^[А-ЯЁа-яё]+(-[А-ЯЁа-яё]+)*$"));
}

QRegularExpression ValidationService::addressTextRegex() {
    return QRegularExpression(QStringLiteral("^[А-ЯЁа-яё]+([ -][А-ЯЁа-яё]+)*$"));
}

QRegularExpression ValidationService::houseRegex() {
    return QRegularExpression(QStringLiteral("^\\d+[А-Яа-я]?(\\/\\d+[А-Яа-я]?)?$"));
}

QRegularExpression ValidationService::postalCodeRegex() {
    return QRegularExpression(QStringLiteral("^\\d{6}$"));
}

bool ValidationService::validateNamePart(const QString& value, QString* err) {
    if (value.trimmed().isEmpty()) {
        if (err) *err = "Поле обязательно";
        return false;
    }
    if (!nameRegex().match(value).hasMatch()) {
        if (err) *err = "Допустима только кириллица и дефис, без пробелов";
        return false;
    }
    return true;
}

bool ValidationService::validateOptionalNamePart(const QString& value, QString* err) {
    if (value.trimmed().isEmpty()) return true;
    return validateNamePart(value, err);
}

bool ValidationService::validateAddress(const Address& address, QString* err) {
    if (address.region.trimmed().isEmpty() || !addressTextRegex().match(address.region).hasMatch()) {
        if (err) *err = "Регион: только кириллица, пробелы и дефис";
        return false;
    }
    if (address.city.trimmed().isEmpty() || !addressTextRegex().match(address.city).hasMatch()) {
        if (err) *err = "Город: только кириллица, пробелы и дефис";
        return false;
    }
    if (address.street.trimmed().isEmpty() || !addressTextRegex().match(address.street).hasMatch()) {
        if (err) *err = "Улица: только кириллица, пробелы и дефис";
        return false;
    }
    if (address.house.trimmed().isEmpty() || !houseRegex().match(address.house).hasMatch()) {
        if (err) *err = "Дом: допустимы цифры, литера и /";
        return false;
    }
    if (!address.building.trimmed().isEmpty() && !houseRegex().match(address.building).hasMatch()) {
        if (err) *err = "Корпус: допустимы цифры, литера и /";
        return false;
    }
    if (!address.apartment.trimmed().isEmpty() && !houseRegex().match(address.apartment).hasMatch()) {
        if (err) *err = "Квартира: допустимы цифры, литера и /";
        return false;
    }
    if (address.postalCode.trimmed().isEmpty() || !postalCodeRegex().match(address.postalCode).hasMatch()) {
        if (err) *err = "Индекс: 6 цифр";
        return false;
    }
    return true;
}
