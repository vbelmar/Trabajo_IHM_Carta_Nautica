#include "historywindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>

HistoryWindow::HistoryWindow(User *user, QWidget *parent)
    : QWidget(parent)
    , m_user(user)
{
    setupUI();
    loadSessionData();
}

HistoryWindow::~HistoryWindow()
{
}

void HistoryWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: #F8F9FA;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);

    // --- Contenedor central para centrar elementos ---
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addStretch(); // Espacio izquierdo

    // Widget contenedor para los elementos centrados
    QWidget *centerWidget = new QWidget(contentWidget);
    QVBoxLayout *centerContentLayout = new QVBoxLayout(centerWidget);
    centerContentLayout->setContentsMargins(0, 0, 0, 0);
    centerContentLayout->setSpacing(20);
    centerWidget->setMaximumWidth(1200); // Ajusta este ancho según necesites

    // --- 1. Filter Section (Top) ---
    QWidget *filterBar = new QWidget(centerWidget);
    filterBar->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border: 1px solid #CCCCCC;"
        "   border-radius: 8px;"
        "}"
        );
    QHBoxLayout *filterLayout = new QHBoxLayout(filterBar);
    filterLayout->setContentsMargins(15, 15, 15, 15);
    filterLayout->setSpacing(15);

    QLabel *filterLabel = new QLabel("Filtrar desde:", filterBar);
    filterLabel->setStyleSheet("color: #2C3E50; font-size: 16px; font-weight: 600; border: none;");
    filterLayout->addWidget(filterLabel);

    m_filterDateEdit = new QDateEdit(filterBar);
    m_filterDateEdit->setCalendarPopup(true);
    m_filterDateEdit->setDate(QDate::currentDate().addMonths(-1));
    m_filterDateEdit->setMaximumDate(QDate::currentDate());
    m_filterDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_filterDateEdit->setStyleSheet(
        "QDateEdit {"
        "   background-color: white;"
        "   color: #2C3E50;"
        "   border: 2px solid #BDC3C7;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "   font-size: 16px;"
        "   min-width: 150px;"
        "}"
        "QDateEdit:focus {"
        "   border-color: #3498DB;"
        "}"
        "QDateEdit::drop-down {"
        "   border: none;"
        "   color: #2C3E50;"
        "}"
        );
    filterLayout->addWidget(m_filterDateEdit);

    m_clearFilterButton = new QPushButton("Mostrar Todo", filterBar);
    m_clearFilterButton->setObjectName("headerButton");
    filterLayout->addWidget(m_clearFilterButton);

    filterLayout->addStretch();

    centerContentLayout->addWidget(filterBar);

    m_sessionsTable = new QTableWidget(centerWidget);
    m_sessionsTable->setObjectName("problemTable");
    m_sessionsTable->setColumnCount(5);
    m_sessionsTable->setHorizontalHeaderLabels({
        "Fecha", "Hora", "Aciertos", "Fallos", "Total"
    });

    m_sessionsTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   color: #2C3E50;"
        "   font-size: 16px;"
        "   gridline-color: #ECF0F1;"
        "   border: 1px solid #BDC3C7;"
        "   border-radius: 4px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #ECF0F1;"
        "   color: #2C3E50;"
        "   padding: 8px;"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   border: none;"
        "}"
        "QTableWidget::item {"
        "   padding: 5px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #3498DB;"
        "   color: white;"
        "}"
        );

    m_sessionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    m_sessionsTable->verticalHeader()->setVisible(false);
    m_sessionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sessionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sessionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sessionsTable->setAlternatingRowColors(true);
    m_sessionsTable->setSortingEnabled(true);

    centerContentLayout->addWidget(m_sessionsTable, 1);

    //Boton de retorno
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    bottomLayout->addStretch();

    QPushButton *backButton = new QPushButton("← Volver", centerWidget);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setMinimumWidth(150);
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498DB;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 10px 20px;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980B9;"
        "}"
        );
    connect(backButton, &QPushButton::clicked, this, &HistoryWindow::backToMap);

    bottomLayout->addWidget(backButton);

    bottomLayout->addStretch();

    centerContentLayout->addLayout(bottomLayout);

    // Agregar el widget central al layout horizontal con stretches
    centerLayout->addWidget(centerWidget);
    centerLayout->addStretch(); // Espacio derecho

    contentLayout->addLayout(centerLayout);

    mainLayout->addWidget(contentWidget, 1);

    connect(m_filterDateEdit, &QDateEdit::dateChanged, this, &HistoryWindow::onFilterDateChanged);
    connect(m_clearFilterButton, &QPushButton::clicked, this, &HistoryWindow::onClearFilter);
}

