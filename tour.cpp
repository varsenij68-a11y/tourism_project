#include "tour.h"

#include <stdexcept>

int Tour::nextId = 1;

Tour::Tour(const QString& name, const QString& country, const QString& tourType,
           const QDate& startDate, int durationDays, double basePrice,
           bool isDomestic, bool visaRequired, const QStringList& travelModes,
           int id)
    : name_(name), country_(country), tourType_(tourType), startDate_(startDate),
    durationDays_(durationDays), basePrice_(basePrice), isDomestic_(isDomestic),
    visaRequired_(visaRequired) {
    if (id > 0) { id_ = id; if (id >= nextId) nextId = id + 1; }
    else { id_ = nextId++; }
    if (name.trimmed().isEmpty()) throw std::invalid_argument("Название тура не может быть пустым");
    if (durationDays <= 0) throw std::invalid_argument("Длительность должна быть больше 0");
    if (basePrice < 0) throw std::invalid_argument("Базовая цена не может быть отрицательной");
    if (travelModes.isEmpty())
        travelModes_ = {"Самолёт", "Поезд"};
    else
        travelModes_ = travelModes;
}

QDate Tour::getEndDate() const {
    return startDate_.addDays(durationDays_);
}
