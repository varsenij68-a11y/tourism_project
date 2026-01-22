#include "tourist.h"

#include <algorithm>
#include <stdexcept>

AdultTourist::AdultTourist(const QString& lastName, const QString& firstName, const QString& middleName)
    : lastName_(lastName), firstName_(firstName), middleName_(middleName) {
    if (lastName.trimmed().isEmpty() || firstName.trimmed().isEmpty())
        throw std::invalid_argument("Фамилия и имя туриста не могут быть пустыми");
}

ChildTourist::ChildTourist(const QString& lastName, const QString& firstName, const QString& middleName,
                           const QDate& dateOfBirth)
    : lastName_(lastName), firstName_(firstName), middleName_(middleName), dateOfBirth_(dateOfBirth) {
    if (lastName.trimmed().isEmpty() || firstName.trimmed().isEmpty())
        throw std::invalid_argument("Фамилия и имя ребёнка не могут быть пустыми");
    if (!dateOfBirth.isValid() || dateOfBirth > QDate::currentDate())
        throw std::invalid_argument("Некорректная дата рождения");
    if (getAge() > 18)
        throw std::invalid_argument("Возраст ребёнка не может быть больше 18 лет");
}

int ChildTourist::getAge(const QDate& asOf) const {
    if (!dateOfBirth_.isValid() || !asOf.isValid()) return 0;
    int a = asOf.year() - dateOfBirth_.year();
    if (asOf.month() < dateOfBirth_.month() ||
        (asOf.month() == dateOfBirth_.month() && asOf.day() < dateOfBirth_.day()))
        --a;
    return std::max(0, a);
}

QString ChildTourist::displayName() const {
    return getFullName() + " (ребёнок, " + QString::number(getAge()) + " лет)";
}

QString AdultTourist::displayName() const {
    return getFullName() + " (взрослый)";
}

QString AdultTourist::getFullName() const {
    const QString mid = middleName_.trimmed();
    return mid.isEmpty()
        ? QString("%1 %2").arg(lastName_, firstName_)
        : QString("%1 %2 %3").arg(lastName_, firstName_, mid);
}

QString ChildTourist::getFullName() const {
    const QString mid = middleName_.trimmed();
    return mid.isEmpty()
        ? QString("%1 %2").arg(lastName_, firstName_)
        : QString("%1 %2 %3").arg(lastName_, firstName_, mid);
}

void ChildTourist::setDateOfBirth(const QDate& d) {
    if (!d.isValid() || d > QDate::currentDate())
        throw std::invalid_argument("Некорректная дата рождения");
    int age = QDate::currentDate().year() - d.year();
    if (QDate::currentDate().month() < d.month() ||
        (QDate::currentDate().month() == d.month() && QDate::currentDate().day() < d.day()))
        --age;
    if (age > 18)
        throw std::invalid_argument("Возраст ребёнка не может быть больше 18 лет");
    dateOfBirth_ = d;
}
