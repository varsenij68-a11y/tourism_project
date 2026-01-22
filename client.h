#pragma once

#include <QDate>
#include <QString>

#include "address.h"

//=============================================================================
// Класс Client — клиент агентства
//=============================================================================

class Client {
public:
    Client(const QString& lastName, const QString& firstName, const QString& middleName,
           const QString& phone, const QString& email, const QDate& dateOfBirth,
           const Address& registrationAddress, const Address& actualAddress,
           const QString& comments = QString(), int id = 0);
    QString getLastName() const { return lastName_; }
    QString getFirstName() const { return firstName_; }
    QString getMiddleName() const { return middleName_; }
    QString getFullName() const;
    QString getPhone() const { return phone_; }
    QString getEmail() const { return email_; }
    QDate getDateOfBirth() const { return dateOfBirth_; }
    QString getComments() const { return comments_; }
    Address getRegistrationAddress() const { return registrationAddress_; }
    Address getActualAddress() const { return actualAddress_; }
    int getId() const { return id_; }
    void setLastName(const QString& n) { lastName_ = n; }
    void setFirstName(const QString& n) { firstName_ = n; }
    void setMiddleName(const QString& n) { middleName_ = n; }
    void setPhone(const QString& p) { phone_ = p; }
    void setEmail(const QString& e) { email_ = e; }
    void setDateOfBirth(const QDate& d) { dateOfBirth_ = d; }
    void setComments(const QString& c) { comments_ = c; }
    void setRegistrationAddress(const Address& address) { registrationAddress_ = address; }
    void setActualAddress(const Address& address) { actualAddress_ = address; }
private:
    int id_;
    QString lastName_;
    QString firstName_;
    QString middleName_;
    QString phone_;
    QString email_;
    QDate dateOfBirth_;
    QString comments_;
    Address registrationAddress_;
    Address actualAddress_;
    static int nextId;
};
