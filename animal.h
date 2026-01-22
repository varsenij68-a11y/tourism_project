#pragma once

#include <QString>

class Animal {
public:
    Animal(const QString& type, double weight, const QString& transport);
    QString getType() const { return type_; }
    double getWeight() const { return weight_; }
    QString getTransport() const { return transport_; }
    void setType(const QString& t);
    void setWeight(double w);
    void setTransport(const QString& t);
    /** Минимальная проверка: тип и способ перевозки не пустые, вес > 0 */
    static bool validate(const QString& type, double weight, const QString& transport, QString* err = nullptr);
private:
    QString type_;
    double weight_;
    QString transport_;
};
