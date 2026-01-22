#include "animal.h"

#include <stdexcept>

Animal::Animal(const QString& type, double weight, const QString& transport)
    : type_(type), weight_(weight), transport_(transport) {
    if (!validate(type_, weight_, transport_))
        throw std::invalid_argument("Некорректные данные животного");
}

void Animal::setType(const QString& t) {
    if (t.trimmed().isEmpty()) throw std::invalid_argument("Тип животного не может быть пустым");
    type_ = t;
}

void Animal::setWeight(double w) {
    if (w <= 0) throw std::invalid_argument("Вес должен быть положительным");
    weight_ = w;
}

void Animal::setTransport(const QString& t) {
    if (t.trimmed().isEmpty()) throw std::invalid_argument("Способ перевозки не может быть пустым");
    transport_ = t;
}

bool Animal::validate(const QString& type, double weight, const QString& transport, QString* err) {
    if (type.trimmed().isEmpty()) { if (err) *err = "Тип животного не указан"; return false; }
    if (weight <= 0) { if (err) *err = "Вес должен быть больше 0"; return false; }
    if (transport.trimmed().isEmpty()) { if (err) *err = "Способ перевозки не указан"; return false; }
    return true;
}
