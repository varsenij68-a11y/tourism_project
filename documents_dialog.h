#pragma once

#include <QDialog>
#include <QHash>
#include <QLabel>
#include <QPointer>
#include <QVariant>

#include "document_service.h"

class QComboBox;
class QListWidget;
class QPushButton;
class QFormLayout;
class QLineEdit;
class QDateEdit;
class TourRequest;
class Tourist;
class Document;

class DocumentsDialog : public QDialog {
    Q_OBJECT
public:
    explicit DocumentsDialog(TourRequest* request, QWidget* parent = nullptr);

private slots:
    void onOwnerChanged(int index);
    void onDocumentSelectionChanged();
    void onAddDocument();
    void onRemoveDocument();
    void onVerifyDocument();

private:
    struct FieldWidgets {
        QWidget* editor = nullptr;
        QLabel* errorLabel = nullptr;
        DocumentField def;
    };

    TourRequest* request_ = nullptr;
    QComboBox* ownerCombo_ = nullptr;
    QListWidget* docList_ = nullptr;
    QLabel* headerError_ = nullptr;
    QFormLayout* formLayout_ = nullptr;
    QPushButton* addButton_ = nullptr;
    QPushButton* removeButton_ = nullptr;
    QPushButton* verifyButton_ = nullptr;

    QHash<QString, FieldWidgets> fields_;

    void refreshOwnerCombo();
    void refreshDocumentList();
    Document* currentDocument() const;
    Tourist* currentTourist() const;
    void rebuildForm(Document* document);
    void updateDocumentStatus(Document* document);
    void updateErrorState(const QString& key, bool ok, const QString& message);
    QString normalizeInput(const QString& value, const DocumentField& field) const;
    bool isRequiredDocument(DocumentType type) const;
};
