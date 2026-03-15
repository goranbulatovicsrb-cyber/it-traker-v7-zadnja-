#include "todopage.h"
#include "session.h"
#include "database.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QDate>

// ─── TodoPage ─────────────────────────────────────────────────────────────────

TodoPage::TodoPage(QWidget* parent) : QWidget(parent)
{
    buildUi();
    refresh();
}

void TodoPage::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28,24,28,24);
    root->setSpacing(16);

    // Header
    auto* hdrRow = new QHBoxLayout;
    auto* title = new QLabel("📝  Bilješke & Todo");
    title->setObjectName("titleLabel");
    hdrRow->addWidget(title);
    hdrRow->addStretch();
    m_lblCount = new QLabel;
    m_lblCount->setStyleSheet("color:#8b949e;");
    hdrRow->addWidget(m_lblCount);
    root->addLayout(hdrRow);

    // Input row
    auto* inputCard = new QFrame;
    inputCard->setObjectName("card");
    auto* inputLay = new QHBoxLayout(inputCard);
    inputLay->setContentsMargins(14,10,14,10);
    inputLay->setSpacing(10);

    m_edtNew = new QLineEdit;
    m_edtNew->setPlaceholderText("✏️  Nova bilješka ili zadatak...");
    m_edtNew->setFixedHeight(38);

    m_cmbPriority = new QComboBox;
    m_cmbPriority->setFixedWidth(110);
    m_cmbPriority->addItem("🔴  Visok",   "Visok");
    m_cmbPriority->addItem("🟡  Srednji", "Srednji");
    m_cmbPriority->addItem("🟢  Nizak",   "Nizak");
    m_cmbPriority->setCurrentIndex(1);

    auto* btnAdd = new QPushButton("➕  Dodaj");
    btnAdd->setObjectName("btnPrimary");
    btnAdd->setFixedHeight(38);

    inputLay->addWidget(m_edtNew, 1);
    inputLay->addWidget(m_cmbPriority);
    inputLay->addWidget(btnAdd);
    root->addWidget(inputCard);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    auto* lblFilter = new QLabel("Prikaži:");
    lblFilter->setStyleSheet("color:#8b949e; font-weight:600;");
    m_cmbFilter = new QComboBox;
    m_cmbFilter->setFixedWidth(160);
    m_cmbFilter->addItem("Sve",         "all");
    m_cmbFilter->addItem("Aktivne",     "active");
    m_cmbFilter->addItem("Završene",    "done");
    m_cmbFilter->addItem("🔴 Visok",    "Visok");
    m_cmbFilter->addItem("🟡 Srednji",  "Srednji");
    m_cmbFilter->addItem("🟢 Nizak",    "Nizak");
    m_cmbFilter->setCurrentIndex(1); // default: active

    m_btnDel = new QPushButton("🗑  Obriši");
    m_btnDel->setObjectName("btnDanger");
    m_btnDel->setEnabled(false);

    auto* btnClearDone = new QPushButton("✅  Obriši završene");
    btnClearDone->setStyleSheet("color:#8b949e;");

    filterRow->addWidget(lblFilter);
    filterRow->addWidget(m_cmbFilter);
    filterRow->addStretch();
    filterRow->addWidget(btnClearDone);
    filterRow->addWidget(m_btnDel);
    root->addLayout(filterRow);

    // List
    m_list = new QListWidget;
    m_list->setStyleSheet(R"(
        QListWidget {
            background:#0d1117; border:1px solid #30363d; border-radius:6px;
            outline:none;
        }
        QListWidget::item {
            padding:12px 14px; border-bottom:1px solid #21262d;
            color:#f0f6fc; font-size:13px;
        }
        QListWidget::item:selected { background:#1f6feb22; }
        QListWidget::item:hover    { background:#161b22; }
    )");
    m_list->setSpacing(2);
    root->addWidget(m_list, 1);

    // Connections
    connect(btnAdd,       &QPushButton::clicked, this, &TodoPage::onAdd);
    connect(m_btnDel,     &QPushButton::clicked, this, &TodoPage::onDelete);
    connect(btnClearDone, &QPushButton::clicked, this, [this]{
        Database::instance().clearDoneTodos();
        refresh();
    });
    connect(m_edtNew,   &QLineEdit::returnPressed, this, &TodoPage::onAdd);
    connect(m_cmbFilter,qOverload<int>(&QComboBox::currentIndexChanged), this, &TodoPage::onFilterChanged);
    connect(m_list, &QListWidget::itemChanged,    this, &TodoPage::onToggleDone);
    connect(m_list, &QListWidget::itemSelectionChanged, this, [this]{
        m_btnDel->setEnabled(m_list->currentRow() >= 0);
    });
}

void TodoPage::refresh()
{
    m_all = Database::instance().getAllTodos();
    populateList();
}

void TodoPage::populateList()
{
    m_list->blockSignals(true);
    m_list->clear();

    QString filter = m_cmbFilter->currentData().toString();

    int shown = 0;
    for (auto& t : m_all) {
        if (filter == "active" && t.done)   continue;
        if (filter == "done"   && !t.done)  continue;
        if (filter == "Visok"  && t.priority != "Visok")   continue;
        if (filter == "Srednji"&& t.priority != "Srednji") continue;
        if (filter == "Nizak"  && t.priority != "Nizak")   continue;

        auto* item = new QListWidgetItem;
        item->setData(Qt::UserRole, t.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(t.done ? Qt::Checked : Qt::Unchecked);

        // Priority icon
        QString pIcon = (t.priority=="Visok") ? "🔴" : (t.priority=="Srednji") ? "🟡" : "🟢";
        item->setText(QString("%1  %2").arg(pIcon).arg(t.text));

        // Style done items
        if (t.done) {
            item->setForeground(QColor("#484f58"));
            QFont f = item->font();
            f.setStrikeOut(true);
            item->setFont(f);
        }

        m_list->addItem(item);
        shown++;
    }

    m_list->blockSignals(false);

    int total  = m_all.size();
    int active = 0;
    for (auto& t : m_all) if (!t.done) active++;
    m_lblCount->setText(QString("%1 aktivan / %2 ukupno").arg(active).arg(total));
    m_btnDel->setEnabled(false);
}

void TodoPage::onAdd()
{
    QString text = m_edtNew->text().trimmed();
    if (text.isEmpty()) return;

    TodoItem t;
    t.userId   = Session::instance().userId();
    t.text     = text;
    t.priority = m_cmbPriority->currentData().toString();
    t.done     = false;
    t.created  = QDate::currentDate().toString("yyyy-MM-dd");

    Database::instance().addTodo(t);
    m_edtNew->clear();
    refresh();
}

void TodoPage::onDelete()
{
    auto* item = m_list->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    Database::instance().deleteTodo(id);
    refresh();
}

void TodoPage::onToggleDone(QListWidgetItem* item)
{
    int id   = item->data(Qt::UserRole).toInt();
    bool done = (item->checkState() == Qt::Checked);
    Database::instance().setTodoDone(id, done);
    refresh();
}

void TodoPage::onFilterChanged()
{
    populateList();
}
