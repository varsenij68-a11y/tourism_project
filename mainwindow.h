/**
 * @file mainwindow.h
 * @brief Главное окно приложения «Туристическое агентство. Клиенты, продажи.»
 * Разделение: интерфейс (MainWindow) и логика (TravelAgency в модулях agency.h/*).
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "agency.h"
#include "address.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QComboBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Клиенты
    void onSearch();
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void onClientSave();
    void onClientCancel();
    void onClientSelectionChanged();
    // Туры
    void onAddTour();
    void onEditTour();
    void onDeleteTour();
    void onTourSave();
    void onTourCancel();
    // Заявки
    void onCreateRequest();
    void onDeleteRequest();
    void onRequestSelectionChanged();
    void onRequestStatusChanged(int index);
    void onSelectRequestClient();
    void onSelectRequestTour();
    void onTravelModeChanged(int index);
    void onTravelClassChanged(int index);
    void onSaveRequest();
    void onSaveRequestAndClose();
    void onAddAdult();
    void onAddChild();
    void onRemoveTourist();
    void onAddAnimal();
    void onRemoveAnimal();
    void onOpenDocumentsDialog();
    void onSameAddressToggled(bool checked);
    // Файл
    void onSaveFile();
    void onLoadFile();

private:
    Ui::MainWindow *ui;
    TravelAgency agency_;

    void refreshClientsTable();
    void refreshClientsTable(const std::vector<Client*>& list);
    void refreshToursTable();
    void refreshRequestsTable();
    void refreshRequestDetails();
    void refreshNewRequestCombos();
    void refreshAnimalTransportOptions();
    void refreshTravelModeOptions(TourRequest* request);
    void refreshTravelClassOptions(TourRequest* request);
    void refreshRequiredDocuments(TourRequest* request);
    Address collectRegistrationAddress() const;
    Address collectActualAddress() const;
    void applyActualAddressEnabled(bool enabled);
    void showClientForm(bool show, bool forEdit, int editId = 0);
    void showTourForm(bool show, bool forEdit, int editId = 0);
    void showRequestDetails(bool show);
    int selectClientFromDialog() const;
    int selectTourFromDialog() const;
    int getSelectedClientId() const;
    int getSelectedTourId() const;
    int getSelectedRequestId() const;
    int getActiveRequestId() const;
    QComboBox* travelClassCombo() const;
};

#endif // MAINWINDOW_H
