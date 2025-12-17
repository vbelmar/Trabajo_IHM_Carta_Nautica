#include "question_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QHeaderView>
#include <QRandomGenerator>
#include <QDebug>
#include <QScrollArea>


QuestionManager::QuestionManager(QWidget *parent)
    : QObject(parent)
    , m_sideMenuFrame(nullptr)
    , m_menuToggleColumn(nullptr)
    , m_questionStack(nullptr)
    , m_toggleMenuButton(nullptr)
    , m_prevButton(nullptr)
    , m_nextButton(nullptr)
    , m_questionIndicator(nullptr)
    , m_menuAnimation(nullptr)
    , m_problemSelectionPage(nullptr)
    , m_randomModeButton(nullptr)
    , m_listModeButton(nullptr)
    , m_problemTable(nullptr)
    , m_backButton(nullptr)
    , m_menuVisible(true)
{
}

QuestionManager::~QuestionManager()
{
    // Los widgets se destruyen automáticamente por Qt parent system
}

void QuestionManager::setupSideMenu(QWidget *parentWidget)
{
    // ==================== COLUMNA DE TOGGLE ====================
    m_menuToggleColumn = new QFrame(parentWidget);
    m_menuToggleColumn->setObjectName("menuToggleColumn");
    m_menuToggleColumn->setFixedWidth(40);
    m_menuToggleColumn->setVisible(false);

    QVBoxLayout *columnLayout = new QVBoxLayout(m_menuToggleColumn);
    columnLayout->setContentsMargins(5, 10, 5, 10);
    columnLayout->setSpacing(0);

    QPushButton *showMenuButton = new QPushButton("▶", m_menuToggleColumn);
    showMenuButton->setObjectName("toggleMenuButton");
    showMenuButton->setFixedSize(36, 36);
    connect(showMenuButton, &QPushButton::clicked, this, &QuestionManager::toggleSideMenu);

    columnLayout->addWidget(showMenuButton);
    columnLayout->addStretch();

    // ==================== FRAME PRINCIPAL DEL MENÚ ====================
    m_sideMenuFrame = new QFrame(parentWidget);
    m_sideMenuFrame->setObjectName("sideMenuFrame");
    m_sideMenuFrame->setFixedWidth(350);

    QVBoxLayout *menuLayout = new QVBoxLayout(m_sideMenuFrame);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);

    // ==================== HEADER DEL MENÚ ====================
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(10, 10, 10, 10);
    headerLayout->setSpacing(10);

    m_toggleMenuButton = new QPushButton("◀", m_sideMenuFrame);
    m_toggleMenuButton->setObjectName("toggleMenuButton");
    m_toggleMenuButton->setFixedSize(36, 36);
    headerLayout->addWidget(m_toggleMenuButton);

    headerLayout->addStretch();
    QLabel *titleLabel = new QLabel("Preguntas", m_sideMenuFrame);
    titleLabel->setObjectName("menuTitleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    QWidget *spacer = new QWidget(m_sideMenuFrame);
    spacer->setFixedSize(36, 36);
    headerLayout->addWidget(spacer);

    menuLayout->addLayout(headerLayout);

    // ==================== STACKED WIDGET PARA PREGUNTAS ====================
    m_questionStack = new QStackedWidget(m_sideMenuFrame);
    m_questionStack->setStyleSheet("QStackedWidget { background-color: white; }");
    menuLayout->addWidget(m_questionStack, 1);

    // ==================== BOTÓN NUEVO PROBLEMA ====================
    QHBoxLayout *newProblemLayout = new QHBoxLayout();
    newProblemLayout->setContentsMargins(10, 10, 10, 10);

    QPushButton *newProblemBtn = new QPushButton("NUEVO PROBLEMA", m_sideMenuFrame);
    newProblemBtn->setObjectName("newProblemButton");
    newProblemBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    connect(newProblemBtn, &QPushButton::clicked, this, &QuestionManager::showProblemSelection);

    newProblemLayout->addStretch();
    newProblemLayout->addWidget(newProblemBtn);
    newProblemLayout->addStretch();

    menuLayout->addLayout(newProblemLayout);

    // ==================== FOOTER CON NAVEGACIÓN ====================
    QWidget *footerWidget = new QWidget(m_sideMenuFrame);
    footerWidget->setObjectName("footerWidget");
    QVBoxLayout *footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(10, 10, 10, 10);
    footerLayout->setSpacing(8);

    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(10);

    m_prevButton = new QPushButton("◀", footerWidget);
    m_nextButton = new QPushButton("▶", footerWidget);
    m_prevButton->setObjectName("navButton");
    m_nextButton->setObjectName("navButton");
    m_prevButton->setFixedWidth(40);
    m_nextButton->setFixedWidth(40);

    m_questionIndicator = new QLabel("1 / 3", footerWidget);
    m_questionIndicator->setObjectName("questionIndicator");
    m_questionIndicator->setAlignment(Qt::AlignCenter);
    m_questionIndicator->setMinimumWidth(60);

    navLayout->addStretch();
    navLayout->addWidget(m_prevButton);
    navLayout->addWidget(m_questionIndicator);
    navLayout->addWidget(m_nextButton);
    navLayout->addStretch();

    footerLayout->addLayout(navLayout);
    menuLayout->addWidget(footerWidget);

    // ==================== CONEXIONES DE NAVEGACIÓN ====================
    connect(m_prevButton, &QPushButton::clicked, this, [this]() {
        int current = m_questionStack->currentIndex();
        int problemCount = static_cast<int>(m_problems.size());
        if (current > 0 && current < problemCount) {
            showQuestion(current - 1);
        }
    });

    connect(m_nextButton, &QPushButton::clicked, this, [this]() {
        int current = m_questionStack->currentIndex();
        int problemCount = static_cast<int>(m_problems.size());
        if (current < problemCount - 1) {
            showQuestion(current + 1);
        }
    });

    // ==================== ANIMACIÓN DEL MENÚ ====================
    m_menuAnimation = new QPropertyAnimation(m_sideMenuFrame, "maximumWidth", this);
    m_menuAnimation->setDuration(300);
    m_menuAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_toggleMenuButton, &QPushButton::clicked, this, &QuestionManager::toggleSideMenu);

    updateNavigationButtons();
    updateQuestionIndicator();
}

