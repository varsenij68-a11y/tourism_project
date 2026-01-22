#include "travel_agency.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <algorithm>
#include <stdexcept>
#include <tuple>

#include "client_service.h"

TravelAgency::TravelAgency() = default;

TravelAgency::~TravelAgency() {
    for (auto* c : clients_) delete c;
    for (auto* t : tours_) delete t;
    for (auto* r : requests_) delete r;
}

Client* TravelAgency::addClient(const QString& lastName, const QString& firstName, const QString& middleName,
                                const QString& phone, const QString& email, const QDate& dateOfBirth,
                                const Address& registrationAddress, const Address& actualAddress,
                                const QString& comments, QString* err) {
    Client* c = ClientService::createClient(lastName, firstName, middleName, phone, email, dateOfBirth,
                                            registrationAddress, actualAddress, comments, 0, err);
    if (!c) return nullptr;
    clients_.push_back(c);
    return c;
}

bool TravelAgency::editClient(int id, const QString& lastName, const QString& firstName, const QString& middleName,
                              const QString& phone, const QString& email, const QDate& dateOfBirth,
                              const Address& registrationAddress, const Address& actualAddress,
                              const QString& comments, QString* err) {
    Client* c = findClientById(id);
    if (!c) { if (err) *err = "Клиент не найден"; return false; }
    if (!ClientService::validateClient(lastName, firstName, middleName, phone, email,
                                       registrationAddress, actualAddress, err)) {
        return false;
    }
    c->setLastName(lastName);
    c->setFirstName(firstName);
    c->setMiddleName(middleName);
    c->setPhone(phone);
    c->setEmail(email);
    c->setDateOfBirth(dateOfBirth);
    c->setComments(comments);
    c->setRegistrationAddress(registrationAddress);
    c->setActualAddress(actualAddress);
    return true;
}

bool TravelAgency::deleteClient(int id, QString* err) {
    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if ((*it)->getId() == id) {
            for (auto* r : requests_)
                if (r->getClient()->getId() == id) {
                    if (err) *err = "Невозможно удалить: есть заявки по этому клиенту";
                    return false;
                }
            delete *it;
            clients_.erase(it);
            return true;
        }
    }
    if (err) *err = "Клиент не найден";
    return false;
}

Client* TravelAgency::findClientById(int id) const {
    for (auto* c : clients_) if (c->getId() == id) return c;
    return nullptr;
}

std::vector<Client*> TravelAgency::searchClients(const QString& query) const {
    std::vector<Client*> out;
    QString q = query.trimmed().toLower();
    if (q.isEmpty()) return out;
    for (auto* c : clients_) {
        if (c->getFullName().toLower().contains(q) ||
            c->getPhone().contains(q) ||
            c->getEmail().toLower().contains(q))
            out.push_back(c);
    }
    return out;
}

std::vector<TourRequest*> TravelAgency::getSalesHistoryForClient(int clientId) const {
    std::vector<TourRequest*> out;
    for (auto* r : requests_)
        if (r->getClient()->getId() == clientId)
            out.push_back(r);
    return out;
}

// --- Tours ---
Tour* TravelAgency::addTour(const QString& name, const QString& country, const QString& tourType,
                            const QDate& startDate, int durationDays, double basePrice,
                            bool isDomestic, bool visaRequired, const QStringList& travelModes,
                            QString* err) {
    try {
        auto* t = new Tour(name, country, tourType, startDate, durationDays, basePrice, isDomestic, visaRequired, travelModes);
        tours_.push_back(t);
        return t;
    } catch (const std::exception& e) {
        if (err) *err = e.what();
        return nullptr;
    }
}

bool TravelAgency::editTour(int id, const QString& name, const QString& country, const QString& tourType,
                            const QDate& startDate, int durationDays, double basePrice,
                            bool isDomestic, bool visaRequired, const QStringList& travelModes,
                            QString* err) {
    Tour* t = findTourById(id);
    if (!t) { if (err) *err = "Тур не найден"; return false; }
    try {
        t->setName(name);
        t->setCountry(country);
        t->setTourType(tourType);
        t->setStartDate(startDate);
        t->setDurationDays(durationDays);
        t->setBasePrice(basePrice);
        t->setDomestic(isDomestic);
        t->setVisaRequired(visaRequired);
        t->setTravelModes(travelModes);
        return true;
    } catch (const std::exception& e) {
        if (err) *err = e.what();
        return false;
    }
}

bool TravelAgency::deleteTour(int id, QString* err) {
    for (auto it = tours_.begin(); it != tours_.end(); ++it) {
        if ((*it)->getId() == id) {
            for (auto* r : requests_)
                if (r->getTour()->getId() == id) {
                    if (err) *err = "Невозможно удалить: есть заявки на этот тур";
                    return false;
                }
            delete *it;
            tours_.erase(it);
            return true;
        }
    }
    if (err) *err = "Тур не найден";
    return false;
}

Tour* TravelAgency::findTourById(int id) const {
    for (auto* t : tours_) if (t->getId() == id) return t;
    return nullptr;
}

