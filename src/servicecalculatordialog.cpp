#include "servicecalculatordialog.h"
#include "style.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QFrame>

ServiceCalculatorDialog::ServiceCalculatorDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("🔧  Kalkulator usluga");
    setModal(true);
    setMinimumSize(620, 560);
    buildUi();
}

void ServiceCalculatorDialog::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24,20,24,20);
    root->setSpacing(16);

    // Title
    auto* title = new QLabel("🔧  Kalkulator cijene usluge");
    title->setObjectName("titleLabel");
    title->setStyleSheet("font-size:18px; font-weight:700; color:#f0f6fc;");
    root->addWidget(title);

    auto* sub = new QLabel("Dodaj dijelove/komponente + rad → program izračuna ukupnu cijenu za mušteriju");
    sub->setStyleSheet("color:#8b949e; font-size:12px;");
    root->addWidget(sub);

    // Service name
    auto* nameRow = new QHBoxLayout;
    auto* lblName = new QLabel("Naziv usluge:");
    lblName->setStyleSheet("color:#8b949e; font-weight:600;");
    m_edtName = new QLineEdit;
    m_edtName->setPlaceholderText("npr. Popravka laptopa...");
    nameRow->addWidget(lblName);
    nameRow->addWidget(m_edtName, 1);
    root->addLayout(nameRow);

    // Parts table
    auto* partsLabel = new QLabel("📦  Dijelovi / Komponente:");
    partsLabel->setStyleSheet("font-weight:600; color:#f0f6fc;");
    root->addWidget(partsLabel);

    m_table = new QTableWidget(0, 4);
    m_table->setHorizontalHeaderLabels({"Naziv dijela", "Nabavna cijena €", "Marža %", "Cijena za mušteriju €"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_table->setColumnWidth(1, 130);
    m_table->setColumnWidth(2, 80);
    m_table->setColumnWidth(3, 160);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setMinimumHeight(160);
    m_table->setStyleSheet("QTableWidget { color:#f0f6fc; } QTableWidget::item { padding:6px; }");
    root->addWidget(m_table);

    // Add/remove row buttons
    auto* tableBtn = new QHBoxLayout;
    auto* btnAddRow = new QPushButton("➕  Dodaj dio");
    btnAddRow->setObjectName("btnPrimary");
    auto* btnRemRow = new QPushButton("➖  Ukloni");
    tableBtn->addWidget(btnAddRow);
    tableBtn->addWidget(btnRemRow);
    tableBtn->addStretch();
    root->addLayout(tableBtn);

    // Labor + settings row
    auto* settingsRow = new QHBoxLayout;
    settingsRow->setSpacing(20);

    // Labor
    auto* laborGroup = new QGroupBox("👨‍🔧  Rad / Usluga");
    laborGroup->setStyleSheet("QGroupBox { color:#8b949e; border:1px solid #30363d; border-radius:6px; margin-top:12px; padding-top:8px; } QGroupBox::title { subcontrol-origin:margin; padding:0 6px; }");
    auto* laborLay = new QFormLayout(laborGroup);
    m_spnLabor = new QDoubleSpinBox;
    m_spnLabor->setRange(0, 9999); m_spnLabor->setSuffix(" €"); m_spnLabor->setValue(20);
    laborLay->addRow("Cijena rada:", m_spnLabor);
    settingsRow->addWidget(laborGroup);

    // Global margin
    auto* marginGroup = new QGroupBox("📈  Globalna marža");
    marginGroup->setStyleSheet(laborGroup->styleSheet());
    auto* marginLay = new QFormLayout(marginGroup);
    m_spnMargin = new QDoubleSpinBox;
    m_spnMargin->setRange(0, 200); m_spnMargin->setSuffix(" %"); m_spnMargin->setValue(20);
    marginLay->addRow("Marža na dijelove:", m_spnMargin);
    settingsRow->addWidget(marginGroup);

    // Discount
    auto* discGroup = new QGroupBox("🎁  Popust");
    discGroup->setStyleSheet(laborGroup->styleSheet());
    auto* discLay = new QFormLayout(discGroup);
    m_spnDiscount = new QDoubleSpinBox;
    m_spnDiscount->setRange(0, 100); m_spnDiscount->setSuffix(" %"); m_spnDiscount->setValue(0);
    discLay->addRow("Popust:", m_spnDiscount);
    settingsRow->addWidget(discGroup);

    root->addLayout(settingsRow);

    // Summary
    auto* summaryCard = new QFrame;
    summaryCard->setObjectName("card");
    summaryCard->setStyleSheet("#card { background:#161b22; border:1px solid #30363d; border-radius:6px; }");
    auto* sumLay = new QHBoxLayout(summaryCard);
    sumLay->setContentsMargins(16,12,16,12);

    auto mkSumItem = [](const QString& label, QLabel*& valLbl, const QString& color) -> QWidget* {
        auto* w = new QWidget;
        auto* l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0); l->setSpacing(2);
        auto* lbl = new QLabel(label);
        lbl->setStyleSheet("color:#8b949e; font-size:11px; font-weight:600;");
        valLbl = new QLabel("0.00 €");
        valLbl->setStyleSheet(QString("font-size:18px; font-weight:700; color:%1;").arg(color));
        l->addWidget(lbl); l->addWidget(valLbl);
        return w;
    };

    sumLay->addWidget(mkSumItem("Dijelovi",  m_lblParts, Style::BLUE));
    auto* sep1 = new QLabel("+"); sep1->setStyleSheet("font-size:20px; color:#484f58;");
    sumLay->addWidget(sep1);
    sumLay->addWidget(mkSumItem("Rad",       m_lblLabor, Style::PURPLE));
    auto* sep2 = new QLabel("="); sep2->setStyleSheet("font-size:20px; color:#484f58;");
    sumLay->addWidget(sep2);
    sumLay->addWidget(mkSumItem("UKUPNO ZA MUŠTERIJU", m_lblTotal, Style::GREEN));
    sumLay->addStretch();

    auto* btnCopy = new QPushButton("📋  Kopiraj");
    btnCopy->setObjectName("btnPrimary");
    sumLay->addWidget(btnCopy);

    root->addWidget(summaryCard);

    // Buttons
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnClose = new QPushButton("Zatvori"); btnClose->setFixedWidth(100);
    btnRow->addWidget(btnClose);
    root->addLayout(btnRow);

    // Connections
    connect(btnAddRow,   &QPushButton::clicked, this, &ServiceCalculatorDialog::addPart);
    connect(btnRemRow,   &QPushButton::clicked, this, &ServiceCalculatorDialog::removePart);
    connect(m_table,     &QTableWidget::cellChanged, this, &ServiceCalculatorDialog::recalculate);
    connect(m_spnLabor,  qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ServiceCalculatorDialog::recalculate);
    connect(m_spnMargin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ServiceCalculatorDialog::recalculate);
    connect(m_spnDiscount,qOverload<double>(&QDoubleSpinBox::valueChanged),this, &ServiceCalculatorDialog::recalculate);
    connect(btnCopy,     &QPushButton::clicked, this, &ServiceCalculatorDialog::copyToClipboard);
    connect(btnClose,    &QPushButton::clicked, this, &QDialog::accept);

    // Add one empty row to start
    addPart();
}

