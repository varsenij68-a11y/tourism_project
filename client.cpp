#include "client.h"

#include <stdexcept>

int Client::nextId = 1;

Client::Client(const QString& lastName, const QString& firstName, const QString& middleName,
               const QString& phone, const QString& email, const QDate& dateOfBirth,
               const Address& registrationAddress, const Address& actualAddress,
               const QString& comments, int id)
    : lastName_(lastName),
      firstName_(firstName),
      middleName_(middleName),
      phone_(phone),
      email_(email),
      dateOfBirth_(dateOfBirth),
      comments_(comments),
      registrationAddress_(registrationAddress),
      actualAddress_(actualAddress) {
    if (id > 0) { id_ = id; if (id >= nextId) nextId = id + 1; }
    else { id_ = nextId++; }
    if (lastName.trimmed().isEmpty() || firstName.trimmed().isEmpty())
        throw std::invalid_argument("Фамилия и имя клиента не могут быть пустыми");
}

QString Client::getFullName() const {
    const QString mid = middleName_.trimmed();
    return mid.isEmpty()
        ? QString("%1 %2").arg(lastName_, firstName_)
        : QString("%1 %2 %3").arg(lastName_, firstName_, mid);
}
