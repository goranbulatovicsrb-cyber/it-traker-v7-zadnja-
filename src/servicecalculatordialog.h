#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>

class ServiceCalculatorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServiceCalculatorDialog(QWidget* parent = nullptr);

private slots:
    void addPart();
    void removePart();
    void recalculate();
    void copyToClipboard();

private:
    void buildUi();

    QTableWidget*   m_table      = nullptr;
    QDoubleSpinBox* m_spnLabor   = nullptr;
    QDoubleSpinBox* m_spnMargin  = nullptr;
    QDoubleSpinBox* m_spnDiscount= nullptr;
    QLabel*         m_lblParts   = nullptr;
    QLabel*         m_lblLabor   = nullptr;
    QLabel*         m_lblTotal   = nullptr;
    QLabel*         m_lblClient  = nullptr;
    QLineEdit*      m_edtName    = nullptr;
};
