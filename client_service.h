#pragma once

#include <QString>
#include <QDate>

#include "address.h"

class Client;

class ClientService {
public:
    static bool validateClient(const QString& lastName, const QString& firstName, const QString& middleName,
                               const QString& phone, const QString& email, const Address& registrationAddress,
                               const Address& actualAddress, QString* err = nullptr);
    static Client* createClient(const QString& lastName, const QString& firstName, const QString& middleName,
                                const QString& phone, const QString& email, const QDate& dateOfBirth,
                                const Address& registrationAddress, const Address& actualAddress,
                                const QString& comments, int id = 0, QString* err = nullptr);
};