void HistoryWindow::loadSessionData()
{
    if (!m_user) return;

    QVector<Session> filteredSessions = getFilteredSessions();
    populateTable(filteredSessions);
}

void HistoryWindow::populateTable(const QVector<Session> &sessions)
{
    m_sessionsTable->setSortingEnabled(false);
    m_sessionsTable->setRowCount(0);

    if (sessions.isEmpty()) {
        m_sessionsTable->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem("📊 No hay sesiones registradas en este período");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(QColor(127, 140, 141)));
        QFont emptyFont = emptyItem->font();
        emptyFont.setPointSize(14);
        emptyItem->setFont(emptyFont);
        m_sessionsTable->setItem(0, 0, emptyItem);
        m_sessionsTable->setSpan(0, 0, 1, 5);
        return;
    }

    for (int i = 0; i < sessions.size(); ++i) {
        const Session &session = sessions[i];

        int row = m_sessionsTable->rowCount();
        m_sessionsTable->insertRow(row);

        // Fecha
        QTableWidgetItem *dateItem = new QTableWidgetItem(
            session.timeStamp().toString("dd/MM/yyyy")
            );
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_sessionsTable->setItem(row, 0, dateItem);

        // Hora
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            session.timeStamp().toString("HH:mm:ss")
            );
        timeItem->setTextAlignment(Qt::AlignCenter);
        m_sessionsTable->setItem(row, 1, timeItem);

        // Aciertos
        QTableWidgetItem *hitsItem = new QTableWidgetItem(
            QString::number(session.hits())
            );
        hitsItem->setTextAlignment(Qt::AlignCenter);
        hitsItem->setForeground(QBrush(QColor(39, 174, 96))); // #27AE60
        QFont hitsFont = hitsItem->font();
        hitsFont.setBold(true);
        hitsItem->setFont(hitsFont);
        m_sessionsTable->setItem(row, 2, hitsItem);

        // Fallos
        QTableWidgetItem *faultsItem = new QTableWidgetItem(
            QString::number(session.faults())
            );
        faultsItem->setTextAlignment(Qt::AlignCenter);
        faultsItem->setForeground(QBrush(QColor(231, 76, 60))); // #E74C3C
        QFont faultsFont = faultsItem->font();
        faultsFont.setBold(true);
        faultsItem->setFont(faultsFont);
        m_sessionsTable->setItem(row, 3, faultsItem);

        // Total
        int total = session.hits() + session.faults();
        QTableWidgetItem *totalItem = new QTableWidgetItem(
            QString::number(total)
            );
        totalItem->setTextAlignment(Qt::AlignCenter);
        QFont boldFont = totalItem->font();
        boldFont.setBold(true);
        totalItem->setFont(boldFont);
        m_sessionsTable->setItem(row, 4, totalItem);
    }

    m_sessionsTable->setSortingEnabled(true);
    m_sessionsTable->sortByColumn(0, Qt::DescendingOrder); // Más recientes primero
}

QVector<Session> HistoryWindow::getFilteredSessions()
{
    if (!m_user) return QVector<Session>();

    const QVector<Session> &allSessions = m_user->sessions();
    QVector<Session> filtered;

    QDate filterDate = m_filterDateEdit->date();

    for (const Session &session : allSessions) {
        if (session.timeStamp().date() >= filterDate) {
            filtered.append(session);
        }
    }

    return filtered;
}

void HistoryWindow::onFilterDateChanged()
{
    QVector<Session> filteredSessions = getFilteredSessions();
    populateTable(filteredSessions);
}

void HistoryWindow::onClearFilter()
{
    if (!m_user) return;

    // Resetear a mostrar todo
    const QVector<Session> &allSessions = m_user->sessions();
    if (!allSessions.isEmpty()) {
        QDate oldestDate = allSessions.first().timeStamp().date();
        for (const Session &session : allSessions) {
            if (session.timeStamp().date() < oldestDate) {
                oldestDate = session.timeStamp().date();
            }
        }
        m_filterDateEdit->setDate(oldestDate);
    }

    populateTable(m_user->sessions());
}

void HistoryWindow::refreshData()
{
    loadSessionData();
}