void ServiceCalculatorDialog::addPart()
{
    int row = m_table->rowCount();
    m_table->insertRow(row);

    auto* namItem = new QTableWidgetItem("Naziv dijela");
    namItem->setForeground(QColor("#f0f6fc"));
    m_table->setItem(row, 0, namItem);

    // Cost spin
    auto* costSpin = new QDoubleSpinBox;
    costSpin->setRange(0, 99999); costSpin->setSuffix(" €"); costSpin->setValue(0);
    costSpin->setStyleSheet("background:#0d1117; color:#f0f6fc; border:none; padding:4px;");
    m_table->setCellWidget(row, 1, costSpin);

    // Margin spin (inherits global margin)
    auto* marginSpin = new QDoubleSpinBox;
    marginSpin->setRange(0, 200); marginSpin->setSuffix(" %"); marginSpin->setValue(m_spnMargin->value());
    marginSpin->setStyleSheet("background:#0d1117; color:#f0f6fc; border:none; padding:4px;");
    m_table->setCellWidget(row, 2, marginSpin);

    // Price (read-only)
    auto* priceItem = new QTableWidgetItem("0.00 €");
    priceItem->setForeground(QColor(Style::GREEN));
    priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont f; f.setBold(true); priceItem->setFont(f);
    priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
    m_table->setItem(row, 3, priceItem);

    connect(costSpin,   qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ServiceCalculatorDialog::recalculate);
    connect(marginSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ServiceCalculatorDialog::recalculate);
}

void ServiceCalculatorDialog::removePart()
{
    int row = m_table->currentRow();
    if (row >= 0) { m_table->removeRow(row); recalculate(); }
}

void ServiceCalculatorDialog::recalculate()
{
    double partsTotal = 0.0;

    for (int r = 0; r < m_table->rowCount(); r++) {
        auto* costW   = qobject_cast<QDoubleSpinBox*>(m_table->cellWidget(r, 1));
        auto* marginW = qobject_cast<QDoubleSpinBox*>(m_table->cellWidget(r, 2));
        if (!costW || !marginW) continue;

        double cost     = costW->value();
        double margin   = marginW->value();
        double price    = cost * (1.0 + margin / 100.0);
        partsTotal     += price;

        if (auto* item = m_table->item(r, 3))
            item->setText(QString("%1 €").arg(price, 0, 'f', 2));
    }

    double labor    = m_spnLabor->value();
    double subtotal = partsTotal + labor;
    double discount = subtotal * (m_spnDiscount->value() / 100.0);
    double total    = subtotal - discount;

    m_lblParts->setText(QString("%1 €").arg(partsTotal, 0, 'f', 2));
    m_lblLabor->setText(QString("%1 €").arg(labor,      0, 'f', 2));
    m_lblTotal->setText(QString("%1 €").arg(total,      0, 'f', 2));
}

void ServiceCalculatorDialog::copyToClipboard()
{
    QString text;
    QString name = m_edtName->text().trimmed();
    if (!name.isEmpty()) text += name + "\n" + QString("-").repeated(40) + "\n";

    text += "DIJELOVI:\n";
    for (int r = 0; r < m_table->rowCount(); r++) {
        auto* nameItem = m_table->item(r, 0);
        auto* priceItem = m_table->item(r, 3);
        if (nameItem && priceItem)
            text += QString("  %1: %2\n").arg(nameItem->text()).arg(priceItem->text());
    }
    text += QString("\nRad: %1 €\n").arg(m_spnLabor->value(), 0, 'f', 2);
    if (m_spnDiscount->value() > 0)
        text += QString("Popust: -%1%\n").arg(m_spnDiscount->value(), 0, 'f', 0);
    text += QString("\nUKUPNO: %1").arg(m_lblTotal->text());

    QApplication::clipboard()->setText(text);
    QMessageBox::information(this, "✅ Kopirano", "Kalkulacija je kopirana u clipboard!\nMožeš je zalijepiti u poruku ili email.");
}
