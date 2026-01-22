#include "document_service.h"

#include <algorithm>

#include "document.h"
#include "tour_request.h"
#include "tour.h"
#include "tourist.h"

std::vector<DocumentField> DocumentService::fieldsForType(DocumentType type) {
    switch (type) {
    case DocumentType::Passport:
        return {
            {"series", "Серия", QRegularExpression("^\\d{4}$"), "0000", true, "0000"},
            {"number", "Номер", QRegularExpression("^\\d{6}$"), "000000", true, "000000"},
            {"issueDate", "Дата выдачи", QRegularExpression(), "", false, ""},
            {"issuedBy", "Кем выдан", QRegularExpression(), "", false, ""},
            {"issuerCode", "Код подразделения", QRegularExpression("^\\d{3}-\\d{3}$"), "000-000", false, "000-000"}
        };
    case DocumentType::InternationalPassport:
        return {{"number", "Номер", QRegularExpression("^\\d{9}$"), "000000000", true, "000000000"}};
    case DocumentType::BirthCertificate:
        return {
            {"series", "Серия", QRegularExpression("^[IVX]{1,4}-?[А-ЯЁ]{2}$"), "", true, "IV-АР"},
            {"number", "Номер", QRegularExpression("^\\d{6}$"), "000000", true, "123456"}
        };
    case DocumentType::OMSPolicy:
        return {{"number", "Номер полиса", QRegularExpression("^\\d{16}$"), "0000 0000 0000 0000", true, "0000 0000 0000 0000"}};
    case DocumentType::SNILS:
        return {{"snils", "СНИЛС", QRegularExpression("^\\d{3}-\\d{3}-\\d{3}\\s\\d{2}$"), "000-000-000 00", true, "000-000-000 00"}};
    case DocumentType::INN:
        return {{"inn", "ИНН", QRegularExpression("^\\d{12}$"), "000000000000", false, "000000000000"}};
    case DocumentType::Visa:
        return {
            {"visaNumber", "Номер визы", QRegularExpression("^[A-Z0-9]{6,12}$"), "", true, "A1B2C3"},
            {"validFrom", "Действует с", QRegularExpression(), "", false, ""},
            {"validTo", "Действует до", QRegularExpression(), "", false, ""}
        };
    case DocumentType::InsurancePolicy:
        return {
            {"policyNumber", "Номер полиса", QRegularExpression("^[A-Z0-9-]{6,20}$"), "", true, "ABC-123"},
            {"company", "Компания", QRegularExpression(), "", false, ""},
            {"validFrom", "Действует с", QRegularExpression(), "", false, ""},
            {"validTo", "Действует до", QRegularExpression(), "", false, ""}
        };
    case DocumentType::BenefitDocument:
        return {
            {"docKind", "Тип льготы", QRegularExpression(), "", true, "Пенсионное/Инвалидность/Студенческий"},
            {"number", "Номер", QRegularExpression("^[A-ZА-Я0-9-]{1,20}$"), "", true, ""}
        };
    case DocumentType::Voucher:
        return {{"voucherNumber", "Номер ваучера", QRegularExpression("^[A-Z0-9-]{6,20}$"), "", true, ""}};
    case DocumentType::Tickets:
        return {
            {"ticketNumber", "Номер билета", QRegularExpression("^[A-Z0-9-]{6,20}$"), "", true, ""},
            {"transportType", "Тип транспорта", QRegularExpression(), "", true, "Самолёт/Поезд/Автобус"}
        };
    case DocumentType::ConsentForChildDeparture:
        return {
            {"docNumber", "Номер документа", QRegularExpression("^[A-ZА-Я0-9-]{1,20}$"), "", true, ""},
            {"docDate", "Дата документа", QRegularExpression(), "", false, ""}
        };
    case DocumentType::VeterinaryPassport:
        return {
            {"vetPassportNumber", "Номер ветпаспорта", QRegularExpression("^[A-ZА-Я0-9-]{1,20}$"), "", true, ""},
            {"vaccinationDate", "Дата вакцинации", QRegularExpression(), "", false, ""}
        };
    }
    return {};
}

