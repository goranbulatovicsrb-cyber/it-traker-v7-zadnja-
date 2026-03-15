#pragma once

#include "database.h"
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class TodoPage : public QWidget
{
    Q_OBJECT
public:
    explicit TodoPage(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onDelete();
    void onToggleDone(QListWidgetItem* item);
    void onFilterChanged();

private:
    void buildUi();
    void populateList();

    QList<TodoItem> m_all;
    QListWidget*    m_list        = nullptr;
    QLineEdit*      m_edtNew      = nullptr;
    QComboBox*      m_cmbPriority = nullptr;
    QComboBox*      m_cmbFilter   = nullptr;
    QPushButton*    m_btnDel      = nullptr;
    QLabel*         m_lblCount    = nullptr;
};