// --- Requests ---
TourRequest* TravelAgency::createRequest(int clientId, int tourId, QString* err) {
    Client* c = findClientById(clientId);
    Tour* t = findTourById(tourId);
    if (!c) { if (err) *err = "Клиент не найден"; return nullptr; }
    if (!t) { if (err) *err = "Тур не найден"; return nullptr; }
    try {
        auto* r = new TourRequest(c, t);
        requests_.push_back(r);
        return r;
    } catch (const std::exception& e) {
        if (err) *err = e.what();
        return nullptr;
    }
}

bool TravelAgency::deleteRequest(int id, QString* err) {
    for (auto it = requests_.begin(); it != requests_.end(); ++it) {
        if ((*it)->getId() == id) {
            delete *it;
            requests_.erase(it);
            return true;
        }
    }
    if (err) *err = "Заявка не найдена";
    return false;
}

TourRequest* TravelAgency::findRequestById(int id) const {
    for (auto* r : requests_) if (r->getId() == id) return r;
    return nullptr;
}

// --- Save / Load (JSON) ---
bool TravelAgency::saveToFile(const QString& path, QString* err) const {
    QJsonObject root;
    QJsonArray arrClients, arrTours, arrRequests;

    for (auto* c : clients_) {
        QJsonObject o;
        o["id"] = c->getId();
        o["lastName"] = c->getLastName();
        o["firstName"] = c->getFirstName();
        o["middleName"] = c->getMiddleName();
        o["fullName"] = c->getFullName();
        o["phone"] = c->getPhone();
        o["email"] = c->getEmail();
        o["dateOfBirth"] = c->getDateOfBirth().toString(Qt::ISODate);
        o["comments"] = c->getComments();
        o["registrationAddress"] = c->getRegistrationAddress().toJson();
        o["actualAddress"] = c->getActualAddress().toJson();
        arrClients.append(o);
    }

    for (auto* t : tours_) {
        QJsonObject o;
        o["id"] = t->getId();
        o["name"] = t->getName();
        o["country"] = t->getCountry();
        o["tourType"] = t->getTourType();
        o["startDate"] = t->getStartDate().toString(Qt::ISODate);
        o["durationDays"] = t->getDurationDays();
        o["basePrice"] = t->getBasePrice();
        o["isDomestic"] = t->isDomestic();
        o["visaRequired"] = t->isVisaRequired();
        QJsonArray travelModes;
        for (const QString& mode : t->getTravelModes())
            travelModes.append(mode);
        o["travelModes"] = travelModes;
        arrTours.append(o);
    }

    for (auto* r : requests_) {
        QJsonObject o;
        o["id"] = r->getId();
        o["clientId"] = r->getClient()->getId();
        o["tourId"] = r->getTour()->getId();
        o["status"] = static_cast<int>(r->getStatus());
        o["travelMode"] = r->getTravelMode();
        o["travelClass"] = r->getTravelClass();

        QJsonArray tourists, animals, documents;

        for (const auto& t : r->getTourists()) {
            QJsonObject to;
            to["isChild"] = t->isChild();
            to["lastName"] = t->getLastName();
            to["firstName"] = t->getFirstName();
            to["middleName"] = t->getMiddleName();
            to["fullName"] = t->getFullName();
            to["hasBenefit"] = t->hasBenefit();
            if (t->isChild()) {
                to["dateOfBirth"] = static_cast<ChildTourist*>(t.get())->getDateOfBirth().toString(Qt::ISODate);
            }
            QJsonArray touristDocs;
            for (const auto& doc : t->documents()) {
                QJsonObject docObj;
                docObj["type"] = static_cast<int>(doc->getType());
                docObj["status"] = static_cast<int>(doc->getStatus());
                docObj["fields"] = QJsonObject::fromVariantMap(doc->fields());
                touristDocs.append(docObj);
            }
            to["documents"] = touristDocs;
            tourists.append(to);
        }

        for (const auto& a : r->getAnimals()) {
            QJsonObject ao;
            ao["type"] = a->getType();
            ao["weight"] = a->getWeight();
            ao["transport"] = a->getTransport();
            animals.append(ao);
        }

        for (const auto& d : r->getDocuments()) {
            QJsonObject docObj;
            docObj["type"] = static_cast<int>(d->getType());
            docObj["status"] = static_cast<int>(d->getStatus());
            docObj["fields"] = QJsonObject::fromVariantMap(d->fields());
            documents.append(docObj);
        }

        o["tourists"] = tourists;
        o["animals"] = animals;
        o["documents"] = documents;
        arrRequests.append(o);
    }

    root["clients"] = arrClients;
    root["tours"] = arrTours;
    root["requests"] = arrRequests;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (err) *err = "Не удалось открыть файл для записи";
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool TravelAgency::loadFromFile(const QString& path, QString* err) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (err) *err = "Не удалось открыть файл";
        return false;
    }

    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &perr);
    if (doc.isNull()) {
        if (err) *err = "Ошибка JSON: " + perr.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    // Очищаем и загружаем заявки в последнюю очередь (зависят от клиентов и туров)
    for (auto* r : requests_) delete r;
    requests_.clear();
    for (auto* c : clients_) delete c;
    clients_.clear();
    for (auto* t : tours_) delete t;
    tours_.clear();

    // Клиенты
    auto parseLegacyName = [](const QString& fullName) {
        QStringList parts = fullName.split(' ', Qt::SkipEmptyParts);
        QString last = parts.value(0);
        QString first = parts.value(1);
        QString middle = parts.mid(2).join(' ');
        return std::tuple<QString, QString, QString>(last, first, middle);
    };

    for (const QJsonValue& v : root["clients"].toArray()) {
        QJsonObject o = v.toObject();
        int id = o["id"].toInt();
        QString last = o["lastName"].toString();
        QString first = o["firstName"].toString();
        QString middle = o["middleName"].toString();
        if (last.isEmpty() && first.isEmpty() && o.contains("fullName")) {
            auto legacy = parseLegacyName(o["fullName"].toString());
            last = std::get<0>(legacy);
            first = std::get<1>(legacy);
            middle = std::get<2>(legacy);
        }
        Address reg = Address::fromJson(o["registrationAddress"].toObject());
        Address actual = Address::fromJson(o["actualAddress"].toObject());
        if (actual.isEmpty() && !reg.isEmpty()) actual = reg;
        clients_.push_back(new Client(
            last,
            first,
            middle,
            o["phone"].toString(),
            o["email"].toString(),
            QDate::fromString(o["dateOfBirth"].toString(), Qt::ISODate),
            reg,
            actual,
            o["comments"].toString(),
            id
            ));
    }

    // Туры
    for (const QJsonValue& v : root["tours"].toArray()) {
        QJsonObject o = v.toObject();
        int id = o["id"].toInt();
        tours_.push_back(new Tour(
            o["name"].toString(),
            o["country"].toString(),
            o["tourType"].toString(),
            QDate::fromString(o["startDate"].toString(), Qt::ISODate),
            o["durationDays"].toInt(),
            o["basePrice"].toDouble(),
            o["isDomestic"].toBool(),
            o["visaRequired"].toBool(),
            [&o]() {
                QStringList modes;
                for (const QJsonValue& mv : o["travelModes"].toArray())
                    modes.append(mv.toString());
                return modes;
            }(),
            id
            ));
    }

    // Заявки
    for (const QJsonValue& v : root["requests"].toArray()) {
        QJsonObject o = v.toObject();
        int id = o["id"].toInt();

        Client* c = findClientById(o["clientId"].toInt());
        Tour* t = findTourById(o["tourId"].toInt());
        if (!c || !t) continue;

        TourRequest* r = new TourRequest(c, t, id);
        r->setStatus(static_cast<RequestStatus>(o["status"].toInt()));
        if (o.contains("travelMode"))
            r->setTravelMode(o["travelMode"].toString());
        if (o.contains("travelClass"))
            r->setTravelClass(o["travelClass"].toString());

        for (const QJsonValue& tv : o["tourists"].toArray()) {
            QJsonObject to = tv.toObject();
            QString last = to["lastName"].toString();
            QString first = to["firstName"].toString();
            QString middle = to["middleName"].toString();
            if (last.isEmpty() && first.isEmpty() && to.contains("fullName")) {
                auto legacy = parseLegacyName(to["fullName"].toString());
                last = std::get<0>(legacy);
                first = std::get<1>(legacy);
                middle = std::get<2>(legacy);
            }
            if (to["isChild"].toBool()) {
                r->addChild(last, first, middle,
                            QDate::fromString(to["dateOfBirth"].toString(), Qt::ISODate));
            } else {
                r->addAdult(last, first, middle);
            }
            auto& tourist = r->tourists().back();
            tourist->setHasBenefit(to["hasBenefit"].toBool());
            tourist->clearDocuments();
            for (const QJsonValue& dv : to["documents"].toArray()) {
                QJsonObject dobj = dv.toObject();
                auto doc = std::make_unique<Document>(static_cast<DocumentType>(dobj["type"].toInt()));
                doc->setStatus(static_cast<DocumentStatus>(dobj["status"].toInt()));
                doc->fields() = dobj["fields"].toObject().toVariantMap();
                tourist->documents().push_back(std::move(doc));
            }
        }

        for (const QJsonValue& av : o["animals"].toArray()) {
            QJsonObject ao = av.toObject();
            r->addAnimal(ao["type"].toString(), ao["weight"].toDouble(), ao["transport"].toString());
        }

        // Документы заявки
        r->regenerateDocuments();
        QJsonArray docsArr = o["documents"].toArray();
        const int n = std::min<int>(static_cast<int>(docsArr.size()),
                                    static_cast<int>(r->getDocuments().size()));
        for (int i = 0; i < n; ++i) {
            QJsonObject dobj = docsArr[i].toObject();
            r->documents()[i]->setStatus(static_cast<DocumentStatus>(dobj["status"].toInt()));
            r->documents()[i]->fields() = dobj["fields"].toObject().toVariantMap();
        }

        requests_.push_back(r);
    }

    return true;
}
