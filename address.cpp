#include "address.h"

bool Address::isEmpty() const {
    return region.trimmed().isEmpty() && city.trimmed().isEmpty() && street.trimmed().isEmpty()
        && house.trimmed().isEmpty() && building.trimmed().isEmpty() && apartment.trimmed().isEmpty()
        && postalCode.trimmed().isEmpty() && additional.trimmed().isEmpty();
}

QJsonObject Address::toJson() const {
    QJsonObject obj;
    obj["region"] = region;
    obj["city"] = city;
    obj["street"] = street;
    obj["house"] = house;
    obj["building"] = building;
    obj["apartment"] = apartment;
    obj["postalCode"] = postalCode;
    obj["additional"] = additional;
    return obj;
}

Address Address::fromJson(const QJsonObject& obj) {
    Address a;
    a.region = obj["region"].toString();
    a.city = obj["city"].toString();
    a.street = obj["street"].toString();
    a.house = obj["house"].toString();
    a.building = obj["building"].toString();
    a.apartment = obj["apartment"].toString();
    a.postalCode = obj["postalCode"].toString();
    a.additional = obj["additional"].toString();
    return a;
}
