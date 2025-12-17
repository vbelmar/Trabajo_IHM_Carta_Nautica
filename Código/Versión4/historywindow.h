#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QVBoxLayout>
#include "navtypes.h"

class HistoryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWindow(User *user, QWidget *parent = nullptr);
    ~HistoryWindow();

signals:
    void backToMap();

private slots:
    void onFilterDateChanged();
    void onClearFilter();

private:
    User *m_user;

    QDateEdit *m_filterDateEdit;
    QPushButton *m_clearFilterButton;
    QTableWidget *m_sessionsTable;

    void setupUI();
    void loadSessionData();
    void populateTable(const QVector<Session> &sessions);
    QVector<Session> getFilteredSessions();

public slots:
    void refreshData();
};

#endif // HISTORYWINDOW_H