static QString normalizedField(const QVariantMap& fields, const QString& key) {
    return fields.value(key).toString().trimmed();
}

bool DocumentService::isMinimumFilled(const Document& document) {
    const auto defs = fieldsForType(document.getType());
    for (const auto& def : defs) {
        if (!def.required) continue;
        const QString value = normalizedField(document.fields(), def.key);
        if (value.isEmpty()) return false;
        if (def.regex.isValid() && !def.regex.pattern().isEmpty() && !def.regex.match(value).hasMatch())
            return false;
    }
    return true;
}

bool DocumentService::validateDocument(const Document& document, QString* err) {
    const auto defs = fieldsForType(document.getType());
    for (const auto& def : defs) {
        const QString value = normalizedField(document.fields(), def.key);
        if (def.required && value.isEmpty()) {
            if (err) *err = QString("Поле '%1' обязательно").arg(def.label);
            return false;
        }
        if (!value.isEmpty() && def.regex.isValid() && !def.regex.pattern().isEmpty()
            && !def.regex.match(value).hasMatch()) {
            if (err) *err = QString("Поле '%1' имеет неверный формат").arg(def.label);
            return false;
        }
    }
    return true;
}

std::vector<DocumentType> DocumentService::requiredPersonalDocuments(const TourRequest& request, const Tourist& tourist) {
    std::vector<DocumentType> out;
    const bool domestic = request.getTour()->isDomestic();
    const bool visaRequired = request.getTour()->isVisaRequired();

    if (domestic) {
        out.push_back(tourist.isChild() ? DocumentType::BirthCertificate : DocumentType::Passport);
        out.push_back(DocumentType::OMSPolicy);
        if (tourist.hasBenefit())
            out.push_back(DocumentType::BenefitDocument);
    } else {
        out.push_back(DocumentType::InternationalPassport);
        if (visaRequired) out.push_back(DocumentType::Visa);
        if (tourist.isChild()) {
            out.push_back(DocumentType::BirthCertificate);
            out.push_back(DocumentType::ConsentForChildDeparture);
        }
        if (tourist.hasBenefit())
            out.push_back(DocumentType::BenefitDocument);
    }

    return out;
}

std::vector<DocumentType> DocumentService::requiredRequestDocuments(const TourRequest& request) {
    std::vector<DocumentType> out;

    if (!request.getTour()->isDomestic()) {
        out.push_back(DocumentType::Voucher);
        out.push_back(DocumentType::InsurancePolicy);
    } else {
        if (request.getTour()->getTourType().toLower().contains("актив"))
            out.push_back(DocumentType::InsurancePolicy);
    }

    if (!request.getTravelMode().trimmed().isEmpty())
        out.push_back(DocumentType::Tickets);

    if (!request.getAnimals().empty())
        out.push_back(DocumentType::VeterinaryPassport);

    return out;
}

QStringList DocumentService::missingDocumentsSummary(const TourRequest& request) {
    QStringList missing;

    for (const auto& t : request.getTourists()) {
        auto required = requiredPersonalDocuments(request, *t);
        for (const auto& docType : required) {
            bool found = false;
            for (const auto& doc : t->documents()) {
                if (doc->getType() == docType && doc->getStatus() == DocumentStatus::Verified) {
                    found = true;
                    break;
                }
            }
            if (!found) missing << QString("%1: %2").arg(t->getFullName(), Document::typeName(docType));
        }
    }

    auto requestRequired = requiredRequestDocuments(request);
    for (const auto& docType : requestRequired) {
        bool found = false;
        for (const auto& doc : request.getDocuments()) {
            if (doc->getType() == docType && doc->getStatus() == DocumentStatus::Verified) {
                found = true;
                break;
            }
        }
        if (!found) missing << QString("Заявка: %1").arg(Document::typeName(docType));
    }

    return missing;
}
