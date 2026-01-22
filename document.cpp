#include "document.h"

#include "document_service.h"

Document::Document(DocumentType type, DocumentStatus status) : type_(type), status_(status) {}

QString Document::typeName(DocumentType t) {
    switch (t) {
    case DocumentType::Passport: return "Внутренний паспорт";
    case DocumentType::InternationalPassport: return "Загранпаспорт";
    case DocumentType::BirthCertificate: return "Свидетельство о рождении";
    case DocumentType::OMSPolicy: return "Полис ОМС";
    case DocumentType::SNILS: return "СНИЛС";
    case DocumentType::INN: return "ИНН";
    case DocumentType::Visa: return "Виза";
    case DocumentType::InsurancePolicy: return "Страховой полис";
    case DocumentType::BenefitDocument: return "Документ о льготах";
    case DocumentType::Voucher: return "Туристический ваучер";
    case DocumentType::Tickets: return "Билеты";
    case DocumentType::ConsentForChildDeparture: return "Согласие на выезд ребёнка";
    case DocumentType::VeterinaryPassport: return "Ветеринарный паспорт";
    }
    return "?";
}

QString Document::statusName(DocumentStatus s) {
    switch (s) {
    case DocumentStatus::Absent: return "Отсутствует";
    case DocumentStatus::Available: return "Имеется";
    case DocumentStatus::Verified: return "Проверен";
    }
    return "?";
}

QString Document::displayName() const {
    return typeName(type_);
}

bool Document::validate(QString* err) const {
    return DocumentService::validateDocument(*this, err);
}
