#pragma once

#include <QDate>
#include <QString>
#include <QStringList>

//=============================================================================
// Класс Tour — тур
//=============================================================================

class Tour {
public:
    Tour(const QString& name, const QString& country, const QString& tourType,
         const QDate& startDate, int durationDays, double basePrice,
         bool isDomestic, bool visaRequired, const QStringList& travelModes = {},
         int id = 0);
    QString getName() const { return name_; }
    QString getCountry() const { return country_; }
    QString getTourType() const { return tourType_; }
    QDate getStartDate() const { return startDate_; }
    int getDurationDays() const { return durationDays_; }
    double getBasePrice() const { return basePrice_; }
    bool isDomestic() const { return isDomestic_; }
    bool isVisaRequired() const { return visaRequired_; }
    QStringList getTravelModes() const { return travelModes_; }
    int getId() const { return id_; }
    void setName(const QString& n) { name_ = n; }
    void setCountry(const QString& c) { country_ = c; }
    void setTourType(const QString& t) { tourType_ = t; }
    void setStartDate(const QDate& d) { startDate_ = d; }
    void setDurationDays(int d) { durationDays_ = d; }
    void setBasePrice(double p) { basePrice_ = p; }
    void setDomestic(bool d) { isDomestic_ = d; }
    void setVisaRequired(bool v) { visaRequired_ = v; }
    void setTravelModes(const QStringList& modes) {
        travelModes_ = modes.isEmpty() ? QStringList({"Самолёт", "Поезд"}) : modes;
    }
    QDate getEndDate() const;
private:
    int id_;
    QString name_;
    QString country_;
    QString tourType_;
    QDate startDate_;
    int durationDays_;
    double basePrice_;
    bool isDomestic_;
    bool visaRequired_;
    QStringList travelModes_;
    static int nextId;
};