void QuestionManager::loadProblems(Navigation &nav)
{
    QVector<Problem> problemsVector = nav.problems();

    m_problems.clear();

    qDebug() << "=== DEBUG PROBLEMAS CARGADOS ===";
    for (const Problem &prob : problemsVector) {
        m_problems.push_back(prob);

        // DEBUG PRINT
        qDebug() << "Problema:" << prob.text();
        for(const Answer &ans : prob.answers()) {
            qDebug() << " - Respuesta:" << ans.text() << " | Es Correcta?:" << ans.validity();
        }
    }
    qDebug() << "================================";

    createQuestionPages();
    createProblemSelectionPage();
    updateQuestionIndicator();
}
void QuestionManager::createQuestionPages()
{
    // Limpiar páginas existentes
    while (m_questionStack->count() > 0) {
        QWidget *widget = m_questionStack->widget(0);
        m_questionStack->removeWidget(widget);
        delete widget;
    }

    if (m_problems.empty()) {
        qDebug() << "No hay problemas para mostrar";
        return;
    }

    // Crear una página por cada problema
    for (size_t i = 0; i < m_problems.size(); ++i) {
        const Problem &problem = m_problems[i];

        QWidget *questionPage = new QWidget();
        QVBoxLayout *pageLayout = new QVBoxLayout(questionPage);
        pageLayout->setContentsMargins(15, 15, 15, 15);
        pageLayout->setSpacing(15);

        // Número de pregunta
        QLabel *questionNumber = new QLabel(
            QString("Pregunta %1").arg(i + 1), questionPage);
        questionNumber->setObjectName("questionNumberLabel");
        pageLayout->addWidget(questionNumber);

        QScrollArea *questionTextScroll = new QScrollArea(questionPage);
        questionTextScroll->setWidgetResizable(true);
        questionTextScroll->setFrameShape(QFrame::NoFrame);
        questionTextScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        questionTextScroll->setStyleSheet("QScrollArea { background-color: white; border: none; }");

        QWidget *textContainer = new QWidget();
        textContainer->setStyleSheet("background-color: white;");
        QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
        textLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *questionText = new QLabel(textContainer);
        questionText->setObjectName("questionTextLabel");
        questionText->setWordWrap(true);
        questionText->setText(problem.text());
        questionText->setStyleSheet("background-color: white;");
        textLayout->addWidget(questionText);
        textLayout->addStretch();

        questionTextScroll->setWidget(textContainer);
        pageLayout->addWidget(questionTextScroll, 1);

        // Etiqueta de opciones
        QLabel *optLabel = new QLabel("Selecciona una respuesta:", questionPage);
        optLabel->setObjectName("optionsLabel");
        pageLayout->addWidget(optLabel, 0);

        QButtonGroup *options = new QButtonGroup(questionPage);
        options->setExclusive(true);

        // Obtener las respuestas
        const QVector<Answer> &problemAnswers = problem.answers();

        for (int j = 0; j < problemAnswers.size(); ++j) {
            const Answer &ans = problemAnswers[j];
            QString answerText = ans.text();

            QRadioButton *rb = new QRadioButton(answerText, questionPage);
            rb->setProperty("answerIndex", j);
            options->addButton(rb);
            pageLayout->addWidget(rb, 0);
        }

        // Botón enviar respuesta
        QHBoxLayout *sendButtonLayout = new QHBoxLayout();
        sendButtonLayout->addStretch();

        QPushButton *sendButton = new QPushButton("Enviar respuesta", questionPage);
        sendButton->setObjectName("sendAnswerButton");
        sendButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        sendButtonLayout->addWidget(sendButton);
        sendButtonLayout->addStretch();
        pageLayout->addLayout(sendButtonLayout, 0);

        // Conexión para validar respuesta
        connect(sendButton, &QPushButton::clicked, this, [this, i, options]() {
            QAbstractButton *selectedButton = options->checkedButton();
            if (!selectedButton) {
                QMessageBox::warning(
                    qobject_cast<QWidget*>(parent()),
                    "Advertencia",
                    "Por favor selecciona una respuesta"
                    );
                return;
            }

            int selectedIndex = selectedButton->property("answerIndex").toInt();
            validateAnswer(i, selectedIndex);
        });

        m_questionStack->addWidget(questionPage);
    }
}

