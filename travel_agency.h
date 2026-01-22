#pragma once

#include <vector>

#include "client.h"
#include "tour.h"
#include "tour_request.h"

//=============================================================================
// TravelAgency — логика приложения (отделение от интерфейса)
//=============================================================================

class TravelAgency {
public:
    TravelAgency();
    ~TravelAgency();

    // --- Клиенты ---
    std::vector<Client*>& clients() { return clients_; }
    const std::vector<Client*>& clients() const { return clients_; }
    Client* addClient(const QString& lastName, const QString& firstName, const QString& middleName,
                      const QString& phone, const QString& email, const QDate& dateOfBirth,
                      const Address& registrationAddress, const Address& actualAddress,
                      const QString& comments, QString* err = nullptr);
    bool editClient(int id, const QString& lastName, const QString& firstName, const QString& middleName,
                    const QString& phone, const QString& email, const QDate& dateOfBirth,
                    const Address& registrationAddress, const Address& actualAddress,
                    const QString& comments, QString* err = nullptr);
    bool deleteClient(int id, QString* err = nullptr);
    Client* findClientById(int id) const;
    /** Поиск по ФИО, телефону, email (подстрока, без учёта регистра) */
    std::vector<Client*> searchClients(const QString& query) const;
    /** История заявок (продаж) по клиенту */
    std::vector<TourRequest*> getSalesHistoryForClient(int clientId) const;

    // --- Туры ---
    std::vector<Tour*>& tours() { return tours_; }
    const std::vector<Tour*>& tours() const { return tours_; }
    Tour* addTour(const QString& name, const QString& country, const QString& tourType,
                  const QDate& startDate, int durationDays, double basePrice,
                  bool isDomestic, bool visaRequired, const QStringList& travelModes,
                  QString* err = nullptr);
    bool editTour(int id, const QString& name, const QString& country, const QString& tourType,
                  const QDate& startDate, int durationDays, double basePrice,
                  bool isDomestic, bool visaRequired, const QStringList& travelModes,
                  QString* err = nullptr);
    bool deleteTour(int id, QString* err = nullptr);
    Tour* findTourById(int id) const;

    // --- Заявки (продажи) ---
    std::vector<TourRequest*>& requests() { return requests_; }
    const std::vector<TourRequest*>& requests() const { return requests_; }
    TourRequest* createRequest(int clientId, int tourId, QString* err = nullptr);
    bool deleteRequest(int id, QString* err = nullptr);
    TourRequest* findRequestById(int id) const;

    // --- Сохранение / загрузка ---
    bool saveToFile(const QString& path, QString* err = nullptr) const;
    bool loadFromFile(const QString& path, QString* err = nullptr);

private:
    std::vector<Client*> clients_;
    std::vector<Tour*> tours_;
    std::vector<TourRequest*> requests_;
};
