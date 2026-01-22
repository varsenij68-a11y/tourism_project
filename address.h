#pragma once

#include <QJsonObject>
#include <QString>

class Address {
public:
    QString region;
    QString city;
    QString street;
    QString house;
    QString building;
    QString apartment;
    QString postalCode;
    QString additional;

    bool isEmpty() const;
    QJsonObject toJson() const;
    static Address fromJson(const QJsonObject& obj);
};