void QuestionManager::createProblemSelectionPage()
{
    m_problemSelectionPage = new QWidget();
    QVBoxLayout *selectionLayout = new QVBoxLayout(m_problemSelectionPage);
    selectionLayout->setContentsMargins(15, 15, 15, 15);
    selectionLayout->setSpacing(15);

    // Título
    QLabel *titleLabel = new QLabel("Seleccione un modo de preguntas:", m_problemSelectionPage);
    titleLabel->setObjectName("problemSelectionTitle");
    selectionLayout->addWidget(titleLabel);

    // Botones RANDOM y LISTA (verticales)
    QVBoxLayout *buttonModeLayout = new QVBoxLayout();
    buttonModeLayout->setSpacing(12);
    buttonModeLayout->setAlignment(Qt::AlignCenter);

    m_randomModeButton = new QPushButton("RANDOM", m_problemSelectionPage);
    m_listModeButton = new QPushButton("LISTA", m_problemSelectionPage);

    m_randomModeButton->setObjectName("modeButton");
    m_listModeButton->setObjectName("modeButton");

    m_randomModeButton->setCheckable(true);
    m_listModeButton->setCheckable(true);

    QButtonGroup *modeGroup = new QButtonGroup(m_problemSelectionPage);
    modeGroup->addButton(m_randomModeButton);
    modeGroup->addButton(m_listModeButton);
    modeGroup->setExclusive(true);

    buttonModeLayout->addWidget(m_randomModeButton);
    buttonModeLayout->addWidget(m_listModeButton);

    selectionLayout->addLayout(buttonModeLayout);

    // Tabla de problemas
    m_problemTable = new QTableWidget(m_problemSelectionPage);
    m_problemTable->setObjectName("problemTable");
    m_problemTable->setColumnCount(2);
    m_problemTable->setHorizontalHeaderLabels({"#", "Problema"});
    m_problemTable->horizontalHeader()->setStretchLastSection(true);
    m_problemTable->verticalHeader()->setVisible(false);
    m_problemTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_problemTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_problemTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_problemTable->setVisible(false);

    updateProblemTable();

    selectionLayout->addWidget(m_problemTable);

    // Botones Cancelar y Aceptar
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    m_backButton = new QPushButton("Cancelar", m_problemSelectionPage);
    m_backButton->setObjectName("backButton");

    QPushButton *acceptButton = new QPushButton("Aceptar", m_problemSelectionPage);
    acceptButton->setObjectName("acceptButton");

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_backButton);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addStretch();

    selectionLayout->addLayout(buttonLayout);
    selectionLayout->addStretch();

    // Conexiones
    connect(m_randomModeButton, &QPushButton::toggled, this, &QuestionManager::onModeChanged);
    connect(m_listModeButton, &QPushButton::toggled, this, &QuestionManager::onModeChanged);
    connect(m_problemTable, &QTableWidget::cellClicked, this, &QuestionManager::showProblemFromTable);

    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        if (!m_problems.empty()) {
            showQuestion(0);
        }
    });

    connect(acceptButton, &QPushButton::clicked, this, [this]() {
        if (m_randomModeButton->isChecked()) {
            int problemCount = static_cast<int>(m_problems.size());
            if (problemCount > 0) {
                int randomIndex = QRandomGenerator::global()->bounded(problemCount);
                showQuestion(randomIndex);
            }
        } else if (m_listModeButton->isChecked()) {
            int selectedRow = m_problemTable->currentRow();
            if (selectedRow >= 0 && selectedRow < static_cast<int>(m_problems.size())) {
                showQuestion(selectedRow);
            }
        }
    });

    m_questionStack->addWidget(m_problemSelectionPage);
}

