#pragma once

#include <QString>
#include <QVariantMap>

#include "agency_types.h"

//=============================================================================
// Класс Document — документ с типом и статусом
//=============================================================================

class Document {
public:
    Document(DocumentType type, DocumentStatus status = DocumentStatus::Absent);
    DocumentType getType() const { return type_; }
    DocumentStatus getStatus() const { return status_; }
    void setStatus(DocumentStatus s) { status_ = s; }
    QVariantMap& fields() { return fields_; }
    const QVariantMap& fields() const { return fields_; }
    QString displayName() const;
    bool validate(QString* err = nullptr) const;
    /** Возвращает строковое название типа документа */
    static QString typeName(DocumentType t);
    /** Возвращает строковое название статуса */
    static QString statusName(DocumentStatus s);
private:
    DocumentType type_;
    DocumentStatus status_;
    QVariantMap fields_;
};
