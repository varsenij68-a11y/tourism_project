#pragma once

#include <QString>

class TourRequest;

class RequestService {
public:
    static bool validateRequest(const TourRequest& request, QString* err = nullptr);
};
