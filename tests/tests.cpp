/**
 * @file tests.cpp
 * @brief Тестовые случаи для приложения «Туристическое агентство».
 * Проверка: валидация, расчёт стоимости, документы, CRUD.
 */
#include "agency.h"
#include <QCoreApplication>
#include <QDate>
#include <cstdio>
#include <cassert>

#define RUN_TEST(name) do { \
    fprintf(stderr, "  [TEST] %s ... ", #name); \
    name(); \
    fprintf(stderr, "OK\n"); \
} while(0)

static Address makeAddress() {
    Address reg;
    reg.region = "Московская";
    reg.city = "Москва";
    reg.street = "Тверская";
    reg.house = "1";
    reg.postalCode = "123456";
    return reg;
}

// --- 1. Client: создание, геттеры ---
void test_client_create() {
    Address reg = makeAddress();
    Address act = reg;
    Client c("Иванов", "Иван", "Иванович", "+7 999 123-45-67", "ivan@mail.ru",
             QDate(1990, 5, 15), reg, act, "VIP");
    assert(c.getId() >= 1);
    assert(c.getFullName() == "Иванов Иван Иванович");
    assert(c.getPhone() == "+7 999 123-45-67");
    assert(c.getEmail() == "ivan@mail.ru");
    assert(c.getDateOfBirth() == QDate(1990, 5, 15));
    assert(c.getComments() == "VIP");
}

// --- 2. Tour: создание, внутренний/виза ---
void test_tour_create() {
    Tour t("Отдых в Сочи", "Россия", "Пляжный",
           QDate(2025, 7, 1), 7, 25000.0, true, false);
    assert(t.getId() >= 1);
    assert(t.getName() == "Отдых в Сочи");
    assert(t.isDomestic() == true);
    assert(t.isVisaRequired() == false);
    assert(t.getBasePrice() == 25000.0);
}

// --- 3. Animal: валидация (минимальная проверка) ---
void test_animal_validate() {
    QString err;
    assert(Animal::validate("Кот", 4.5, "В салоне", &err) == true);
    assert(Animal::validate("", 4.5, "В салоне", &err) == false);
    assert(Animal::validate("Собака", 0, "Багаж", &err) == false);
    assert(Animal::validate("Собака", 5, "", &err) == false);
}

// --- 4. ChildTourist: автоматический расчёт возраста ---
void test_child_age() {
    ChildTourist ch("Петя", "И", "", QDate(2020, 3, 10));
    assert(ch.isChild() == true);
    int age = ch.getAge(QDate(2025, 5, 1)); // 5 полных лет
    assert(age == 5);
}

// --- 5. TourRequest: расчёт стоимости (взрослые + дети со скидкой + животные) ---
void test_request_cost() {
    Address reg = makeAddress();
    Address act = reg;
    Client cl("Тест", "Имя", "", "1", "a@a.ru", QDate(1980,1,1), reg, act, "");
    Tour tr("Тур", "РФ", "Экскурсия", QDate::currentDate().addDays(30), 5, 10000.0, true, false);
    TourRequest req(&cl, &tr);
    req.addAdult("Взрослый", "Один", "");
    req.addChild("Ребёнок", "Малый", "", QDate::currentDate().addYears(-5));
    req.addAnimal("Кот", 4.0, "Салон");
    double cost = req.calculateTotalCost();
    // 1*10000 + 1*10000*0.5 + (1000 + 4*5) = 10000 + 5000 + 1020 = 16020
    assert(cost > 16000 && cost < 16100);
}

// --- 6. Document: типы и статусы ---
void test_document_types() {
    Document d(DocumentType::Visa, DocumentStatus::Available);
    assert(d.getType() == DocumentType::Visa);
    assert(d.getStatus() == DocumentStatus::Available);
    assert(!Document::typeName(DocumentType::Passport).isEmpty());
    assert(!Document::statusName(DocumentStatus::Verified).isEmpty());
}

// --- 7. TourRequest: автогенерация документов ---
void test_documents_generated() {
    Address reg = makeAddress();
    Address act = reg;
    Client cl("К", "Л", "", "1", "a@a.ru", QDate(1985,1,1), reg, act, "");
    Tour tr("Загран", "Турция", "Пляж", QDate::currentDate().addDays(60), 7, 50000.0, false, true);
    TourRequest r(&cl, &tr);
    r.addAdult("Взрослый", "Турист", "");
    r.addChild("Ребёнок", "Турист", "", QDate(2018, 6, 1));
    r.regenerateDocuments();
    const auto& adultDocs = r.getTourists()[0]->documents();
    const auto& childDocs = r.getTourists()[1]->documents();
    assert(adultDocs.size() >= 1);
    assert(childDocs.size() >= 2);
}

// --- 8. TravelAgency: add, search, createRequest, getSalesHistory ---
void test_agency_crud() {
    TravelAgency a;
    Address reg = makeAddress();
    Address act = reg;
    Client* c = a.addClient("Сидоров", "Имя", "", "2", "s@r.ru", QDate(1975,2,2), reg, act, "");
    assert(c != nullptr && c->getFullName().contains("Сидоров"));
    Tour* t = a.addTour("Турция", "Турция", "Тур", QDate::currentDate().addDays(10),
                        7, 30000.0, false, true, {"Самолёт", "Поезд"});
    assert(t != nullptr);
    TourRequest* r = a.createRequest(c->getId(), t->getId());
    assert(r != nullptr);
    auto hist = a.getSalesHistoryForClient(c->getId());
    assert(hist.size() == 1 && hist[0]->getId() == r->getId());
    auto found = a.searchClients("Сидор");
    assert(found.size() >= 1);
}

// --- 9. Предупреждения о документах ---
void test_document_warnings() {
    Address reg = makeAddress();
    Address act = reg;
    Client cl("X", "Y", "", "1", "a@a.ru", QDate(1980,1,1), reg, act, "");
    Tour tr("Т", "РФ", "Т", QDate::currentDate().addDays(1), 1, 1000.0, true, false);
    TourRequest r(&cl, &tr);
    r.addAdult("Человек", "П", "");
    r.regenerateDocuments();
    // Все документы по умолчанию Absent → должны быть предупреждения
    QStringList w = r.getDocumentWarnings();
    assert(!w.isEmpty());
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    fprintf(stderr, "Тесты: Туристическое агентство\n");
    RUN_TEST(test_client_create);
    RUN_TEST(test_tour_create);
    RUN_TEST(test_animal_validate);
    RUN_TEST(test_child_age);
    RUN_TEST(test_request_cost);
    RUN_TEST(test_document_types);
    RUN_TEST(test_documents_generated);
    RUN_TEST(test_agency_crud);
    RUN_TEST(test_document_warnings);
    fprintf(stderr, "Все тесты пройдены.\n");
    return 0;
}