void QuestionManager::toggleSideMenu()
{
    if (m_menuVisible) {
        // Ocultar menú
        m_menuAnimation->setStartValue(m_sideMenuFrame->width());
        m_menuAnimation->setEndValue(0);
        m_menuAnimation->setTargetObject(m_sideMenuFrame);
        m_menuAnimation->setPropertyName("minimumWidth");

        connect(m_menuAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_sideMenuFrame->setMaximumWidth(0);
            disconnect(m_menuAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
        });

        m_menuVisible = false;
        m_menuToggleColumn->setVisible(true);
    } else {
        // Mostrar menú
        m_sideMenuFrame->setMaximumWidth(350);
        m_menuAnimation->setStartValue(0);
        m_menuAnimation->setEndValue(350);
        m_menuAnimation->setTargetObject(m_sideMenuFrame);
        m_menuAnimation->setPropertyName("minimumWidth");

        m_menuVisible = true;
        m_menuToggleColumn->setVisible(false);
    }

    m_menuAnimation->start();
    emit menuVisibilityChanged(m_menuVisible);
}

void QuestionManager::showQuestion(int questionIndex)
{
    if (questionIndex >= 0 && questionIndex < m_problems.size()) {
        m_questionStack->setCurrentIndex(questionIndex);

        // Resetear la página actual
        QWidget *currentPage = m_questionStack->currentWidget();
        if (currentPage && currentPage != m_problemSelectionPage) {
            // Primero, encontrar el QButtonGroup y desmarcar todos
            QButtonGroup *buttonGroup = currentPage->findChild<QButtonGroup*>();
            if (buttonGroup) {
                buttonGroup->setExclusive(false);
                QAbstractButton *checkedBtn = buttonGroup->checkedButton();
                if (checkedBtn) {
                    checkedBtn->setChecked(false);
                }
                buttonGroup->setExclusive(true);
            }

            // Ahora resetear el estilo de cada radio button
            QList<QRadioButton*> radios = currentPage->findChildren<QRadioButton*>();
            for (QRadioButton *rb : radios) {
                rb->setChecked(false);
                rb->setEnabled(true);

                // Aplicar el estilo original explícitamente
                rb->setStyleSheet(
                    "QRadioButton {"
                    "  spacing: 5px;"
                    "}"
                    "QRadioButton::indicator {"
                    "  width: 18px;"
                    "  height: 18px;"
                    "}"
                    "QRadioButton::indicator:unchecked {"
                    "  border: 2px solid #999;"
                    "  border-radius: 9px;"
                    "  background-color: white;"
                    "}"
                    "QRadioButton::indicator:checked {"
                    "  border: 2px solid #2196F3;"
                    "  border-radius: 9px;"
                    "  background-color: #2196F3;"
                    "}"
                    );
            }
        }

        m_questionIndicator->setVisible(true);
        m_prevButton->setVisible(true);
        m_nextButton->setVisible(true);

        updateNavigationButtons();
        updateQuestionIndicator();
    }
}

