#pragma once

#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <vector>

#include "agency_types.h"

class Document;
class TourRequest;
class Tourist;

struct DocumentField {
    QString key;
    QString label;
    QRegularExpression regex;
    QString inputMask;
    bool required = true;
    QString placeholder;
};

class DocumentService {
public:
    static std::vector<DocumentField> fieldsForType(DocumentType type);
    static bool validateDocument(const Document& document, QString* err = nullptr);
    static bool isMinimumFilled(const Document& document);

    static std::vector<DocumentType> requiredPersonalDocuments(const TourRequest& request, const Tourist& tourist);
    static std::vector<DocumentType> requiredRequestDocuments(const TourRequest& request);

    static QStringList missingDocumentsSummary(const TourRequest& request);
};
