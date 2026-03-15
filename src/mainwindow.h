#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>

class Dashboard;
class ClientsPage;
class WorkLogPage;
class TodoPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onLogout();
    void onUserManager();
    void onQuickAdd();
    void onServiceCalculator();

private:
    void buildUi();
    void setupShortcuts();
    void switchPage(int index);

    QStackedWidget* m_stack   = nullptr;
    Dashboard*      m_dash    = nullptr;
    ClientsPage*    m_clients = nullptr;
    WorkLogPage*    m_worklog = nullptr;
    TodoPage*       m_todo    = nullptr;

    QLabel*             m_lblUser = nullptr;
    QList<QPushButton*> m_navBtns;
};
