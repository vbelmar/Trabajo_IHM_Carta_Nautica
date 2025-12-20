#ifndef QUESTIONMANAGER_H
#define QUESTIONMANAGER_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QFrame>
#include <QPropertyAnimation>
#include <vector>
#include "navigation.h"
#include "navtypes.h"

class QuestionManager : public QObject
{
    Q_OBJECT

public:
    explicit QuestionManager(QWidget *parent = nullptr);
    ~QuestionManager();

    QFrame* getSideMenuFrame() const { return m_sideMenuFrame; }
    QFrame* getMenuToggleColumn() const { return m_menuToggleColumn; }
    const std::vector<Problem>& getProblems() const {return m_problems; }

    void setupSideMenu(QWidget *parentWidget);
    void loadProblems(Navigation &nav);

    bool isMenuVisible() const { return m_menuVisible; }

public slots:
    void toggleSideMenu();
    void showQuestion(int questionIndex);
    void showProblemSelection();
    void showProblemFromTable(int row, int column);
    void onModeChanged();

signals:
    void problemAnswered(int problemIndex, bool correct);
    void menuVisibilityChanged(bool visible);

private:
    // Widgets principales
    QFrame *m_sideMenuFrame;
    QFrame *m_menuToggleColumn;
    QStackedWidget *m_questionStack;
    QPushButton *m_toggleMenuButton;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QLabel *m_questionIndicator;
    QPropertyAnimation *m_menuAnimation;

    // Página de selección de problemas
    QWidget *m_problemSelectionPage;
    QPushButton *m_randomModeButton;
    QPushButton *m_listModeButton;
    QTableWidget *m_problemTable;
    QPushButton *m_backButton;
    QDialog *m_problemListDialog;

    // Estado
    bool m_menuVisible;
    std::vector<Problem> m_problems;

    // Métodos privados de construcción
    void createQuestionPages();
    void createProblemSelectionPage();
    void updateNavigationButtons();
    void updateQuestionIndicator();
    void updateProblemTable();
    void showProblemListDialog();

    // Validación de respuestas
    void validateAnswer(size_t problemIndex, int selectedAnswerIndex);
};

#endif // QUESTIONMANAGER_H
