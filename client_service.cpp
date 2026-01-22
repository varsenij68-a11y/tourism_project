#include "client_service.h"

#include <exception>

#include "client.h"
#include "validation_service.h"

bool ClientService::validateClient(const QString& lastName, const QString& firstName, const QString& middleName,
                                   const QString& phone, const QString& email,
                                   const Address& registrationAddress, const Address& actualAddress,
                                   QString* err) {
    if (!ValidationService::validateNamePart(lastName, err)) return false;
    if (!ValidationService::validateNamePart(firstName, err)) return false;
    if (!ValidationService::validateOptionalNamePart(middleName, err)) return false;
    if (!ValidationService::validateAddress(registrationAddress, err)) return false;
    if (!ValidationService::validateAddress(actualAddress, err)) return false;
    if (phone.trimmed().isEmpty()) {
        if (err) *err = "Телефон обязателен";
        return false;
    }
    if (email.trimmed().isEmpty()) {
        if (err) *err = "Email обязателен";
        return false;
    }
    return true;
}

Client* ClientService::createClient(const QString& lastName, const QString& firstName, const QString& middleName,
                                    const QString& phone, const QString& email, const QDate& dateOfBirth,
                                    const Address& registrationAddress, const Address& actualAddress,
                                    const QString& comments, int id, QString* err) {
    if (!validateClient(lastName, firstName, middleName, phone, email,
                        registrationAddress, actualAddress, err)) {
        return nullptr;
    }
    try {
        return new Client(lastName, firstName, middleName, phone, email, dateOfBirth,
                          registrationAddress, actualAddress, comments, id);
    } catch (const std::exception& e) {
        if (err) *err = e.what();
        return nullptr;
    }
}
