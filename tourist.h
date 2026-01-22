#pragma once

#include <QDate>
#include <QString>
#include <memory>
#include <vector>

#include "document.h"

//=============================================================================
// Базовый класс Tourist и наследники: AdultTourist, ChildTourist
//=============================================================================

class Tourist {
public:
    virtual ~Tourist() = default;
    virtual bool isChild() const = 0;
    virtual int getAge(const QDate& asOf = QDate::currentDate()) const = 0;
    virtual QString displayName() const = 0;
    virtual QString getFullName() const = 0;
    virtual QString getLastName() const = 0;
    virtual QString getFirstName() const = 0;
    virtual QString getMiddleName() const = 0;
    virtual bool hasBenefit() const = 0;
    virtual void setHasBenefit(bool value) = 0;
    std::vector<std::unique_ptr<Document>>& documents() { return documents_; }
    const std::vector<std::unique_ptr<Document>>& documents() const { return documents_; }
    void clearDocuments() { documents_.clear(); }
protected:
    std::vector<std::unique_ptr<Document>> documents_;
};

class AdultTourist : public Tourist {
public:
    AdultTourist(const QString& lastName, const QString& firstName, const QString& middleName);
    bool isChild() const override { return false; }
    int getAge(const QDate&) const override { return 0; }
    QString displayName() const override;
    QString getFullName() const override;
    QString getLastName() const override { return lastName_; }
    QString getFirstName() const override { return firstName_; }
    QString getMiddleName() const override { return middleName_; }
    bool hasBenefit() const override { return hasBenefit_; }
    void setHasBenefit(bool value) override { hasBenefit_ = value; }
    void setLastName(const QString& n) { lastName_ = n; }
    void setFirstName(const QString& n) { firstName_ = n; }
    void setMiddleName(const QString& n) { middleName_ = n; }
private:
    QString lastName_;
    QString firstName_;
    QString middleName_;
    bool hasBenefit_ = false;
};

class ChildTourist : public Tourist {
public:
    ChildTourist(const QString& lastName, const QString& firstName, const QString& middleName,
                 const QDate& dateOfBirth);
    bool isChild() const override { return true; }
    /** Автоматическое определение возраста по дате рождения на указанную дату */
    int getAge(const QDate& asOf = QDate::currentDate()) const override;
    QString displayName() const override;
    QString getFullName() const override;
    QString getLastName() const override { return lastName_; }
    QString getFirstName() const override { return firstName_; }
    QString getMiddleName() const override { return middleName_; }
    bool hasBenefit() const override { return hasBenefit_; }
    void setHasBenefit(bool value) override { hasBenefit_ = value; }
    QDate getDateOfBirth() const { return dateOfBirth_; }
    void setLastName(const QString& n) { lastName_ = n; }
    void setFirstName(const QString& n) { firstName_ = n; }
    void setMiddleName(const QString& n) { middleName_ = n; }
    void setDateOfBirth(const QDate& d);
private:
    QString lastName_;
    QString firstName_;
    QString middleName_;
    QDate dateOfBirth_;
    bool hasBenefit_ = false;
};
