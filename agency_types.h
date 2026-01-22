#pragma once

/** Типы документов для поездки */
enum class DocumentType {
    Passport,
    InternationalPassport,
    BirthCertificate,
    OMSPolicy,
    SNILS,
    INN,
    Visa,
    InsurancePolicy,
    BenefitDocument,
    Voucher,
    Tickets,
    ConsentForChildDeparture,
    VeterinaryPassport
};

/** Статус документа: отсутствует / имеется / проверен */
enum class DocumentStatus {
    Absent,      // Отсутствует
    Available,   // Имеется
    Verified     // Проверен
};

/** Статус заявки на тур */
enum class RequestStatus {
    Draft,       // Черновик
    Completed,   // Оформлена
    Paid,        // Оплачена
    Canceled     // Отменена
};
