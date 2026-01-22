/**
 * @file mainwindow.cpp
 * @brief Реализация главного окна. Формы ввода, таблицы, сообщения об ошибках.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QHeaderView>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QToolButton>
#include <QRegularExpressionValidator>

#include "documents_dialog.h"
#include "validation_service.h"
#include "document_service.h"
#include "request_service.h"
#include "client_service.h"
// Для режима редактирования клиента/тура (0 = добавление)
static const int NO_EDIT_ID = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    ui->setupUi(this);

    // --- Заголовки таблиц ---
    ui->clientsTable->setHorizontalHeaderLabels({"ID", "ФИО", "Телефон", "Email", "Дата рождения", "Комментарии"});
    ui->clientsTable->horizontalHeader()->setStretchLastSection(true);

    ui->toursTable->setHorizontalHeaderLabels({"ID", "Название", "Страна", "Тип", "Начало", "Дней", "Цена", "Внутр.", "Виза"});
    ui->toursTable->horizontalHeader()->setStretchLastSection(true);

    ui->requestsTable->setHorizontalHeaderLabels({"ID", "Клиент", "Тур", "Статус", "Стоимость"});
    ui->requestsTable->horizontalHeader()->setStretchLastSection(true);

    // --- Типы туров ---
    ui->tourType->addItems({
        "Экскурсионный",
        "Пляжный",
        "Лечебный",
        "Горнолыжный",
        "Круиз",
        "Экстремальный"
    });

    // --- Статусы заявки ---
    ui->requestStatusCombo->addItem("Черновик",   (int)RequestStatus::Draft);
    ui->requestStatusCombo->addItem("Оформлена",  (int)RequestStatus::Completed);
    ui->requestStatusCombo->addItem("Оплачена",   (int)RequestStatus::Paid);
    ui->requestStatusCombo->addItem("Отменена",   (int)RequestStatus::Canceled);

    // --- Перевозка животных ---
    ui->travelModeCombo->setToolTip("Способ поездки");
    ui->animalTransportCombo->setToolTip("Способ перевозки животного");
    if (QComboBox* cls = travelClassCombo()) {
        cls->setToolTip("Тип перевозки туристов");
    }
    refreshAnimalTransportOptions();

    const QDate today = QDate::currentDate();
    ui->childDobEdit->setMaximumDate(today);
    ui->childDobEdit->setMinimumDate(today.addYears(-18));
    ui->childDobEdit->setDate(today.addYears(-10));
    ui->clientErrorLabel->setStyleSheet("color: #b00020; font-weight: 600;");
    ui->requestErrorLabel->setStyleSheet("color: #b00020; font-weight: 600;");
    ui->clientErrorLabel->setWordWrap(true);
    ui->requestErrorLabel->setWordWrap(true);

    const QRegularExpression nameRegex = ValidationService::nameRegex();
    auto* nameValidator = new QRegularExpressionValidator(nameRegex, this);
    ui->clientLastName->setValidator(nameValidator);
    ui->clientFirstName->setValidator(nameValidator);
    ui->clientMiddleName->setValidator(nameValidator);
    ui->touristLastNameEdit->setValidator(nameValidator);
    ui->touristFirstNameEdit->setValidator(nameValidator);
    ui->touristMiddleNameEdit->setValidator(nameValidator);
    const QString nameHint = "Только кириллица и дефис, без пробелов";
    ui->clientLastName->setToolTip(nameHint);
    ui->clientFirstName->setToolTip(nameHint);
    ui->clientMiddleName->setToolTip(nameHint);
    ui->touristLastNameEdit->setToolTip(nameHint);
    ui->touristFirstNameEdit->setToolTip(nameHint);
    ui->touristMiddleNameEdit->setToolTip(nameHint);

    auto* addrTextValidator = new QRegularExpressionValidator(ValidationService::addressTextRegex(), this);
    ui->regRegionEdit->setValidator(addrTextValidator);
    ui->regCityEdit->setValidator(addrTextValidator);
    ui->regStreetEdit->setValidator(addrTextValidator);
    ui->actRegionEdit->setValidator(addrTextValidator);
    ui->actCityEdit->setValidator(addrTextValidator);
    ui->actStreetEdit->setValidator(addrTextValidator);

    auto* houseValidator = new QRegularExpressionValidator(ValidationService::houseRegex(), this);
    ui->regHouseEdit->setValidator(houseValidator);
    ui->regBuildingEdit->setValidator(houseValidator);
    ui->regApartmentEdit->setValidator(houseValidator);
    ui->actHouseEdit->setValidator(houseValidator);
    ui->actBuildingEdit->setValidator(houseValidator);
    ui->actApartmentEdit->setValidator(houseValidator);

    auto* postalValidator = new QRegularExpressionValidator(ValidationService::postalCodeRegex(), this);
    ui->regPostalEdit->setValidator(postalValidator);
    ui->actPostalEdit->setValidator(postalValidator);
    applyActualAddressEnabled(!ui->sameAddressCheck->isChecked());

    // --- Скрываем формы до нажатия Добавить/Изменить ---
    showClientForm(false, false);
    showTourForm(false, false);
    showRequestDetails(false);

    // --- Подключение сигналов. Клиенты ---
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(ui->addClientButton, &QPushButton::clicked, this, &MainWindow::onAddClient);
    connect(ui->editClientButton, &QPushButton::clicked, this, &MainWindow::onEditClient);
    connect(ui->deleteClientButton, &QPushButton::clicked, this, &MainWindow::onDeleteClient);
    connect(ui->clientSaveButton, &QPushButton::clicked, this, &MainWindow::onClientSave);
    connect(ui->clientCancelButton, &QPushButton::clicked, this, &MainWindow::onClientCancel);
    connect(ui->clientsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onClientSelectionChanged);

    // --- Туры ---
    connect(ui->addTourButton, &QPushButton::clicked, this, &MainWindow::onAddTour);
    connect(ui->editTourButton, &QPushButton::clicked, this, &MainWindow::onEditTour);
    connect(ui->deleteTourButton, &QPushButton::clicked, this, &MainWindow::onDeleteTour);
    connect(ui->tourSaveButton, &QPushButton::clicked, this, &MainWindow::onTourSave);
    connect(ui->tourCancelButton, &QPushButton::clicked, this, &MainWindow::onTourCancel);

    // --- Заявки ---
    connect(ui->createRequestButton, &QPushButton::clicked, this, &MainWindow::onCreateRequest);
    connect(ui->deleteRequestButton, &QPushButton::clicked, this, &MainWindow::onDeleteRequest);
    connect(ui->newRequestClientSearchButton, &QPushButton::clicked, this, &MainWindow::onSelectRequestClient);
    connect(ui->newRequestTourSearchButton, &QPushButton::clicked, this, &MainWindow::onSelectRequestTour);

    connect(ui->requestsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRequestSelectionChanged);

    connect(ui->requestStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onRequestStatusChanged);

    connect(ui->travelModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTravelModeChanged);

    if (QComboBox* cls = travelClassCombo()) {
        connect(cls, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MainWindow::onTravelClassChanged);
    }

    connect(ui->addAdultButton, &QPushButton::clicked, this, &MainWindow::onAddAdult);
    connect(ui->addChildButton, &QPushButton::clicked, this, &MainWindow::onAddChild);
    connect(ui->removeTouristButton, &QPushButton::clicked, this, &MainWindow::onRemoveTourist);
    connect(ui->addAnimalButton, &QPushButton::clicked, this, &MainWindow::onAddAnimal);
    connect(ui->removeAnimalButton, &QPushButton::clicked, this, &MainWindow::onRemoveAnimal);
    connect(ui->saveRequestButton, &QPushButton::clicked, this, &MainWindow::onSaveRequest);
    connect(ui->saveRequestCloseButton, &QPushButton::clicked, this, &MainWindow::onSaveRequestAndClose);
    connect(ui->openDocumentsButton, &QPushButton::clicked, this, &MainWindow::onOpenDocumentsDialog);
    connect(ui->sameAddressCheck, &QCheckBox::toggled, this, &MainWindow::onSameAddressToggled);

    connect(ui->requestDetailsToggle, &QToolButton::toggled, this, [this](bool checked) {
        if (!ui->requestDetailsToggle->isEnabled()) return;
        ui->requestDetailsGroup->setVisible(checked);
        ui->requestDetailsToggle->setText(checked ? "Скрыть детали заявки" : "Показать детали заявки");
    });

    // --- Файл ---
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveFile);
    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::onLoadFile);

    Q_UNUSED(NO_EDIT_ID);

    refreshClientsTable();
    refreshToursTable();
    refreshRequestsTable();
    refreshNewRequestCombos();
}

MainWindow::~MainWindow() {
    delete ui;
}

//-----------------------------------------------------------------------------
// Клиенты
//-----------------------------------------------------------------------------
void MainWindow::refreshClientsTable() {
    refreshClientsTable(agency_.clients());
}

void MainWindow::refreshClientsTable(const std::vector<Client*>& list) {
    ui->clientsTable->setRowCount((int)list.size());
    for (int i = 0; i < (int)list.size(); ++i) {
        Client* c = list[i];
        ui->clientsTable->setItem(i, 0, new QTableWidgetItem(QString::number(c->getId())));
        ui->clientsTable->setItem(i, 1, new QTableWidgetItem(c->getFullName()));
        ui->clientsTable->setItem(i, 2, new QTableWidgetItem(c->getPhone()));
        ui->clientsTable->setItem(i, 3, new QTableWidgetItem(c->getEmail()));
        ui->clientsTable->setItem(i, 4, new QTableWidgetItem(c->getDateOfBirth().toString(Qt::ISODate)));
        ui->clientsTable->setItem(i, 5, new QTableWidgetItem(c->getComments()));
    }
}

void MainWindow::onSearch() {
    const QString q = ui->searchEdit->text().trimmed();
    if (q.isEmpty()) refreshClientsTable();
    else refreshClientsTable(agency_.searchClients(q));
}

void MainWindow::onAddClient() {
    showClientForm(true, false);
    ui->clientLastName->clear();
    ui->clientFirstName->clear();
    ui->clientMiddleName->clear();
    ui->clientPhone->clear();
    ui->clientEmail->clear();
    ui->clientDateOfBirth->setDate(QDate::currentDate().addYears(-30));
    ui->clientComments->clear();
    ui->regRegionEdit->clear();
    ui->regCityEdit->clear();
    ui->regStreetEdit->clear();
    ui->regHouseEdit->clear();
    ui->regBuildingEdit->clear();
    ui->regApartmentEdit->clear();
    ui->regPostalEdit->clear();
    ui->regAdditionalEdit->clear();
    ui->sameAddressCheck->setChecked(false);
    ui->actRegionEdit->clear();
    ui->actCityEdit->clear();
    ui->actStreetEdit->clear();
    ui->actHouseEdit->clear();
    ui->actBuildingEdit->clear();
    ui->actApartmentEdit->clear();
    ui->actPostalEdit->clear();
    ui->actAdditionalEdit->clear();
    ui->clientErrorLabel->clear();
}

void MainWindow::onEditClient() {
    const int id = getSelectedClientId();
    if (id == 0) { QMessageBox::information(this, "Ошибка", "Выберите клиента."); return; }

    Client* c = agency_.findClientById(id);
    if (!c) return;

    showClientForm(true, true, id);
    ui->clientLastName->setText(c->getLastName());
    ui->clientFirstName->setText(c->getFirstName());
    ui->clientMiddleName->setText(c->getMiddleName());
    ui->clientPhone->setText(c->getPhone());
    ui->clientEmail->setText(c->getEmail());
    ui->clientDateOfBirth->setDate(c->getDateOfBirth());
    ui->clientComments->setPlainText(c->getComments());
    const Address reg = c->getRegistrationAddress();
    const Address act = c->getActualAddress();
    ui->regRegionEdit->setText(reg.region);
    ui->regCityEdit->setText(reg.city);
    ui->regStreetEdit->setText(reg.street);
    ui->regHouseEdit->setText(reg.house);
    ui->regBuildingEdit->setText(reg.building);
    ui->regApartmentEdit->setText(reg.apartment);
    ui->regPostalEdit->setText(reg.postalCode);
    ui->regAdditionalEdit->setText(reg.additional);
    ui->sameAddressCheck->setChecked(reg.region == act.region && reg.city == act.city
                                     && reg.street == act.street && reg.house == act.house
                                     && reg.building == act.building && reg.apartment == act.apartment
                                     && reg.postalCode == act.postalCode && reg.additional == act.additional);
    ui->actRegionEdit->setText(act.region);
    ui->actCityEdit->setText(act.city);
    ui->actStreetEdit->setText(act.street);
    ui->actHouseEdit->setText(act.house);
    ui->actBuildingEdit->setText(act.building);
    ui->actApartmentEdit->setText(act.apartment);
    ui->actPostalEdit->setText(act.postalCode);
    ui->actAdditionalEdit->setText(act.additional);
    ui->clientErrorLabel->clear();
}

void MainWindow::onDeleteClient() {
    const int id = getSelectedClientId();
    if (id == 0) { QMessageBox::information(this, "Ошибка", "Выберите клиента."); return; }

    if (QMessageBox::question(this, "Подтверждение", "Удалить клиента?") != QMessageBox::Yes) return;

    QString err;
    if (!agency_.deleteClient(id, &err)) {
        QMessageBox::warning(this, "Ошибка", err);
        return;
    }

    showClientForm(false, false);
    refreshClientsTable();
    refreshNewRequestCombos();
    ui->salesHistoryList->clear();
}

void MainWindow::onClientSave() {
    const QString last = ui->clientLastName->text().trimmed();
    const QString first = ui->clientFirstName->text().trimmed();
    const QString middle = ui->clientMiddleName->text().trimmed();
    const QString ph = ui->clientPhone->text().trimmed();
    const QString em = ui->clientEmail->text().trimmed();
    const QDate dob = ui->clientDateOfBirth->date();
    const QString cm = ui->clientComments->toPlainText().trimmed();
    const Address reg = collectRegistrationAddress();
    const Address act = ui->sameAddressCheck->isChecked() ? reg : collectActualAddress();

    ui->clientErrorLabel->clear();
    auto mark = [](QWidget* w, bool ok) {
        w->setStyleSheet(ok ? "" : "border: 1px solid #b00020;");
    };
    QString err;
    const bool lastOk = ValidationService::validateNamePart(last, &err);
    mark(ui->clientLastName, lastOk);
    if (!lastOk) { ui->clientErrorLabel->setText(err); return; }
    const bool firstOk = ValidationService::validateNamePart(first, &err);
    mark(ui->clientFirstName, firstOk);
    if (!firstOk) { ui->clientErrorLabel->setText(err); return; }
    const bool middleOk = ValidationService::validateOptionalNamePart(middle, &err);
    mark(ui->clientMiddleName, middleOk);
    if (!middleOk) { ui->clientErrorLabel->setText(err); return; }

    const bool regOk = ValidationService::validateAddress(reg, &err);
    mark(ui->regRegionEdit, regOk);
    mark(ui->regCityEdit, regOk);
    mark(ui->regStreetEdit, regOk);
    mark(ui->regHouseEdit, regOk);
    mark(ui->regPostalEdit, regOk);
    if (!regOk) { ui->clientErrorLabel->setText(err); return; }
    const bool actOk = ValidationService::validateAddress(act, &err);
    mark(ui->actRegionEdit, actOk);
    mark(ui->actCityEdit, actOk);
    mark(ui->actStreetEdit, actOk);
    mark(ui->actHouseEdit, actOk);
    mark(ui->actPostalEdit, actOk);
    if (!actOk) { ui->clientErrorLabel->setText(err); return; }

    if (ph.isEmpty()) { ui->clientErrorLabel->setText("Телефон обязателен"); mark(ui->clientPhone, false); return; }
    mark(ui->clientPhone, true);
    if (em.isEmpty()) { ui->clientErrorLabel->setText("Email обязателен"); mark(ui->clientEmail, false); return; }
    mark(ui->clientEmail, true);

    const int editId = ui->clientFormGroup->property("editingId").toInt();
    if (editId == 0) {
        Client* c = agency_.addClient(last, first, middle, ph, em, dob, reg, act, cm, &err);
        if (!c) { ui->clientErrorLabel->setText(err.isEmpty() ? "Не удалось добавить клиента." : err); return; }
    } else {
        if (!agency_.editClient(editId, last, first, middle, ph, em, dob, reg, act, cm, &err)) {
            ui->clientErrorLabel->setText(err);
            return;
        }
    }

    showClientForm(false, false);
    refreshClientsTable();
    refreshNewRequestCombos();
}

void MainWindow::onClientCancel() {
    showClientForm(false, false);
}

void MainWindow::onClientSelectionChanged() {
    const int id = getSelectedClientId();
    ui->salesHistoryList->clear();
    if (id == 0) return;

    for (TourRequest* r : agency_.getSalesHistoryForClient(id)) {
        const QString s = "Заявка #" + QString::number(r->getId()) +
                          " | " + r->getTour()->getName() +
                          " | " + QString::number(r->calculateTotalCost(), 'f', 2) + " руб.";
        ui->salesHistoryList->addItem(s);
    }
}

void MainWindow::showClientForm(bool show, bool forEdit, int editId) {
    ui->clientFormGroup->setVisible(show);
    ui->clientFormGroup->setProperty("editingId", forEdit ? editId : 0);
    if (!show) ui->clientErrorLabel->clear();
}

//-----------------------------------------------------------------------------
// Туры
//-----------------------------------------------------------------------------
void MainWindow::refreshToursTable() {
    ui->toursTable->setRowCount((int)agency_.tours().size());
    for (int i = 0; i < (int)agency_.tours().size(); ++i) {
        Tour* t = agency_.tours()[i];
        ui->toursTable->setItem(i, 0, new QTableWidgetItem(QString::number(t->getId())));
        ui->toursTable->setItem(i, 1, new QTableWidgetItem(t->getName()));
        ui->toursTable->setItem(i, 2, new QTableWidgetItem(t->getCountry()));
        ui->toursTable->setItem(i, 3, new QTableWidgetItem(t->getTourType()));
        ui->toursTable->setItem(i, 4, new QTableWidgetItem(t->getStartDate().toString(Qt::ISODate)));
        ui->toursTable->setItem(i, 5, new QTableWidgetItem(QString::number(t->getDurationDays())));
        ui->toursTable->setItem(i, 6, new QTableWidgetItem(QString::number(t->getBasePrice(), 'f', 2)));
        ui->toursTable->setItem(i, 7, new QTableWidgetItem(t->isDomestic() ? "Да" : "Нет"));
        ui->toursTable->setItem(i, 8, new QTableWidgetItem(t->isVisaRequired() ? "Да" : "Нет"));
    }
}

void MainWindow::onAddTour() {
    showTourForm(true, false);

    ui->tourName->clear();
    ui->tourCountry->clear();
    ui->tourType->setCurrentIndex(0);
    ui->tourStartDate->setDate(QDate::currentDate().addDays(7));
    ui->tourDuration->setValue(7);
    ui->tourBasePrice->setValue(10000);
    ui->tourDomestic->setChecked(true);
    ui->tourVisaRequired->setChecked(false);
    ui->tourTravelPlane->setChecked(true);
    ui->tourTravelTrain->setChecked(true);
}

void MainWindow::onEditTour() {
    const int id = getSelectedTourId();
    if (id == 0) { QMessageBox::information(this, "Ошибка", "Выберите тур."); return; }

    Tour* t = agency_.findTourById(id);
    if (!t) return;

    showTourForm(true, true, id);

    ui->tourName->setText(t->getName());
    ui->tourCountry->setText(t->getCountry());

    int typeIndex = ui->tourType->findText(t->getTourType());
    if (typeIndex < 0) {
        ui->tourType->addItem(t->getTourType());
        typeIndex = ui->tourType->count() - 1;
    }
    ui->tourType->setCurrentIndex(typeIndex);

    ui->tourStartDate->setDate(t->getStartDate());
    ui->tourDuration->setValue(t->getDurationDays());
    ui->tourBasePrice->setValue(t->getBasePrice());
    ui->tourDomestic->setChecked(t->isDomestic());
    ui->tourVisaRequired->setChecked(t->isVisaRequired());

    const QStringList modes = t->getTravelModes();
    ui->tourTravelPlane->setChecked(modes.contains("Самолёт"));
    ui->tourTravelTrain->setChecked(modes.contains("Поезд"));
}

void MainWindow::onDeleteTour() {
    const int id = getSelectedTourId();
    if (id == 0) { QMessageBox::information(this, "Ошибка", "Выберите тур."); return; }

    if (QMessageBox::question(this, "Подтверждение", "Удалить тур?") != QMessageBox::Yes) return;

    QString err;
    if (!agency_.deleteTour(id, &err)) { QMessageBox::warning(this, "Ошибка", err); return; }

    showTourForm(false, false);
    refreshToursTable();
    refreshNewRequestCombos();
}

void MainWindow::onTourSave() {
    const QString nm = ui->tourName->text().trimmed();
    const QString co = ui->tourCountry->text().trimmed();
    const QString tt = ui->tourType->currentText().trimmed();
    const QDate sd = ui->tourStartDate->date();
    const int dur = ui->tourDuration->value();
    const double pr = ui->tourBasePrice->value();
    const bool dom = ui->tourDomestic->isChecked();
    const bool visa = ui->tourVisaRequired->isChecked();

    QStringList travelModes;
    if (ui->tourTravelPlane->isChecked()) travelModes << "Самолёт";
    if (ui->tourTravelTrain->isChecked()) travelModes << "Поезд";

    if (nm.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Название тура не может быть пустым."); return; }
    if (tt.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Выберите тип тура."); return; }
    if (travelModes.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Выберите хотя бы один способ проезда."); return; }

    const int editId = ui->tourFormGroup->property("editingId").toInt();
    if (editId == 0) {
        Tour* t = agency_.addTour(nm, co, tt, sd, dur, pr, dom, visa, travelModes);
        if (!t) { QMessageBox::warning(this, "Ошибка", "Не удалось добавить тур."); return; }
    } else {
        QString err;
        if (!agency_.editTour(editId, nm, co, tt, sd, dur, pr, dom, visa, travelModes, &err)) {
            QMessageBox::warning(this, "Ошибка", err);
            return;
        }
    }

    showTourForm(false, false);
    refreshToursTable();
    refreshNewRequestCombos();
}

void MainWindow::onTourCancel() {
    showTourForm(false, false);
}

void MainWindow::showTourForm(bool show, bool forEdit, int editId) {
    ui->tourFormGroup->setVisible(show);
    ui->tourFormGroup->setProperty("editingId", forEdit ? editId : 0);
}

//-----------------------------------------------------------------------------
// Заявки
//-----------------------------------------------------------------------------
void MainWindow::refreshRequestsTable() {
    ui->requestsTable->setRowCount((int)agency_.requests().size());
    for (int i = 0; i < (int)agency_.requests().size(); ++i) {
        TourRequest* r = agency_.requests()[i];

        QString statusStr;
        switch (r->getStatus()) {
        case RequestStatus::Draft:     statusStr = "Черновик"; break;
        case RequestStatus::Completed: statusStr = "Оформлена"; break;
        case RequestStatus::Paid:      statusStr = "Оплачена"; break;
        case RequestStatus::Canceled:  statusStr = "Отменена"; break;
        }

        ui->requestsTable->setItem(i, 0, new QTableWidgetItem(QString::number(r->getId())));
        ui->requestsTable->setItem(i, 1, new QTableWidgetItem(r->getClient()->getFullName()));
        ui->requestsTable->setItem(i, 2, new QTableWidgetItem(r->getTour()->getName()));
        ui->requestsTable->setItem(i, 3, new QTableWidgetItem(statusStr));
        ui->requestsTable->setItem(i, 4, new QTableWidgetItem(QString::number(r->calculateTotalCost(), 'f', 2)));
    }
}

void MainWindow::refreshNewRequestCombos() {
    ui->newRequestClientCombo->clear();
    ui->newRequestTourCombo->clear();

    for (Client* c : agency_.clients())
        ui->newRequestClientCombo->addItem(c->getFullName(), c->getId());

    for (Tour* t : agency_.tours())
        ui->newRequestTourCombo->addItem(t->getName(), t->getId());
}

void MainWindow::refreshTravelModeOptions(TourRequest* request) {
    ui->travelModeCombo->blockSignals(true);
    ui->travelModeCombo->clear();

    if (!request) {
        ui->travelModeCombo->setEnabled(false);
        ui->travelModeCombo->blockSignals(false);
        refreshAnimalTransportOptions();
        return;
    }

    QStringList modes = request->getTour()->getTravelModes();
    if (modes.isEmpty()) modes = {"Самолёт", "Поезд"};

    for (const QString& mode : modes)
        ui->travelModeCombo->addItem(mode);

    QString current = request->getTravelMode();
    int idx = ui->travelModeCombo->findText(current);
    if (idx < 0 && !modes.isEmpty()) {
        current = modes.first();
        request->setTravelMode(current);
        idx = 0;
    }
    if (idx >= 0) ui->travelModeCombo->setCurrentIndex(idx);

    ui->travelModeCombo->setEnabled(modes.size() > 1);
    ui->travelModeCombo->blockSignals(false);

    refreshAnimalTransportOptions();
}

void MainWindow::refreshTravelClassOptions(TourRequest* request) {
    QComboBox* clsCb = travelClassCombo();
    if (!clsCb) return;

    clsCb->blockSignals(true);
    clsCb->clear();

    if (!request) {
        clsCb->setEnabled(false);
        clsCb->blockSignals(false);
        return;
    }

    const QStringList classes = TourRequest::travelClassOptionsForMode(request->getTravelMode());
    for (const QString& cls : classes)
        clsCb->addItem(cls);

    QString current = request->getTravelClass();
    int idx = clsCb->findText(current);
    if (idx < 0 && !classes.isEmpty()) {
        request->setTravelClass(classes.first());
        idx = 0;
    }
    if (idx >= 0) clsCb->setCurrentIndex(idx);

    clsCb->setEnabled(classes.size() > 1);
    clsCb->blockSignals(false);
}

void MainWindow::onCreateRequest() {
    const int cid = ui->newRequestClientCombo->currentData().toInt();
    const int tid = ui->newRequestTourCombo->currentData().toInt();
    if (cid == 0 || tid == 0) { QMessageBox::information(this, "Ошибка", "Выберите клиента и тур."); return; }

    QString err;
    TourRequest* r = agency_.createRequest(cid, tid, &err);
    if (!r) { QMessageBox::warning(this, "Ошибка", err); return; }

    refreshRequestsTable();

    // Выбрать новую заявку и показать детали
    for (int i = 0; i < ui->requestsTable->rowCount(); ++i) {
        if (ui->requestsTable->item(i, 0)->text().toInt() == r->getId()) {
            ui->requestsTable->selectRow(i);
            break;
        }
    }
    refreshRequestDetails();
}

void MainWindow::onDeleteRequest() {
    const int id = getSelectedRequestId();
    if (id == 0) { QMessageBox::information(this, "Ошибка", "Выберите заявку."); return; }

    if (QMessageBox::question(this, "Подтверждение", "Удалить заявку?") != QMessageBox::Yes) return;

    QString err;
    if (!agency_.deleteRequest(id, &err)) { QMessageBox::warning(this, "Ошибка", err); return; }

    showRequestDetails(false);
    refreshRequestsTable();
}

void MainWindow::onSelectRequestClient() {
    const int id = selectClientFromDialog();
    if (id == 0) return;

    const int idx = ui->newRequestClientCombo->findData(id);
    if (idx >= 0) ui->newRequestClientCombo->setCurrentIndex(idx);
}

void MainWindow::onSelectRequestTour() {
    const int id = selectTourFromDialog();
    if (id == 0) return;

    const int idx = ui->newRequestTourCombo->findData(id);
    if (idx >= 0) ui->newRequestTourCombo->setCurrentIndex(idx);
}

void MainWindow::onRequestSelectionChanged() {
    refreshRequestDetails();
}

void MainWindow::onRequestStatusChanged(int index) {
    const int id = getActiveRequestId();
    if (id == 0) return;

    TourRequest* r = agency_.findRequestById(id);
    if (!r) return;

    const RequestStatus s = (RequestStatus)ui->requestStatusCombo->itemData(index).toInt();
    r->setStatus(s);
    refreshRequestsTable();
}

void MainWindow::onTravelModeChanged(int index) {
    Q_UNUSED(index);

    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) return;

    const QString mode = ui->travelModeCombo->currentText().trimmed();
    if (!mode.isEmpty())
        r->setTravelMode(mode);

    refreshAnimalTransportOptions();
    refreshTravelClassOptions(r);
    refreshRequiredDocuments(r);
}

void MainWindow::onTravelClassChanged(int index) {
    Q_UNUSED(index);

    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) return;

    QComboBox* clsCb = travelClassCombo();
    if (!clsCb) return;

    const QString travelClass = clsCb->currentText().trimmed();
    if (!travelClass.isEmpty())
        r->setTravelClass(travelClass);
}

void MainWindow::refreshRequestDetails() {
    const int id = getSelectedRequestId();
    TourRequest* r = id ? agency_.findRequestById(id) : nullptr;

    if (!r) {
        showRequestDetails(false);
        return;
    }

    ui->requestDetailsGroup->setProperty("requestId", r->getId());
    showRequestDetails(true);
    ui->requestErrorLabel->clear();

    ui->requestClientName->setText(r->getClient()->getFullName());
    ui->requestTourName->setText(r->getTour()->getName());
    ui->requestCostLabel->setText(QString::number(r->calculateTotalCost(), 'f', 2) + " руб.");

    int adults = 0;
    int children = 0;
    for (const auto& t : r->getTourists()) {
        if (t->isChild()) ++children;
        else ++adults;
    }

    const int animalsCount = (int)r->getAnimals().size();
    const QString breakdown = QString("Взр.: %1 × %2, Дет.: %3 × %2 × 0.5, Жив.: %4 (1000 + 5×кг)")
                                  .arg(adults)
                                  .arg(QString::number(r->getTour()->getBasePrice(), 'f', 2))
                                  .arg(children)
                                  .arg(animalsCount);
    ui->requestCostBreakdownLabel->setText(breakdown);

    // Статус заявки
    int si = 0;
    for (int i = 0; i < ui->requestStatusCombo->count(); ++i) {
        if (ui->requestStatusCombo->itemData(i).toInt() == (int)r->getStatus()) { si = i; break; }
    }
    ui->requestStatusCombo->blockSignals(true);
    ui->requestStatusCombo->setCurrentIndex(si);
    ui->requestStatusCombo->blockSignals(false);

    refreshTravelModeOptions(r);
    refreshTravelClassOptions(r);

    // Туристы
    ui->touristsList->clear();
    for (const auto& t : r->getTourists()) {
        const QString benefit = t->hasBenefit() ? " (льгота)" : "";
        ui->touristsList->addItem(t->displayName() + benefit);
    }

    // Животные
    ui->animalsList->clear();
    for (const auto& a : r->getAnimals())
        ui->animalsList->addItem(a->getType() + ", " + QString::number(a->getWeight()) + " кг, " + a->getTransport());

    refreshRequiredDocuments(r);

    const QStringList w = r->getDocumentWarnings() + r->getValidationWarnings();
    ui->warningsText->setPlainText(w.join("\n"));
}

void MainWindow::onSaveRequest() {
    TourRequest* request = agency_.findRequestById(getActiveRequestId());
    if (!request) {
        QMessageBox::information(this, "Ошибка", "Сначала выберите заявку.");
        return;
    }
    QString err;
    if (!RequestService::validateRequest(*request, &err)) {
        ui->requestErrorLabel->setText(err);
        return;
    }
    onSaveFile();
}

void MainWindow::onSaveRequestAndClose() {
    onSaveRequest();
    ui->requestDetailsToggle->setChecked(false);
}

void MainWindow::onAddAdult() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) { QMessageBox::information(this, "Ошибка", "Сначала выберите заявку."); return; }

    ui->requestErrorLabel->clear();
    const QString last = ui->touristLastNameEdit->text().trimmed();
    const QString first = ui->touristFirstNameEdit->text().trimmed();
    const QString middle = ui->touristMiddleNameEdit->text().trimmed();
    QString err;
    if (!ValidationService::validateNamePart(last, &err) ||
        !ValidationService::validateNamePart(first, &err) ||
        !ValidationService::validateOptionalNamePart(middle, &err)) {
        ui->requestErrorLabel->setText(err);
        return;
    }

    try {
        r->addAdult(last, first, middle);
        r->tourists().back()->setHasBenefit(ui->touristBenefitCheck->isChecked());
        ui->touristLastNameEdit->clear();
        ui->touristFirstNameEdit->clear();
        ui->touristMiddleNameEdit->clear();
        ui->touristBenefitCheck->setChecked(false);
        refreshRequestDetails();
        refreshRequestsTable();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", e.what());
    }
}

void MainWindow::onAddChild() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) { QMessageBox::information(this, "Ошибка", "Сначала выберите заявку."); return; }

    ui->requestErrorLabel->clear();
    const QString last = ui->touristLastNameEdit->text().trimmed();
    const QString first = ui->touristFirstNameEdit->text().trimmed();
    const QString middle = ui->touristMiddleNameEdit->text().trimmed();
    const QDate dob = ui->childDobEdit->date();

    QString err;
    if (!ValidationService::validateNamePart(last, &err) ||
        !ValidationService::validateNamePart(first, &err) ||
        !ValidationService::validateOptionalNamePart(middle, &err)) {
        ui->requestErrorLabel->setText(err);
        return;
    }
    if (!dob.isValid() || dob > QDate::currentDate()) { QMessageBox::warning(this, "Ошибка", "Укажите корректную дату рождения."); return; }

    int age = QDate::currentDate().year() - dob.year();
    if (QDate::currentDate().month() < dob.month() ||
        (QDate::currentDate().month() == dob.month() && QDate::currentDate().day() < dob.day())) {
        --age;
    }
    if (age > 18) { QMessageBox::warning(this, "Ошибка", "Возраст ребёнка не может быть больше 18 лет."); return; }

    try {
        r->addChild(last, first, middle, dob);
        r->tourists().back()->setHasBenefit(ui->touristBenefitCheck->isChecked());
        ui->touristLastNameEdit->clear();
        ui->touristFirstNameEdit->clear();
        ui->touristMiddleNameEdit->clear();
        ui->touristBenefitCheck->setChecked(false);
        refreshRequestDetails();
        refreshRequestsTable();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", e.what());
    }
}

void MainWindow::onRemoveTourist() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) return;

    const int row = ui->touristsList->currentRow();
    if (row < 0) { QMessageBox::information(this, "Ошибка", "Выберите туриста в списке."); return; }

    r->removeTourist(row);
    refreshRequestDetails();
    refreshRequestsTable();
}

void MainWindow::onAddAnimal() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) { QMessageBox::information(this, "Ошибка", "Сначала выберите заявку."); return; }

    const QString type = ui->animalTypeEdit->text().trimmed();
    const double w = ui->animalWeightSpin->value();
    const QString tr = ui->animalTransportCombo->currentText().trimmed();

    QString err;
    if (!Animal::validate(type, w, tr, &err)) { QMessageBox::warning(this, "Ошибка", err); return; }

    try {
        r->addAnimal(type, w, tr);
        ui->animalTypeEdit->clear();
        ui->animalTransportCombo->setCurrentIndex(0);
        refreshRequestDetails();
        refreshRequestsTable();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", e.what());
    }
}

void MainWindow::onRemoveAnimal() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) return;

    const int row = ui->animalsList->currentRow();
    if (row < 0) { QMessageBox::information(this, "Ошибка", "Выберите животное в списке."); return; }

    r->removeAnimal(row);
    refreshRequestDetails();
    refreshRequestsTable();
}

void MainWindow::showRequestDetails(bool show) {
    ui->requestDetailsToggle->setEnabled(show);

    if (!show) {
        ui->requestDetailsGroup->setVisible(false);
        ui->requestDetailsToggle->setText("Показать детали заявки");
        ui->requestDetailsGroup->setProperty("requestId", 0);
        refreshTravelModeOptions(nullptr);
        refreshTravelClassOptions(nullptr);
        ui->requiredDocsList->clear();
        ui->missingDocsText->clear();
        return;
    }

    ui->requestDetailsGroup->setVisible(ui->requestDetailsToggle->isChecked());
    ui->requestDetailsToggle->setText(ui->requestDetailsToggle->isChecked()
                                          ? "Скрыть детали заявки"
                                          : "Показать детали заявки");
}

void MainWindow::refreshRequiredDocuments(TourRequest* request) {
    ui->requiredDocsList->clear();
    ui->missingDocsText->clear();
    if (!request) return;

    for (const auto& tourist : request->getTourists()) {
        const auto required = DocumentService::requiredPersonalDocuments(*request, *tourist);
        for (const auto& docType : required) {
            ui->requiredDocsList->addItem(tourist->getFullName() + ": " + Document::typeName(docType));
        }
    }
    const auto requestDocs = DocumentService::requiredRequestDocuments(*request);
    for (const auto& docType : requestDocs) {
        ui->requiredDocsList->addItem("Заявка: " + Document::typeName(docType));
    }

    const QStringList missing = DocumentService::missingDocumentsSummary(*request);
    ui->missingDocsText->setPlainText(missing.join("\n"));
}

Address MainWindow::collectRegistrationAddress() const {
    Address a;
    a.region = ui->regRegionEdit->text().trimmed();
    a.city = ui->regCityEdit->text().trimmed();
    a.street = ui->regStreetEdit->text().trimmed();
    a.house = ui->regHouseEdit->text().trimmed();
    a.building = ui->regBuildingEdit->text().trimmed();
    a.apartment = ui->regApartmentEdit->text().trimmed();
    a.postalCode = ui->regPostalEdit->text().trimmed();
    a.additional = ui->regAdditionalEdit->text().trimmed();
    return a;
}

Address MainWindow::collectActualAddress() const {
    Address a;
    a.region = ui->actRegionEdit->text().trimmed();
    a.city = ui->actCityEdit->text().trimmed();
    a.street = ui->actStreetEdit->text().trimmed();
    a.house = ui->actHouseEdit->text().trimmed();
    a.building = ui->actBuildingEdit->text().trimmed();
    a.apartment = ui->actApartmentEdit->text().trimmed();
    a.postalCode = ui->actPostalEdit->text().trimmed();
    a.additional = ui->actAdditionalEdit->text().trimmed();
    return a;
}

void MainWindow::applyActualAddressEnabled(bool enabled) {
    ui->actualAddressGroup->setEnabled(enabled);
    ui->actualAddressGroup->setVisible(enabled);
}

void MainWindow::onSameAddressToggled(bool checked) {
    applyActualAddressEnabled(!checked);
    if (checked) {
        ui->actRegionEdit->setText(ui->regRegionEdit->text());
        ui->actCityEdit->setText(ui->regCityEdit->text());
        ui->actStreetEdit->setText(ui->regStreetEdit->text());
        ui->actHouseEdit->setText(ui->regHouseEdit->text());
        ui->actBuildingEdit->setText(ui->regBuildingEdit->text());
        ui->actApartmentEdit->setText(ui->regApartmentEdit->text());
        ui->actPostalEdit->setText(ui->regPostalEdit->text());
        ui->actAdditionalEdit->setText(ui->regAdditionalEdit->text());
    }
}

void MainWindow::onOpenDocumentsDialog() {
    TourRequest* r = agency_.findRequestById(getActiveRequestId());
    if (!r) { QMessageBox::information(this, "Ошибка", "Сначала выберите заявку."); return; }
    DocumentsDialog dialog(r, this);
    dialog.exec();
    refreshRequestDetails();
}

//-----------------------------------------------------------------------------
// Файл
//-----------------------------------------------------------------------------
void MainWindow::onSaveFile() {
    const QString path = ui->dataFilePath->text().trimmed().isEmpty()
    ? "agency_data.json"
    : ui->dataFilePath->text().trimmed();

    QString err;
    if (!agency_.saveToFile(path, &err)) {
        QMessageBox::warning(this, "Ошибка", err);
        return;
    }
    ui->fileStatusLabel->setText("Сохранено: " + path);
}

void MainWindow::onLoadFile() {
    const QString path = ui->dataFilePath->text().trimmed().isEmpty()
    ? "agency_data.json"
    : ui->dataFilePath->text().trimmed();

    QString err;
    if (!agency_.loadFromFile(path, &err)) {
        QMessageBox::warning(this, "Ошибка", err);
        return;
    }

    ui->fileStatusLabel->setText("Загружено: " + path);

    refreshClientsTable();
    refreshToursTable();
    refreshRequestsTable();
    refreshNewRequestCombos();
    showClientForm(false, false);
    showTourForm(false, false);
    showRequestDetails(false);
    ui->clientErrorLabel->clear();
    ui->requestErrorLabel->clear();
}

//-----------------------------------------------------------------------------
// Вспомогательные
//-----------------------------------------------------------------------------
int MainWindow::selectClientFromDialog() const {
    QDialog dialog(const_cast<MainWindow*>(this));
    dialog.setWindowTitle("Поиск клиента");
    dialog.resize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLineEdit* searchEdit = new QLineEdit(&dialog);
    searchEdit->setPlaceholderText("Введите имя, телефон или email...");
    layout->addWidget(searchEdit);

    QTableWidget* table = new QTableWidget(&dialog);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"ID", "ФИО", "Телефон", "Email"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto refresh = [this, table](const QString& query) {
        const std::vector<Client*> list = query.trimmed().isEmpty()
        ? agency_.clients()
        : agency_.searchClients(query);

        table->setRowCount((int)list.size());
        for (int i = 0; i < (int)list.size(); ++i) {
            Client* c = list[i];
            table->setItem(i, 0, new QTableWidgetItem(QString::number(c->getId())));
            table->setItem(i, 1, new QTableWidgetItem(c->getFullName()));
            table->setItem(i, 2, new QTableWidgetItem(c->getPhone()));
            table->setItem(i, 3, new QTableWidgetItem(c->getEmail()));
        }
    };

    refresh("");

    connect(searchEdit, &QLineEdit::textChanged, &dialog, [&refresh](const QString& text) {
        refresh(text);
    });

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    connect(table, &QTableWidget::cellDoubleClicked, &dialog, [&dialog](int, int) {
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) return 0;
    const int row = table->currentRow();
    if (row < 0) return 0;

    QTableWidgetItem* it = table->item(row, 0);
    return it ? it->text().toInt() : 0;
}

int MainWindow::selectTourFromDialog() const {
    QDialog dialog(const_cast<MainWindow*>(this));
    dialog.setWindowTitle("Поиск тура");
    dialog.resize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLineEdit* searchEdit = new QLineEdit(&dialog);
    searchEdit->setPlaceholderText("Введите название, страну или тип тура...");
    layout->addWidget(searchEdit);

    QTableWidget* table = new QTableWidget(&dialog);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"ID", "Название", "Страна", "Тип"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto refresh = [this, table](const QString& query) {
        const QString q = query.trimmed().toLower();
        std::vector<Tour*> list;
        list.reserve((size_t)agency_.tours().size());

        for (Tour* t : agency_.tours()) {
            if (q.isEmpty() ||
                t->getName().toLower().contains(q) ||
                t->getCountry().toLower().contains(q) ||
                t->getTourType().toLower().contains(q)) {
                list.push_back(t);
            }
        }

        table->setRowCount((int)list.size());
        for (int i = 0; i < (int)list.size(); ++i) {
            Tour* t = list[i];
            table->setItem(i, 0, new QTableWidgetItem(QString::number(t->getId())));
            table->setItem(i, 1, new QTableWidgetItem(t->getName()));
            table->setItem(i, 2, new QTableWidgetItem(t->getCountry()));
            table->setItem(i, 3, new QTableWidgetItem(t->getTourType()));
        }
    };

    refresh("");

    connect(searchEdit, &QLineEdit::textChanged, &dialog, [&refresh](const QString& text) {
        refresh(text);
    });

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    connect(table, &QTableWidget::cellDoubleClicked, &dialog, [&dialog](int, int) {
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) return 0;
    const int row = table->currentRow();
    if (row < 0) return 0;

    QTableWidgetItem* it = table->item(row, 0);
    return it ? it->text().toInt() : 0;
}

int MainWindow::getSelectedClientId() const {
    const int row = ui->clientsTable->currentRow();
    if (row < 0) return 0;
    QTableWidgetItem* it = ui->clientsTable->item(row, 0);
    return it ? it->text().toInt() : 0;
}

int MainWindow::getSelectedTourId() const {
    const int row = ui->toursTable->currentRow();
    if (row < 0) return 0;
    QTableWidgetItem* it = ui->toursTable->item(row, 0);
    return it ? it->text().toInt() : 0;
}

int MainWindow::getSelectedRequestId() const {
    const int row = ui->requestsTable->currentRow();
    if (row < 0) return 0;
    QTableWidgetItem* it = ui->requestsTable->item(row, 0);
    return it ? it->text().toInt() : 0;
}

int MainWindow::getActiveRequestId() const {
    const int id = ui->requestDetailsGroup->property("requestId").toInt();
    if (id != 0) return id;
    return getSelectedRequestId();
}

QComboBox* MainWindow::travelClassCombo() const {
    // Важно: не называй локальные переменные "travelClassCombo" (будет конфликт как раньше).
    return findChild<QComboBox*>("travelClassCombo");
}

void MainWindow::refreshAnimalTransportOptions() {
    ui->animalTransportCombo->clear();

    const QString mode = ui->travelModeCombo->currentText();
    if (mode.isEmpty()) return;

    if (mode == "Самолёт") {
        ui->animalTransportCombo->addItems({"Салон", "Багаж", "Карго"});
    } else if (mode == "Поезд") {
        ui->animalTransportCombo->addItems({
            "Переноска (до 180 см, до 2 животных)",
            "Отдельное купе (крупные собаки, выкуп всего купе)"
        });
    }
}
