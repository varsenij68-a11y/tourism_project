#pragma once

#include <QStringList>
#include <memory>
#include <vector>

#include "agency_types.h"
#include "animal.h"
#include "client.h"
#include "document.h"
#include "tour.h"
#include "tourist.h"

//=============================================================================
// Класс TourRequest (Sale) — заявка на тур
//=============================================================================

class TourRequest {
public:
    TourRequest(Client* client, Tour* tour, int id = 0);
    Client* getClient() const { return client_; }
    Tour* getTour() const { return tour_; }
    RequestStatus getStatus() const { return status_; }
    void setStatus(RequestStatus s) { status_ = s; }
    QString getTravelMode() const { return travelMode_; }
    void setTravelMode(const QString& mode);
    QString getTravelClass() const { return travelClass_; }
    void setTravelClass(const QString& travelClass);
    static QStringList travelClassOptionsForMode(const QString& mode);
    int getId() const { return id_; }

    // Туристы
    const std::vector<std::unique_ptr<Tourist>>& getTourists() const { return tourists_; }
    std::vector<std::unique_ptr<Tourist>>& tourists() { return tourists_; }
    void addAdult(const QString& lastName, const QString& firstName, const QString& middleName);
    void addChild(const QString& lastName, const QString& firstName, const QString& middleName,
                  const QDate& dateOfBirth);
    void removeTourist(int index);

    // Животные
    const std::vector<std::unique_ptr<Animal>>& getAnimals() const { return animals_; }
    void addAnimal(const QString& type, double weight, const QString& transport);
    void removeAnimal(int index);

    // Документы заявки (поездки)
    const std::vector<std::unique_ptr<Document>>& getDocuments() const { return documents_; }
    std::vector<std::unique_ptr<Document>>& documents() { return documents_; }
    void regenerateDocuments();
    Document* getDocument(int index);
    void setDocumentStatus(int index, DocumentStatus s);

    // Автоматический расчёт стоимости: взрослые + дети (скидка) + животные (доплата)
    double calculateTotalCost() const;

    // Проверка наличия обязательных документов
    bool checkDocumentsComplete() const;
    /** Список предупреждений о недостающих документах */
    QStringList getDocumentWarnings() const;

    // Предупреждения о возможных ошибках ввода
    QStringList getValidationWarnings() const;

private:
    int id_;
    Client* client_;
    Tour* tour_;
    RequestStatus status_;
    QString travelMode_;
    QString travelClass_;
    std::vector<std::unique_ptr<Tourist>> tourists_;
    std::vector<std::unique_ptr<Animal>> animals_;
    std::vector<std::unique_ptr<Document>> documents_;
    static int nextId;
    static constexpr double CHILD_DISCOUNT = 0.5;   // 50% скидка детям
    static constexpr double ANIMAL_BASE = 1000.0;   // базовая доплата за животное
    static constexpr double ANIMAL_PER_KG = 5.0;    // доплата за кг веса
};
