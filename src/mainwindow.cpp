#include "mainwindow.h"
#include "dashboard.h"
#include "clientspage.h"
#include "worklogpage.h"
#include "todopage.h"
#include "usermanagerdialog.h"
#include "loginscreen.h"
#include "jobdialog.h"
#include "servicecalculatordialog.h"
#include "session.h"
#include "style.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QApplication>
#include <QStatusBar>
#include <QShortcut>
#include <QKeySequence>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("IT Tracker — " + Session::instance().name());
    setMinimumSize(1100, 680);
    resize(1280, 760);
    buildUi();
    setupShortcuts();
    switchPage(0);
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* rootLay = new QHBoxLayout(central);
    rootLay->setContentsMargins(0,0,0,0);
    rootLay->setSpacing(0);

    // ── Sidebar ──────────────────────────────────────────────────────────────
    auto* sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(210);

    auto* sidebarLay = new QVBoxLayout(sidebar);
    sidebarLay->setContentsMargins(0,0,0,0);
    sidebarLay->setSpacing(0);

    // Logo
    auto* logo = new QWidget;
    logo->setFixedHeight(64);
    logo->setStyleSheet("background:#0d1117; border-bottom:1px solid #21262d;");
    auto* logoLay = new QHBoxLayout(logo);
    logoLay->setContentsMargins(20,0,20,0);
    auto* logoIcon = new QLabel("💻");
    logoIcon->setStyleSheet("font-size:22px;");
    auto* logoText = new QLabel("IT Tracker");
    logoText->setStyleSheet("font-size:16px; font-weight:700; color:#f0f6fc;");
    logoLay->addWidget(logoIcon);
    logoLay->addWidget(logoText);
    logoLay->addStretch();
    sidebarLay->addWidget(logo);

    // User badge
    auto* userBadge = new QFrame;
    userBadge->setStyleSheet("background:#161b22; border-bottom:1px solid #21262d;");
    auto* ubLay = new QHBoxLayout(userBadge);
    ubLay->setContentsMargins(14,10,14,10);
    ubLay->setSpacing(8);
    auto* userIcon = new QLabel("👤");
    userIcon->setStyleSheet("font-size:14px;");
    m_lblUser = new QLabel(Session::instance().name());
    m_lblUser->setStyleSheet("font-size:12px; font-weight:600; color:#f0f6fc;");
    m_lblUser->setWordWrap(true);
    ubLay->addWidget(userIcon);
    ubLay->addWidget(m_lblUser, 1);
    if (Session::instance().isAdmin()) {
        auto* adminBadge = new QLabel("👑");
        adminBadge->setStyleSheet("font-size:12px;");
        adminBadge->setToolTip("Administrator");
        ubLay->addWidget(adminBadge);
    }
    sidebarLay->addWidget(userBadge);

    // Quick add button (Ctrl+N)
    auto* btnQuickAdd = new QPushButton("⚡  Brzi unos  (Ctrl+N)");
    btnQuickAdd->setObjectName("btnPrimary");
    btnQuickAdd->setFixedHeight(36);
    btnQuickAdd->setCursor(Qt::PointingHandCursor);
    btnQuickAdd->setContentsMargins(20,0,20,0);
    btnQuickAdd->setStyleSheet(R"(
        QPushButton {
            background:#1f6feb; color:#fff; border:none;
            border-radius:0; font-size:12px; font-weight:600;
            padding: 0 20px;
        }
        QPushButton:hover { background:#388bfd; }
    )");
    sidebarLay->addWidget(btnQuickAdd);

    // Nav label
    auto* navLabel = new QLabel("NAVIGACIJA");
    navLabel->setContentsMargins(20,14,0,6);
    navLabel->setStyleSheet("font-size:10px; font-weight:700; color:#484f58; letter-spacing:1.5px;");
    sidebarLay->addWidget(navLabel);

    // Nav buttons
    struct NavItem { QString icon; QString label; };
    const QList<NavItem> items = {
        {"📊", "Dashboard"},
        {"📋", "Evidencija poslova"},
        {"👤", "Mušterije"},
        {"📝", "Bilješke & Todo"},
    };
    for (int i = 0; i < items.size(); i++) {
        auto* btn = new QPushButton(items[i].icon + "  " + items[i].label);
        btn->setObjectName("sidebarBtn");
        btn->setCheckable(true);
        btn->setFixedHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        sidebarLay->addWidget(btn);
        m_navBtns.append(btn);
        connect(btn, &QPushButton::clicked, this, [this, i]{ switchPage(i); });
    }

    sidebarLay->addStretch();

    // Tools separator
    auto* toolsLabel = new QLabel("ALATI");
    toolsLabel->setContentsMargins(20,8,0,4);
    toolsLabel->setStyleSheet("font-size:10px; font-weight:700; color:#484f58; letter-spacing:1.5px;");
    sidebarLay->addWidget(toolsLabel);

    // Service calculator
    auto* btnCalc = new QPushButton("🔧  Kalkulator usluga");
    btnCalc->setObjectName("sidebarBtn");
    btnCalc->setFixedHeight(40);
    btnCalc->setCursor(Qt::PointingHandCursor);
    sidebarLay->addWidget(btnCalc);
    connect(btnCalc, &QPushButton::clicked, this, &MainWindow::onServiceCalculator);

    // Admin: User manager
    if (Session::instance().isAdmin()) {
        auto* btnUsers = new QPushButton("👥  Korisnici");
        btnUsers->setObjectName("sidebarBtn");
        btnUsers->setFixedHeight(40);
        btnUsers->setCursor(Qt::PointingHandCursor);
        sidebarLay->addWidget(btnUsers);
        connect(btnUsers, &QPushButton::clicked, this, &MainWindow::onUserManager);
    }

    // Separator
    auto* sep2 = new QFrame;
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background:#21262d;");
    sep2->setFixedHeight(1);
    sidebarLay->addWidget(sep2);

    // Logout
    auto* btnLogout = new QPushButton("🚪  Odjava");
    btnLogout->setObjectName("sidebarBtn");
    btnLogout->setFixedHeight(40);
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet("QPushButton#sidebarBtn { color:#f85149; } "
                             "QPushButton#sidebarBtn:hover { background:#161b22; color:#f85149; }");
    sidebarLay->addWidget(btnLogout);

    auto* footer = new QLabel("v1.0  |  IT Tracker");
    footer->setAlignment(Qt::AlignCenter);
    footer->setContentsMargins(0,4,0,10);
    footer->setStyleSheet("font-size:10px; color:#484f58;");
    sidebarLay->addWidget(footer);

    rootLay->addWidget(sidebar);

    // Separator
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("background:#21262d;");
    sep->setFixedWidth(1);
    rootLay->addWidget(sep);

    // Main stack
    m_stack   = new QStackedWidget;
    m_dash    = new Dashboard(m_stack);
    m_worklog = new WorkLogPage(m_stack);
    m_clients = new ClientsPage(m_stack);
    m_todo    = new TodoPage(m_stack);

    m_stack->addWidget(m_dash);    // 0
    m_stack->addWidget(m_worklog); // 1
    m_stack->addWidget(m_clients); // 2
    m_stack->addWidget(m_todo);    // 3

    rootLay->addWidget(m_stack, 1);

    connect(m_worklog, &WorkLogPage::dataChanged, m_dash,    &Dashboard::refresh);
    connect(m_worklog, &WorkLogPage::dataChanged, m_clients, &ClientsPage::refresh);
    connect(btnQuickAdd, &QPushButton::clicked, this, &MainWindow::onQuickAdd);
    connect(btnLogout,   &QPushButton::clicked, this, &MainWindow::onLogout);

    statusBar()->setStyleSheet("background:#010409; color:#8b949e; border-top:1px solid #21262d;");
    statusBar()->showMessage("Prijavljen: " + Session::instance().name() +
                             "   |   Ctrl+N = Brzi unos   |   Ctrl+K = Kalkulator");
}

