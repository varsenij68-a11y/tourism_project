#include "tour_request.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include "document_service.h"

int TourRequest::nextId = 1;

TourRequest::TourRequest(Client* client, Tour* tour, int id)
    : client_(client), tour_(tour), status_(RequestStatus::Draft) {
    if (id > 0) { id_ = id; if (id >= nextId) nextId = id + 1; }
    else { id_ = nextId++; }
    if (!client_ || !tour_) throw std::invalid_argument("Клиент и тур обязательны");
    const QStringList modes = tour_->getTravelModes();
    travelMode_ = modes.isEmpty() ? "Самолёт" : modes.first();
    const QStringList classes = travelClassOptionsForMode(travelMode_);
    travelClass_ = classes.isEmpty() ? QString() : classes.first();
}

void TourRequest::addAdult(const QString& lastName, const QString& firstName, const QString& middleName) {
    tourists_.push_back(std::make_unique<AdultTourist>(lastName, firstName, middleName));
    regenerateDocuments();
}

void TourRequest::addChild(const QString& lastName, const QString& firstName, const QString& middleName,
                           const QDate& dateOfBirth) {
    tourists_.push_back(std::make_unique<ChildTourist>(lastName, firstName, middleName, dateOfBirth));
    regenerateDocuments();
}

void TourRequest::removeTourist(int index) {
    if (index >= 0 && index < static_cast<int>(tourists_.size()))
        tourists_.erase(tourists_.begin() + index);
    regenerateDocuments();
}

void TourRequest::addAnimal(const QString& type, double weight, const QString& transport) {
    QString err;
    if (!Animal::validate(type, weight, transport, &err))
        throw std::invalid_argument(err.toStdString());
    animals_.push_back(std::make_unique<Animal>(type, weight, transport));
    regenerateDocuments();
}

void TourRequest::removeAnimal(int index) {
    if (index >= 0 && index < static_cast<int>(animals_.size()))
        animals_.erase(animals_.begin() + index);
    regenerateDocuments();
}

void TourRequest::setTravelMode(const QString& mode) {
    if (mode.trimmed().isEmpty()) return;
    const QStringList modes = tour_->getTravelModes();
    if (!modes.isEmpty() && !modes.contains(mode)) {
        travelMode_ = modes.first();
    } else {
        travelMode_ = mode;
    }
    const QStringList classes = travelClassOptionsForMode(travelMode_);
    if (!classes.isEmpty() && !classes.contains(travelClass_)) {
        travelClass_ = classes.first();
    }
}

void TourRequest::setTravelClass(const QString& travelClass) {
    if (travelClass.trimmed().isEmpty()) return;
    const QStringList classes = travelClassOptionsForMode(travelMode_);
    if (!classes.isEmpty() && !classes.contains(travelClass)) {
        travelClass_ = classes.first();
        return;
    }
    travelClass_ = travelClass;
}

QStringList TourRequest::travelClassOptionsForMode(const QString& mode) {
    if (mode == "Поезд") {
        return {"Купе", "Плацкарт"};
    }
    if (mode == "Самолёт") {
        return {"Эконом", "Бизнес", "Первый класс"};
    }
    return {};
}

void TourRequest::regenerateDocuments() {
    struct Snapshot {
        DocumentStatus status;
        QVariantMap fields;
    };

    auto snapshotDocs = [](const std::vector<std::unique_ptr<Document>>& docs) {
        std::unordered_map<int, Snapshot> map;
        for (const auto& doc : docs) {
            map[(int)doc->getType()] = Snapshot{doc->getStatus(), doc->fields()};
        }
        return map;
    };

    const auto requestSnapshot = snapshotDocs(documents_);

    for (auto& t : tourists_) {
        const auto touristSnapshot = snapshotDocs(t->documents());
        t->clearDocuments();
        const auto required = DocumentService::requiredPersonalDocuments(*this, *t);
        for (const auto& docType : required) {
            auto doc = std::make_unique<Document>(docType);
            auto it = touristSnapshot.find((int)docType);
            if (it != touristSnapshot.end()) {
                doc->setStatus(it->second.status);
                doc->fields() = it->second.fields;
            }
            t->documents().push_back(std::move(doc));
        }
        for (const auto& item : touristSnapshot) {
            const DocumentType docType = static_cast<DocumentType>(item.first);
            if (std::find(required.begin(), required.end(), docType) != required.end()) continue;
            auto doc = std::make_unique<Document>(docType);
            doc->setStatus(item.second.status);
            doc->fields() = item.second.fields;
            t->documents().push_back(std::move(doc));
        }
    }

    documents_.clear();
    const auto requestDocs = DocumentService::requiredRequestDocuments(*this);
    for (const auto& docType : requestDocs) {
        auto doc = std::make_unique<Document>(docType);
        auto it = requestSnapshot.find((int)docType);
        if (it != requestSnapshot.end()) {
            doc->setStatus(it->second.status);
            doc->fields() = it->second.fields;
        }
        documents_.push_back(std::move(doc));
    }
    for (const auto& item : requestSnapshot) {
        const DocumentType docType = static_cast<DocumentType>(item.first);
        if (std::find(requestDocs.begin(), requestDocs.end(), docType) != requestDocs.end()) continue;
        auto doc = std::make_unique<Document>(docType);
        doc->setStatus(item.second.status);
        doc->fields() = item.second.fields;
        documents_.push_back(std::move(doc));
    }
}

Document* TourRequest::getDocument(int index) {
    if (index >= 0 && index < static_cast<int>(documents_.size()))
        return documents_[index].get();
    return nullptr;
}

void TourRequest::setDocumentStatus(int index, DocumentStatus s) {
    Document* d = getDocument(index);
    if (d) d->setStatus(s);
}

double TourRequest::calculateTotalCost() const {
    int adults = 0, children = 0;
    for (const auto& t : tourists_) {
        if (t->isChild()) ++children; else ++adults;
    }
    double bp = tour_->getBasePrice();
    double cost = adults * bp + children * bp * CHILD_DISCOUNT;
    for (const auto& a : animals_)
        cost += ANIMAL_BASE + a->getWeight() * ANIMAL_PER_KG;
    return cost;
}

bool TourRequest::checkDocumentsComplete() const {
    return DocumentService::missingDocumentsSummary(*this).isEmpty();
}

QStringList TourRequest::getDocumentWarnings() const {
    QStringList w = DocumentService::missingDocumentsSummary(*this);
    if (tourists_.empty()) w << "В заявке нет ни одного туриста.";
    return w;
}

QStringList TourRequest::getValidationWarnings() const {
    QStringList w;
    for (const auto& t : tourists_) {
        if (t->isChild()) {
            auto* c = dynamic_cast<ChildTourist*>(t.get());
            if (c && !c->getDateOfBirth().isValid())
                w << "У ребёнка " + c->getFullName() + " не указана дата рождения.";
        }
    }
    return w;
}
