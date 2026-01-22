#include "documents_dialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDate>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <algorithm>
#include <QPushButton>
#include <QVBoxLayout>

#include "document.h"
#include "tour_request.h"
#include "tourist.h"

DocumentsDialog::DocumentsDialog(TourRequest* request, QWidget* parent)
    : QDialog(parent), request_(request) {
    setWindowTitle("Документы");
    resize(900, 540);

    auto* mainLayout = new QVBoxLayout(this);
    headerError_ = new QLabel(this);
    headerError_->setStyleSheet("color: #b00020; font-weight: 600;");
    mainLayout->addWidget(headerError_);

    auto* topLayout = new QHBoxLayout();
    ownerCombo_ = new QComboBox(this);
    topLayout->addWidget(new QLabel("Документы для:", this));
    topLayout->addWidget(ownerCombo_, 1);
    mainLayout->addLayout(topLayout);

    auto* contentLayout = new QHBoxLayout();
    docList_ = new QListWidget(this);
    docList_->setMinimumWidth(260);
    contentLayout->addWidget(docList_);

    auto* formWidget = new QWidget(this);
    formLayout_ = new QFormLayout(formWidget);
    formLayout_->setLabelAlignment(Qt::AlignTop);
    formLayout_->setFormAlignment(Qt::AlignTop);
    contentLayout->addWidget(formWidget, 1);

    mainLayout->addLayout(contentLayout);

    auto* buttonLayout = new QHBoxLayout();
    addButton_ = new QPushButton("Добавить", this);
    removeButton_ = new QPushButton("Удалить", this);
    verifyButton_ = new QPushButton("Отметить проверенным", this);
    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(removeButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(verifyButton_);
    mainLayout->addLayout(buttonLayout);

    refreshOwnerCombo();
    refreshDocumentList();

    connect(ownerCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DocumentsDialog::onOwnerChanged);
    connect(docList_, &QListWidget::currentRowChanged,
            this, &DocumentsDialog::onDocumentSelectionChanged);
    connect(addButton_, &QPushButton::clicked, this, &DocumentsDialog::onAddDocument);
    connect(removeButton_, &QPushButton::clicked, this, &DocumentsDialog::onRemoveDocument);
    connect(verifyButton_, &QPushButton::clicked, this, &DocumentsDialog::onVerifyDocument);
}

void DocumentsDialog::refreshOwnerCombo() {
    ownerCombo_->clear();
    ownerCombo_->addItem("Заявка", QVariant::fromValue(-1));
    if (!request_) return;
    for (size_t i = 0; i < request_->getTourists().size(); ++i) {
        ownerCombo_->addItem(request_->getTourists()[i]->getFullName(), QVariant::fromValue((int)i));
    }
}

Tourist* DocumentsDialog::currentTourist() const {
    if (!request_) return nullptr;
    const int idx = ownerCombo_->currentData().toInt();
    if (idx < 0) return nullptr;
    if (idx >= (int)request_->getTourists().size()) return nullptr;
    return request_->tourists()[idx].get();
}

Document* DocumentsDialog::currentDocument() const {
    const int row = docList_->currentRow();
    if (row < 0) return nullptr;
    Tourist* tourist = currentTourist();
    if (tourist) {
        if (row >= (int)tourist->documents().size()) return nullptr;
        return tourist->documents()[row].get();
    }
    if (!request_) return nullptr;
    if (row >= (int)request_->documents().size()) return nullptr;
    return request_->documents()[row].get();
}

void DocumentsDialog::refreshDocumentList() {
    docList_->clear();
    Tourist* tourist = currentTourist();
    if (tourist) {
        for (const auto& doc : tourist->documents()) {
            docList_->addItem(doc->displayName() + " — " + Document::statusName(doc->getStatus()));
        }
    } else if (request_) {
        for (const auto& doc : request_->documents()) {
            docList_->addItem(doc->displayName() + " — " + Document::statusName(doc->getStatus()));
        }
    }
    if (docList_->count() > 0) docList_->setCurrentRow(0);
    else rebuildForm(nullptr);
}

void DocumentsDialog::onOwnerChanged(int) {
    refreshDocumentList();
}

void DocumentsDialog::onDocumentSelectionChanged() {
    rebuildForm(currentDocument());
}

QString DocumentsDialog::normalizeInput(const QString& value, const DocumentField& field) const {
    if (field.regex.pattern().contains("[A-Z")) {
        return value.toUpper();
    }
    if (field.regex.pattern() == "^\\d{16}$") {
        QString cleaned = value;
        cleaned.remove(' ');
        return cleaned;
    }
    return value;
}

void DocumentsDialog::updateErrorState(const QString& key, bool ok, const QString& message) {
    if (!fields_.contains(key)) return;
    auto& fw = fields_[key];
    if (fw.editor) {
        fw.editor->setStyleSheet(ok ? "" : "border: 1px solid #b00020;" );
    }
    if (fw.errorLabel) {
        fw.errorLabel->setText(ok ? "" : message);
    }
}

void DocumentsDialog::updateDocumentStatus(Document* document) {
    if (!document) return;
    if (!DocumentService::isMinimumFilled(*document)) {
        document->setStatus(DocumentStatus::Absent);
    } else if (document->getStatus() != DocumentStatus::Verified) {
        document->setStatus(DocumentStatus::Available);
    }
    const int row = docList_->currentRow();
    if (row >= 0) {
        docList_->item(row)->setText(document->displayName() + " — " + Document::statusName(document->getStatus()));
    }
}

void DocumentsDialog::rebuildForm(Document* document) {
    QLayoutItem* child;
    while ((child = formLayout_->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    fields_.clear();

    if (!document) {
        headerError_->clear();
        return;
    }

    const auto fields = DocumentService::fieldsForType(document->getType());
    for (const auto& def : fields) {
        auto* editorContainer = new QWidget(this);
        auto* editorLayout = new QVBoxLayout(editorContainer);
        editorLayout->setContentsMargins(0, 0, 0, 0);

        QWidget* editor = nullptr;
        if (def.key.contains("Date")) {
            auto* dateEdit = new QDateEdit(this);
            dateEdit->setCalendarPopup(true);
            const QString existing = document->fields().value(def.key).toString();
            if (!existing.isEmpty())
                dateEdit->setDate(QDate::fromString(existing, Qt::ISODate));
            editor = dateEdit;
            connect(dateEdit, &QDateEdit::dateChanged, this, [this, document, def](const QDate& date) {
                document->fields()[def.key] = date.toString(Qt::ISODate);
                updateDocumentStatus(document);
                updateErrorState(def.key, true, "");
            });
        } else {
            auto* line = new QLineEdit(this);
            if (!def.inputMask.isEmpty()) {
                line->setInputMask(def.inputMask);
            } else if (def.regex.isValid() && !def.regex.pattern().isEmpty()) {
                line->setValidator(new QRegularExpressionValidator(def.regex, line));
            }
            line->setPlaceholderText(def.placeholder);
            const QString existing = document->fields().value(def.key).toString();
            line->setText(existing);
            editor = line;
            connect(line, &QLineEdit::textChanged, this, [this, document, def, line](const QString& text) {
                const bool hasMask = !def.inputMask.isEmpty();
                QString normalized = hasMask ? text : normalizeInput(text, def);
                if (hasMask && def.regex.pattern() == "^\\d{16}$") {
                    normalized.remove(' ');
                }
                if (!hasMask && normalized != text) {
                    QSignalBlocker blocker(line);
                    line->setText(normalized);
                }
                document->fields()[def.key] = normalized;
                bool ok = true;
                QString error;
                const QString valueForCheck = normalized;
                if (def.required && valueForCheck.trimmed().isEmpty()) {
                    ok = false;
                    error = "Обязательное поле";
                } else if (!valueForCheck.trimmed().isEmpty() && def.regex.isValid()
                           && !def.regex.pattern().isEmpty()
                           && !def.regex.match(valueForCheck).hasMatch()) {
                    ok = false;
                    error = "Неверный формат";
                }
                updateErrorState(def.key, ok, error);
                updateDocumentStatus(document);
            });
        }

        auto* errorLabel = new QLabel(this);
        errorLabel->setStyleSheet("color: #b00020;");

        editorLayout->addWidget(editor);
        editorLayout->addWidget(errorLabel);
        formLayout_->addRow(def.required ? def.label + " *" : def.label, editorContainer);

        fields_.insert(def.key, {editor, errorLabel, def});
    }

    headerError_->clear();
}

bool DocumentsDialog::isRequiredDocument(DocumentType type) const {
    if (!request_) return false;
    Tourist* tourist = currentTourist();
    if (tourist) {
        const auto required = DocumentService::requiredPersonalDocuments(*request_, *tourist);
        return std::find(required.begin(), required.end(), type) != required.end();
    }
    const auto required = DocumentService::requiredRequestDocuments(*request_);
    return std::find(required.begin(), required.end(), type) != required.end();
}

void DocumentsDialog::onAddDocument() {
    if (!request_) return;
    QStringList types;
    for (int i = 0; i <= (int)DocumentType::VeterinaryPassport; ++i) {
        types << Document::typeName(static_cast<DocumentType>(i));
    }
    bool ok = false;
    const QString typeName = QInputDialog::getItem(this, "Добавить документ", "Тип документа:",
                                                   types, 0, false, &ok);
    if (!ok || typeName.isEmpty()) return;

    DocumentType chosen = DocumentType::Passport;
    for (int i = 0; i <= (int)DocumentType::VeterinaryPassport; ++i) {
        if (Document::typeName(static_cast<DocumentType>(i)) == typeName) {
            chosen = static_cast<DocumentType>(i);
            break;
        }
    }

    auto doc = std::make_unique<Document>(chosen);
    Tourist* tourist = currentTourist();
    if (tourist) {
        for (const auto& existing : tourist->documents()) {
            if (existing->getType() == chosen) {
                QMessageBox::information(this, "Документы", "Этот документ уже добавлен.");
                return;
            }
        }
        tourist->documents().push_back(std::move(doc));
    } else {
        for (const auto& existing : request_->documents()) {
            if (existing->getType() == chosen) {
                QMessageBox::information(this, "Документы", "Этот документ уже добавлен.");
                return;
            }
        }
        request_->documents().push_back(std::move(doc));
    }
    refreshDocumentList();
}

void DocumentsDialog::onRemoveDocument() {
    Document* doc = currentDocument();
    if (!doc) return;
    if (isRequiredDocument(doc->getType())) {
        QMessageBox::information(this, "Удаление", "Обязательный документ удалить нельзя.");
        return;
    }
    const int row = docList_->currentRow();
    Tourist* tourist = currentTourist();
    if (tourist) {
        if (row >= 0 && row < (int)tourist->documents().size())
            tourist->documents().erase(tourist->documents().begin() + row);
    } else if (request_) {
        if (row >= 0 && row < (int)request_->documents().size())
            request_->documents().erase(request_->documents().begin() + row);
    }
    refreshDocumentList();
}

void DocumentsDialog::onVerifyDocument() {
    Document* doc = currentDocument();
    if (!doc) return;
    QString err;
    if (!doc->validate(&err)) {
        headerError_->setText("Ошибка проверки: " + err);
        return;
    }
    doc->setStatus(DocumentStatus::Verified);
    headerError_->setText("Документ отмечен как проверенный");
    refreshDocumentList();
}