void QuestionManager::showProblemSelection()
{
    m_questionStack->setCurrentWidget(m_problemSelectionPage);
    m_randomModeButton->setChecked(false);
    m_listModeButton->setChecked(false);
    m_problemTable->setVisible(false);
    m_problemTable->clearSelection();

    m_questionIndicator->setVisible(false);
    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
}

void QuestionManager::showProblemFromTable(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < static_cast<int>(m_problems.size())) {
        showQuestion(row);
    }
}

void QuestionManager::onModeChanged()
{
    if (m_listModeButton->isChecked()) {
        m_problemTable->setVisible(true);
    } else {
        m_problemTable->setVisible(false);
    }
}

void QuestionManager::updateNavigationButtons()
{
    int current = m_questionStack->currentIndex();
    int totalQuestions = static_cast<int>(m_problems.size());

    if (current >= 0 && current < totalQuestions) {
        m_prevButton->setEnabled(current > 0);
        m_nextButton->setEnabled(current < totalQuestions - 1);
    } else {
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
    }
}

void QuestionManager::updateQuestionIndicator()
{
    int current = m_questionStack->currentIndex();
    int totalQuestions = static_cast<int>(m_problems.size());

    if (current >= 0 && current < totalQuestions) {
        m_questionIndicator->setText(
            QString("%1 / %2").arg(current + 1).arg(totalQuestions)
            );
    }
}

void QuestionManager::updateProblemTable()
{
    if (!m_problemTable) return;

    m_problemTable->setRowCount(static_cast<int>(m_problems.size()));

    for (size_t i = 0; i < m_problems.size(); ++i) {
        const Problem &problem = m_problems[i];

        // Columna de número
        QTableWidgetItem *numberItem = new QTableWidgetItem(QString::number(i + 1));
        numberItem->setTextAlignment(Qt::AlignCenter);
        m_problemTable->setItem(static_cast<int>(i), 0, numberItem);

        // Columna de descripción
        QString statement = problem.text();
        if (statement.length() > 60) {
            statement = statement.left(60) + "...";
        }

        QTableWidgetItem *statementItem = new QTableWidgetItem(statement);
        m_problemTable->setItem(static_cast<int>(i), 1, statementItem);
    }
}

void QuestionManager::validateAnswer(size_t problemIndex, int selectedAnswerIndex)
{
    if (problemIndex >= m_problems.size()) {
        qWarning() << "Índice de problema fuera de rango:" << problemIndex;
        return;
    }

    const Problem &prob = m_problems[problemIndex];
    const QVector<Answer> &answers = prob.answers();

    if (selectedAnswerIndex < 0 || selectedAnswerIndex >= answers.size()) {
        qWarning() << "Índice de respuesta fuera de rango:" << selectedAnswerIndex;
        return;
    }

    bool correct = answers[selectedAnswerIndex].validity();

    QWidget *currentPage = m_questionStack->currentWidget();
    if (!currentPage) return;

    QList<QRadioButton*> radioButtons = currentPage->findChildren<QRadioButton*>();

    for (QRadioButton *rb : radioButtons) {
        int idx = rb->property("answerIndex").toInt();
        bool isCorrect = answers[idx].validity();

        if (isCorrect) {
            rb->setStyleSheet(
                "QRadioButton { "
                "  color: #155724; "
                "  font-weight: bold; "
                "  background-color: #d4edda; "
                "  padding: 6px; "
                "  border-radius: 5px; "
                "} "
                "QRadioButton::indicator:checked { "
                "  background-color: #28a745; "
                "  border: 2px solid #28a745; "
                "}"
                );
        }
        else if(rb->isChecked()){
            rb->setStyleSheet(
                "QRadioButton { "
                "  color: #721c24; "
                "  font-weight: bold; "
                "  background-color: #f8d7da; "
                "  padding: 6px; "
                "  border-radius: 5px; "
                "} "
                "QRadioButton::indicator:checked { "
                "  background-color: #dc3545; "
                "  border: 2px solid #dc3545; "
                "}"
                );
        }
        rb->setEnabled(false);
    }

    emit problemAnswered(static_cast<int>(problemIndex), correct);
}