void MainWindow::setupShortcuts()
{
    auto* scNew  = new QShortcut(QKeySequence("Ctrl+N"), this);
    auto* scCalc = new QShortcut(QKeySequence("Ctrl+K"), this);
    auto* scDash = new QShortcut(QKeySequence("Ctrl+1"), this);
    auto* scWork = new QShortcut(QKeySequence("Ctrl+2"), this);
    auto* scCli  = new QShortcut(QKeySequence("Ctrl+3"), this);
    auto* scTodo = new QShortcut(QKeySequence("Ctrl+4"), this);

    connect(scNew,  &QShortcut::activated, this, &MainWindow::onQuickAdd);
    connect(scCalc, &QShortcut::activated, this, &MainWindow::onServiceCalculator);
    connect(scDash, &QShortcut::activated, this, [this]{ switchPage(0); });
    connect(scWork, &QShortcut::activated, this, [this]{ switchPage(1); });
    connect(scCli,  &QShortcut::activated, this, [this]{ switchPage(2); });
    connect(scTodo, &QShortcut::activated, this, [this]{ switchPage(3); });
}

void MainWindow::switchPage(int index)
{
    for (int i = 0; i < m_navBtns.size(); i++)
        m_navBtns[i]->setChecked(i == index);
    m_stack->setCurrentIndex(index);
    switch (index) {
        case 0: m_dash->refresh();    break;
        case 1: m_worklog->refresh(); break;
        case 2: m_clients->refresh(); break;
        case 3: m_todo->refresh();    break;
    }
}

void MainWindow::onQuickAdd()
{
    if (Database::instance().totalClients() == 0) {
        QMessageBox::information(this, "Info",
            "Prvo dodajte barem jednu mušteriju u sekciji 'Mušterije'!");
        return;
    }
    JobDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        WorkEntry w = dlg.result();
        Database::instance().addWork(w);
        m_worklog->refresh();
        m_dash->refresh();
        statusBar()->showMessage("✅ Posao dodan!", 3000);
    }
}

void MainWindow::onServiceCalculator()
{
    ServiceCalculatorDialog dlg(this);
    dlg.exec();
}

void MainWindow::onLogout()
{
    Session::instance().logout();
    close();
    LoginScreen* login = new LoginScreen;
    if (login->exec() == QDialog::Accepted) {
        MainWindow* w = new MainWindow;
        w->show();
    } else {
        qApp->quit();
    }
    login->deleteLater();
}

void MainWindow::onUserManager()
{
    UserManagerDialog dlg(this);
    dlg.exec();
}
