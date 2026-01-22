#include "request_service.h"

#include "document_service.h"
#include "tour_request.h"

bool RequestService::validateRequest(const TourRequest& request, QString* err) {
    if (request.getTourists().empty()) {
        if (err) *err = "Добавьте хотя бы одного туриста";
        return false;
    }
    const auto missing = DocumentService::missingDocumentsSummary(request);
    if (!missing.isEmpty()) {
        if (err) *err = "Есть незаполненные обязательные документы";
        return false;
    }
    return true;
}
