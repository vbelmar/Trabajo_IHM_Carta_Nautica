#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "authwindow.h"
#include <QGraphicsPixmapItem>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSlider>
#include <QMouseEvent>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTextEdit>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPen>
#include <QLineF>
#include <QMessageBox>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QWheelEvent>
#include <QInputDialog>
#include <QColorDialog>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <cmath>
#include <QStackedWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
// Asumir que Tool y Compass son clases personalizadas definidas en otros headers
// Si no, necesitarás incluirlos o definirlos.

MainWindow::MainWindow(const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , view(new QGraphicsView(this))
    , currentZoom(0.1)
    , minZoom(0.1)
    , maxZoom(2.0)
    , m_drawPointMode(false)
    , m_drawLineMode(false)
    , m_drawTextMode(false)
    , m_protractorMode(false)
    , m_rulerMode(false)
    , m_rubberMode(false)
    , m_compassMode(false)
    , m_coordinateMode(false)
    , m_compassPivotSet(false)  //NUEVO COMPAS
    , m_currentLineItem(nullptr)
    , m_currentLineItemBorder(nullptr)
    , m_drawingLine(false)
    , m_protractor(nullptr)
    , m_ruler(nullptr)
    , m_compass(nullptr)
    , m_protractorBaseScale(1000.0)
    , m_rulerBaseScale(1000.0)
    , m_compassHasRadius(false)
    , m_compassArc(nullptr)
    // NUEVO COMPAS
    , m_compassFixedLeg(nullptr)
    , m_drawingColor(Qt::red)
    , m_drawingSize(8)
    , m_sizeSlider(nullptr)
    , m_sizeLabel(nullptr)
    , m_coordLabel(nullptr)
    //, m_menuVisible(true)
    , m_currentUsername(username)
    , m_pages(new QStackedWidget(this))
    , m_mapPage(new QWidget(this))
    //LUNA
    , m_questionManager(nullptr)
    , nav(Navigation::instance())
    , m_currentUser(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("POLICHART");

    // Cargar estilos
    //loadStyleSheet();

    view->setScene(scene);

    QPixmap pm(":/new/prefix1/resources/carta_nautica.jpg");
    QGraphicsPixmapItem *item = scene->addPixmap(pm);
    item->setZValue(0);

    view->scale(currentZoom, currentZoom);
    view->setDragMode(QGraphicsView::NoDrag);

    // Desactivar scroll con rueda
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Modos del ratón
    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->addAction(ui->actionMoveMode);
    modeGroup->addAction(ui->actionClickMode);
    ui->actionMoveMode->setCheckable(true);
    ui->actionClickMode->setCheckable(true);
    ui->actionClickMode->setChecked(true);

    // Herramientas dibujo
    QActionGroup *toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(ui->actionPoint);
    toolsGroup->addAction(ui->actionDrawLine);
    toolsGroup->addAction(ui->actionText);
    toolsGroup->addAction(ui->actionProtractor);
    toolsGroup->addAction(ui->actionRuler);
    toolsGroup->addAction(ui->actionRubber);
    toolsGroup->addAction(ui->actionCompass);
    toolsGroup->addAction(ui->actionCoordinate);

    ui->actionPoint->setCheckable(true);
    ui->actionDrawLine->setCheckable(true);
    ui->actionText->setCheckable(true);
    ui->actionProtractor->setCheckable(true);
    ui->actionRuler->setCheckable(true);
    ui->actionRubber->setCheckable(true);
    ui->actionCompass->setCheckable(true);
    ui->actionCoordinate->setCheckable(true);

    toolsGroup->setExclusive(false);

    // Slider zoom
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(100);
    zoomSlider->setFixedWidth(350);

    int initialValue = (std::log(currentZoom / minZoom) / std::log(maxZoom / minZoom)) * 100;
    zoomSlider->setValue(initialValue);

    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addStretch();
    sliderLayout->addWidget(zoomSlider);
    sliderLayout->addStretch();
    sliderLayout->setContentsMargins(0, 5, 0, 5);

    // Slider tamaño (ahora horizontal para el toolbar)
    m_sizeSlider = new QSlider(Qt::Horizontal, this);
    m_sizeSlider->setMinimum(2);
    m_sizeSlider->setMaximum(50);
    m_sizeSlider->setValue(m_drawingSize);
    m_sizeSlider->setFixedWidth(150);  // Ajustado para toolbar

    m_sizeLabel = new QLabel(QString::number(m_drawingSize), this);
    m_sizeLabel->setAlignment(Qt::AlignCenter);
    m_sizeLabel->setFixedWidth(30);  // Pequeño para toolbar

    // Setup Header y toolbar (el slider de tamaño se añade al toolbar)
    setupHeader();
    setupMapToolbar();

    // Layout mapa
    QVBoxLayout *mapLayout = new QVBoxLayout();
    mapLayout->setSpacing(0);
    mapLayout->setContentsMargins(0, 0, 0, 0);
    mapLayout->addWidget(m_mapToolbar);
    mapLayout->addWidget(view, 1);
    mapLayout->addLayout(sliderLayout);

    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addLayout(mapLayout, 1);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    m_questionManager = new QuestionManager(this);
    m_questionManager->setupSideMenu(this);
    m_questionManager->loadProblems(nav);

    connect(m_questionManager, &QuestionManager::problemAnswered, this, &MainWindow::onProblemAnswered);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(m_questionManager->getMenuToggleColumn());
    contentLayout->addWidget(m_questionManager->getSideMenuFrame());
    contentLayout->addLayout(centerLayout, 1);

    QWidget *mapContentWidget = new QWidget();
    mapContentWidget->setLayout(contentLayout);
    m_mapPage = mapContentWidget;

    m_pages->addWidget(m_mapPage);
    m_historyWindow = nullptr;

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(m_headerWidget);
    mainLayout->addWidget(m_pages, 1);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);


    // Conexiones modos ratón
    connect(ui->actionMoveMode, &QAction::triggered, this, &MainWindow::setMoveMode);
    connect(ui->actionClickMode, &QAction::triggered, this, &MainWindow::setClickMode);

    // Conexiones herramientas
    connect(ui->actionPoint, &QAction::triggered, this, &MainWindow::togglePointMode);
    connect(ui->actionDrawLine, &QAction::triggered, this, &MainWindow::toggleLineMode);
    connect(ui->actionText, &QAction::triggered, this, &MainWindow::toggleTextMode);
    connect(ui->actionProtractor, &QAction::triggered, this, &MainWindow::toggleProtractorMode);
    connect(ui->actionRuler, &QAction::triggered, this, &MainWindow::toggleRulerMode);
    connect(ui->actionRubber, &QAction::triggered, this, &MainWindow::toggleRubberMode);
    connect(ui->actionCompass, &QAction::triggered, this, &MainWindow::toggleCompassMode);
    connect(ui->actionCoordinate, &QAction::triggered, this, &MainWindow::toggleCoordinateMode);

    connect(ui->actionColor, &QAction::triggered, this, &MainWindow::selectColor);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteCanvas);

    // Conectar sliders
    connect(zoomSlider, &QSlider::valueChanged, this, &MainWindow::onZoomSliderChanged);
    connect(m_sizeSlider, &QSlider::valueChanged, this, &MainWindow::onSizeSliderChanged);

    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showControlsHelp);

    // Event filter
    view->viewport()->installEventFilter(this);

    // Crear herramientas
    m_protractor = new Tool(":/new/prefix1/resources/icons/transportador.svg");
    m_protractor->setToolSize(QSizeF(1500, 1500));
    m_protractor->setZValue(100);
    m_protractor->setVisible(false);
    m_protractor->setFlag(QGraphicsItem::ItemIsMovable, false);
    scene->addItem(m_protractor);

    // Guardar posición inicial
    m_protractorScenePos = view->mapToScene(view->viewport()->rect().center());
    m_protractor->setPos(m_protractorScenePos);

    m_ruler = new Tool(":/new/prefix1/resources/icons/ruler.svg");
    m_ruler->setToolSize(QSizeF(3000, 3000));
    m_ruler->setZValue(100);
    m_ruler->setVisible(false);
    m_ruler->setFlag(QGraphicsItem::ItemIsMovable, false);
    scene->addItem(m_ruler);
    m_ruler->setPos(m_protractorScenePos);

    // Crear pata móvil del compás
    m_compass = new Compass(":/new/prefix1/resources/icons/compass_leg.svg");
    m_compass->setSize(QSizeF(800, 400));  // CAMBIADO: de 1800 a 800 inicialmente
    m_compass->setZValue(101);
    m_compass->setVisible(false);
    m_compass->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(m_compass);

    // Crear pata fija del compás (también SVG)
    m_compassFixedLeg = new Compass(":/new/prefix1/resources/icons/compass_leg.svg");
    m_compassFixedLeg->setSize(QSizeF(800, 400));  // CAMBIADO: de 1800 a 800 inicialmente
    m_compassFixedLeg->setZValue(100);
    m_compassFixedLeg->setVisible(false);
    m_compassFixedLeg->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(m_compassFixedLeg);


    m_compass->setPos(m_protractorScenePos);

    m_compassStopTimer = new QTimer(this);
    m_compassStopTimer->setSingleShot(true);
    m_compassStopTimer->setInterval(300);
    connect(m_compassStopTimer, &QTimer::timeout, this, &MainWindow::drawCompassFinalArc);

    goToMapPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStyleSheet() {
    QFile file(":/new/prefix1/resources/styles.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }
}

void MainWindow::setupHeader() {
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("headerWidget");
    m_headerWidget->setFixedHeight(60);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(15, 5, 15, 5);
    headerLayout->setSpacing(15);

    QHBoxLayout *leftSection = new QHBoxLayout();
    leftSection->setSpacing(10);

    m_avatarLabel = new QLabel(m_headerWidget);
    m_avatarLabel->setFixedSize(35, 35);
    m_avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #3498DB;"
        "   color: white;"
        "   border-radius: 17px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        );
    m_avatarLabel->setAlignment(Qt::AlignCenter);

    m_currentUser = nav.findUser(m_currentUsername);

    if (m_currentUser) {
        QImage avatar = m_currentUser->avatar();
        if (!avatar.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(avatar).scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap rounded(35, 35);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(0, 0, 35, 35);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pixmap);
            m_avatarLabel->setPixmap(rounded);
            m_avatarLabel->setStyleSheet("QLabel { border-radius: 17px; }");
        } else {
            QString initials = m_currentUsername.left(2).toUpper();
            m_avatarLabel->setText(initials);
        }
    } else {
        m_avatarLabel->setText("XX");
    }

    m_avatarLabel->setCursor(Qt::PointingHandCursor);
    m_avatarLabel->installEventFilter(this);

    leftSection->addWidget(m_avatarLabel);

    m_usernameLabel = new QLabel(m_currentUsername, m_headerWidget);
    m_usernameLabel->setObjectName("usernameLabel");
    m_usernameLabel->setCursor(Qt::PointingHandCursor);
    m_usernameLabel->installEventFilter(this);
    leftSection->addWidget(m_usernameLabel);

    m_historyButton = new QPushButton("Mostrar historial", m_headerWidget);
    m_historyButton->setObjectName("headerButton");
    connect(m_historyButton, &QPushButton::clicked, this, &MainWindow::showHistory);
    leftSection->addWidget(m_historyButton);

    headerLayout->addLayout(leftSection);

    headerLayout->addStretch();
    m_appTitle = new QLabel("POLICHART", m_headerWidget);
    m_appTitle->setObjectName("appTitle");
    headerLayout->addWidget(m_appTitle);
    headerLayout->addStretch();

    m_logoutButton = new QPushButton("CERRAR SESIÓN", m_headerWidget);
    m_logoutButton->setObjectName("logoutButton");
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
    headerLayout->addWidget(m_logoutButton);
}

void MainWindow::setupMapToolbar() {
    m_mapToolbar = new QToolBar(this);
    m_mapToolbar->setObjectName("mapToolbar");
    m_mapToolbar->setMovable(false);
    m_mapToolbar->setFloatable(false);
    m_mapToolbar->setIconSize(QSize(30, 30));

    // Grupo 1: Modos de ratón (flecha y mano)
    m_mapToolbar->addAction(ui->actionClickMode);
    m_mapToolbar->addAction(ui->actionMoveMode);

    // Separador después del grupo 1
    m_mapToolbar->addSeparator();

    // Grupo 2: Métodos de dibujo (punto, línea, texto)
    m_mapToolbar->addAction(ui->actionPoint);
    m_mapToolbar->addAction(ui->actionDrawLine);
    m_mapToolbar->addAction(ui->actionText);
    m_mapToolbar->addAction(ui->actionCoordinate);
    m_mapToolbar->addAction(ui->actionRubber);

    // Separador después del grupo 2
    m_mapToolbar->addSeparator();

    // Grupo 3: Herramientas de dibujo (transportador, regla, compás, goma, coordenadas, color)
    m_mapToolbar->addAction(ui->actionProtractor);
    m_mapToolbar->addAction(ui->actionRuler);
    m_mapToolbar->addAction(ui->actionCompass);

    // Separador después del grupo 3
    m_mapToolbar->addSeparator();

    // Papelera
    m_mapToolbar->addAction(ui->actionDelete);

    // Separador después de la papelera
    m_mapToolbar->addSeparator();

    // Slider de tamaño
    m_mapToolbar->addAction(ui->actionColor);
    m_mapToolbar->addWidget(new QLabel("Tamaño:", this));
    m_mapToolbar->addWidget(m_sizeSlider);
    m_mapToolbar->addWidget(m_sizeLabel);

    // Botón de información de controles
    m_mapToolbar->addSeparator();

    m_mapToolbar->addAction(ui->actionHelp);

    /*
    QPushButton *helpButton = new QPushButton("?", this);
    helpButton->setObjectName("helpButton");
    helpButton->setFixedSize(60, 60);
    helpButton->setToolTip("Mostrar controles");
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::showControlsHelp);
    m_mapToolbar->addWidget(helpButton);
*/
}

void MainWindow::logout() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Cerrar Sesión",
        "¿Está seguro de que desea cerrar sesión?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (m_currentUser && (m_sessionHits > 0 || m_sessionFaults > 0)) {
            Session s(QDateTime::currentDateTime(), m_sessionHits, m_sessionFaults);
            m_currentUser->addSession(s);

            try {
                nav.addSession(m_currentUsername, s);
                qDebug() << "Sesión guardada en BD";
            } catch (const std::exception &e) {
                qDebug() << "Error guardando sesión:" << e.what();
            }
        }

        this->deleteLater();

        // Crear nueva AuthWindow
        AuthWindow *authWindow = new AuthWindow();
        authWindow->setWindowState(Qt::WindowMaximized);

        // Mostrar login
        if (authWindow->exec() == QDialog::Accepted) {
            QString username = authWindow->getLoggedUsername();
            MainWindow *newMainWindow = new MainWindow(username);
            newMainWindow->showMaximized();
        } else {
            QApplication::quit();
        }

        // Eliminar AuthWindow explícitamente
        delete authWindow;
    }
}

void MainWindow::showHistory()
{
    if (!m_currentUser) {
        m_currentUser = nav.findUser(m_currentUsername);
        if (!m_currentUser) {
            QMessageBox::warning(this, "Error", "No se pudo cargar el usuario actual.");
            return;
        }
    }

    if (m_pages->currentWidget() == m_historyWindow) {
        goToMapPage();
        return;
    }

    if (!m_historyWindow) {
        m_historyWindow = new HistoryWindow(m_currentUser, this);
        m_pages->addWidget(m_historyWindow);

        connect(m_historyWindow, &HistoryWindow::backToMap, this, &MainWindow::goToMapPage);

    }

    m_historyWindow->refreshData();
    m_pages->setCurrentWidget(m_historyWindow);

    if (m_appTitle) m_appTitle->setText("HISTORIAL");
}

void MainWindow::setMoveMode() {
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->actionMoveMode->setChecked(true);
    ui->actionClickMode->setChecked(false);
    view->setCursor(Qt::OpenHandCursor);
}

void MainWindow::setClickMode() {
    view->setDragMode(QGraphicsView::NoDrag);
    ui->actionClickMode->setChecked(true);
    ui->actionMoveMode->setChecked(false);
    view->setCursor(Qt::ArrowCursor);
}

void MainWindow::disableAllTools() {
    m_drawPointMode = false;
    m_drawLineMode = false;
    m_drawTextMode = false;
    m_rubberMode = false;
    m_coordinateMode = false;

    ui->actionPoint->setChecked(false);
    ui->actionDrawLine->setChecked(false);
    ui->actionText->setChecked(false);
    ui->actionRubber->setChecked(false);
    ui->actionCoordinate->setChecked(false);

    /*if (m_protractor) m_protractor->setVisible(false);
    if (m_ruler) m_ruler->setVisible(false);
    if (m_compass) m_compass->setVisible(false);*/
    if (m_coordLabel) m_coordLabel->setVisible(false);
}

void MainWindow::togglePointMode() {
    if (m_drawPointMode) {
        disableAllTools();
    } else {
        disableAllTools();
        m_drawPointMode = true;
        ui->actionPoint->setChecked(true);
        view->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::toggleLineMode() {
    if (m_drawLineMode) {
        disableAllTools();
    } else {
        disableAllTools();
        m_drawLineMode = true;
        ui->actionDrawLine->setChecked(true);
        view->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::toggleTextMode() {
    if (m_drawTextMode) {
        disableAllTools();
    } else {
        disableAllTools();
        m_drawTextMode = true;
        ui->actionText->setChecked(true);
        view->setCursor(Qt::IBeamCursor);
    }
}

void MainWindow::toggleProtractorMode() {
    if (m_protractorMode) {
        m_protractorMode = false;
        m_protractor->setVisible(false);
        ui->actionProtractor->setChecked(false);
    } else {
        m_protractorMode = true;
        ui->actionProtractor->setChecked(true);
        view->setCursor(Qt::OpenHandCursor);
        if (m_protractor) {
            m_protractor->setVisible(true);
            m_protractor->setFlag(QGraphicsItem::ItemIsMovable, true);
            updateProtractorPosition();
        }
    }
}

void MainWindow::toggleRulerMode() {
    if (m_rulerMode) {
        m_rulerMode = false;
        m_ruler->setVisible(false);
        ui->actionRuler->setChecked(false);
    } else {
        m_rulerMode = true;
        ui->actionRuler->setChecked(true);
        view->setCursor(Qt::OpenHandCursor);
        if (m_ruler) {
            m_ruler->setVisible(true);
            m_ruler->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_ruler->setPos(view->mapToScene(view->viewport()->rect().center()));
        }
    }
}

void MainWindow::toggleRubberMode() {
    if (m_rubberMode) {
        disableAllTools();
    } else {
        disableAllTools();
        m_rubberMode = true;
        ui->actionRubber->setChecked(true);
        view->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::toggleCompassMode() {
    if (m_compassMode) {
        m_compassMode = false;
        m_compass->setVisible(false);
        m_compassFixedLeg->setVisible(false);
        ui->actionCompass->setChecked(false);
        m_compassPivotSet = false;
        m_compassHasRadius = false;
        m_compassRotationInProgress = false;
        view->setCursor(Qt::ArrowCursor);
    } else {
        m_compassMode = true;
        ui->actionCompass->setChecked(true);

        if (m_compass && m_compassFixedLeg) {
            QPointF centerPos = view->mapToScene(view->viewport()->rect().center());

            // Tamaño fijo inicial
            qreal initialRadius = 500;

            m_compass->setVisible(true);
            m_compass->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_compass->setPos(centerPos);
            m_compass->setAngleDegrees(0);
            m_compass->setSize(QSizeF(initialRadius, 400));

            m_compassFixedLeg->setVisible(true);
            m_compassFixedLeg->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_compassFixedLeg->setPos(centerPos);
            m_compassFixedLeg->setAngleDegrees(180);
            m_compassFixedLeg->setSize(QSizeF(initialRadius, 400));

            m_compassRadius = initialRadius;
        }

        m_compassPivotSet = false;
        m_compassHasRadius = false;
        statusBar()->showMessage("Compás activado. Click para definir longitud. Modo Move para moverlo.", 3000);
    }
}
void MainWindow::toggleCoordinateMode() {
    if (m_coordinateMode) {
        m_coordinateMode = false;  // Resetear el estado
        ui->actionCoordinate->setChecked(false);
        view->setCursor(Qt::ArrowCursor);
        // Ya no necesitamos el label de coordenadas
    } else {
        disableAllTools();
        m_coordinateMode = true;
        ui->actionCoordinate->setChecked(true);
        view->setCursor(Qt::CrossCursor);
    }
}

void MainWindow::selectColor() {
    QColor color = QColorDialog::getColor(m_drawingColor, this, "Seleccionar color de dibujo",
                                          QColorDialog::DontUseNativeDialog);

    if (color.isValid()) {
        m_drawingColor = color;
        statusBar()->showMessage(QString("Color seleccionado: %1").arg(color.name()), 2000);
    }
}

void MainWindow::onSizeSliderChanged(int value) {
    m_drawingSize = value;
    m_sizeLabel->setText(QString::number(value));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if ((obj == m_avatarLabel || obj == m_usernameLabel) && event->type() == QEvent::MouseButtonPress) {
        showEditProfile();
        return true;
    }

    if (obj == view->viewport()) {
        if (event->type() == QEvent::Wheel) {
            auto *e = static_cast<QWheelEvent*>(event);
            QPointF scenePos = view->mapToScene(e->position().toPoint());
            QGraphicsItem *itemUnderMouse = scene->itemAt(scenePos, view->transform());

            // Detectar sobre qué pata está el ratón
            bool overMovingLeg = (itemUnderMouse == m_compass || itemUnderMouse->parentItem() == m_compass);
            bool overFixedLeg = (itemUnderMouse == m_compassFixedLeg || itemUnderMouse->parentItem() == m_compassFixedLeg);

            if (overMovingLeg || overFixedLeg) {
                if (m_compassHasRadius) {
                    bool shiftPressed = (QApplication::keyboardModifiers() & Qt::ShiftModifier);

                    if (shiftPressed && overMovingLeg) {
                        // SHIFT + RUEDA sobre pata móvil: pintar arco
                        if (!m_compassRotationInProgress) {
                            m_compassStartAngle = m_compass->rotation();
                            m_compassRotationInProgress = true;
                        }
                        m_compassStopTimer->start();
                    }

                    // La rotación de cada pata la maneja su propio wheelEvent
                }
                return false;  // Dejar que wheelEvent de Compass maneje la rotación
            }

            // Otras herramientas
            if (itemUnderMouse && (itemUnderMouse == m_protractor || itemUnderMouse == m_ruler)) {
                return false;
            }

            event->ignore();
            return true;
        }

        if (event->type() == QEvent::MouseMove) {
            auto *e = static_cast<QMouseEvent*>(event);

            // En MODO MOVE: mover ambas patas juntas (cambiar pivote)
            if (m_compassMode && ui->actionMoveMode->isChecked() && (e->buttons() & Qt::LeftButton)) {
                QPointF scenePos = view->mapToScene(e->pos());
                QGraphicsItem *itemUnderMouse = scene->itemAt(scenePos, view->transform());

                if (itemUnderMouse == m_compass || itemUnderMouse == m_compassFixedLeg ||
                    itemUnderMouse->parentItem() == m_compass || itemUnderMouse->parentItem() == m_compassFixedLeg) {

                    QPointF newPos = (itemUnderMouse == m_compass || itemUnderMouse->parentItem() == m_compass)
                    ? m_compass->pos()
                    : m_compassFixedLeg->pos();

                    m_compass->setPos(newPos);
                    m_compassFixedLeg->setPos(newPos);

                    // Actualizar pivote
                    m_compassPivotPoint = newPos;

                    return true;
                }
            }

            // Líneas de dibujo
            if (m_drawingLine && m_currentLineItem) {
                QPointF p2 = view->mapToScene(e->pos());
                m_currentLineItem->setLine(QLineF(m_lineStart, p2));
                return true;
            }
        }

        if (m_rubberMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton) {
                    QPointF scenePos = view->mapToScene(e->pos());
                    QGraphicsItem *itemToDelete = scene->itemAt(scenePos, view->transform());

                    if (itemToDelete) {
                        if (itemToDelete->zValue() >= 8) {
                            QGraphicsEllipseItem *ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(itemToDelete);
                            if (ellipseItem && m_drawnPoints.contains(ellipseItem)) {
                                m_drawnPoints.removeOne(ellipseItem);
                                scene->removeItem(ellipseItem);
                                delete ellipseItem;
                                return true;
                            }

                            QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem*>(itemToDelete);
                            if (lineItem && m_drawnLines.contains(lineItem)) {
                                m_drawnLines.removeOne(lineItem);
                                scene->removeItem(lineItem);
                                delete lineItem;
                                return true;
                            }

                            QGraphicsPathItem *pathItem = dynamic_cast<QGraphicsPathItem*>(itemToDelete);
                            if (pathItem && m_compassArcs.contains(pathItem)) {
                                m_compassArcs.removeOne(pathItem);
                                scene->removeItem(pathItem);
                                delete pathItem;
                                return true;
                            }

                            if (m_drawnTexts.contains(itemToDelete)) {
                                m_drawnTexts.removeOne(itemToDelete);
                                scene->removeItem(itemToDelete);
                                delete itemToDelete;
                                return true;
                            }
                        }
                    }
                    return true;
                }
            }
        }

        if (m_drawPointMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton) {
                    QPointF scenePos = view->mapToScene(e->pos());
                    qreal pointRadius = m_drawingSize / 2.0;

                    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(-pointRadius, -pointRadius,
                                                                           m_drawingSize, m_drawingSize);
                    point->setPos(scenePos);
                    point->setBrush(QBrush(m_drawingColor));
                    point->setPen(Qt::NoPen);
                    point->setZValue(10);

                    scene->addItem(point);
                    m_drawnPoints.append(point);
                    return true;
                }
            }
        }

        if (m_drawLineMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton && !m_drawingLine) {
                    m_lineStart = view->mapToScene(e->pos());
                    QPen pen(m_drawingColor, m_drawingSize);
                    m_currentLineItem = new QGraphicsLineItem();
                    m_currentLineItem->setZValue(10);
                    m_currentLineItem->setPen(pen);
                    m_currentLineItem->setLine(QLineF(m_lineStart, m_lineStart));
                    scene->addItem(m_currentLineItem);
                    m_drawingLine = true;
                    return true;
                } else if (e->button() == Qt::LeftButton && m_drawingLine) {
                    QPointF p2 = view->mapToScene(e->pos());
                    if (m_currentLineItem) {
                        m_currentLineItem->setLine(QLineF(m_lineStart, p2));
                        m_drawnLines.append(m_currentLineItem);
                    }
                    m_currentLineItem = nullptr;
                    m_drawingLine = false;
                    return true;
                }
            }
        }

        if (m_drawTextMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton) {
                    QPointF scenePos = view->mapToScene(e->pos());

                    bool ok;
                    QString text = QInputDialog::getText(this, "Escribir texto",
                                                         "Introduce el texto:",
                                                         QLineEdit::Normal, "", &ok);

                    if (ok && !text.isEmpty()) {
                        int fontSize = qMax(8, m_drawingSize);
                        QFont textFont("Arial", fontSize, QFont::Bold);

                        QPainterPath path;
                        path.addText(0, 0, textFont, text);

                        QGraphicsPathItem *textPath = new QGraphicsPathItem(path);
                        textPath->setPos(scenePos);
                        textPath->setPen(Qt::NoPen);
                        textPath->setBrush(QBrush(m_drawingColor));
                        textPath->setZValue(10);
                        scene->addItem(textPath);
                        m_drawnTexts.append(textPath);
                    }
                    return true;
                }
            }
        }

        if (m_compassMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                QPointF clickScene = view->mapToScene(e->pos());

                if (e->button() == Qt::LeftButton) {
                    // UN SOLO CLICK: Definir longitud de ambas patas
                    m_compassPivotPoint = m_compass->pos();

                    QLineF radiusLine(m_compassPivotPoint, clickScene);
                    m_compassRadius = radiusLine.length();

                    // Validar radio
                    if (m_compassRadius < 50) {
                        statusBar()->showMessage("Longitud muy pequeña. Haz click más lejos.", 2000);
                        return true;
                    }
                    if (m_compassRadius > 3000) {
                        m_compassRadius = 3000;
                    }

                    m_compassHasRadius = true;
                    m_compassPivotSet = true;

                    // Calcular ángulo hacia el click
                    qreal angle = -radiusLine.angle();

                    // Ajustar tamaño de AMBAS patas
                    m_compass->setSize(QSizeF(m_compassRadius, 400));
                    m_compassFixedLeg->setSize(QSizeF(m_compassRadius, 400));

                    // Mantener orientación desde el click
                    m_compass->setPos(m_compassPivotPoint);
                    m_compass->setAngleDegrees(angle);

                    m_compassFixedLeg->setPos(m_compassPivotPoint);
                    m_compassFixedLeg->setAngleDegrees(angle + 180);

                    m_compassStartAngle = angle;

                    statusBar()->showMessage(
                        QString("Longitud: %1 px. Rueda: girar pata. Shift+Rueda: pintar arco. Modo Move: mover compás.")
                            .arg((int)m_compassRadius), 4000);

                    return true;

                } else if (e->button() == Qt::RightButton) {
                    // RESETEAR: volver al tamaño inicial
                    m_compassPivotSet = false;
                    m_compassHasRadius = false;
                    m_compassRotationInProgress = false;

                    QPointF currentPos = m_compass->pos();
                    qreal initialRadius = 500;

                    m_compass->setSize(QSizeF(initialRadius, 400));
                    m_compassFixedLeg->setSize(QSizeF(initialRadius, 400));
                    m_compass->setPos(currentPos);
                    m_compassFixedLeg->setPos(currentPos);
                    m_compass->setAngleDegrees(0);
                    m_compassFixedLeg->setAngleDegrees(180);

                    m_compassRadius = initialRadius;

                    if (m_compassArc) {
                        scene->removeItem(m_compassArc);
                        delete m_compassArc;
                        m_compassArc = nullptr;
                    }

                    statusBar()->showMessage("Compás reseteado.", 2000);
                    return true;
                }
            }
        }

        if (m_coordinateMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton) {
                    QPointF scenePos = view->mapToScene(e->pos());

                    QRectF sceneRect = scene->sceneRect();
                    if (sceneRect.isEmpty()) {
                        sceneRect = QRectF(0, 0, 3000, 2000);
                    }

                    // Línea vertical
                    QGraphicsLineItem *verticalLine = new QGraphicsLineItem();
                    verticalLine->setLine(scenePos.x(), sceneRect.top(),
                                          scenePos.x(), sceneRect.bottom());
                    verticalLine->setPen(QPen(m_drawingColor, m_drawingSize));
                    verticalLine->setZValue(10);
                    scene->addItem(verticalLine);
                    m_drawnLines.append(verticalLine);

                    // Línea horizontal
                    QGraphicsLineItem *horizontalLine = new QGraphicsLineItem();
                    horizontalLine->setLine(sceneRect.left(), scenePos.y(),
                                            sceneRect.right(), scenePos.y());
                    horizontalLine->setPen(QPen(m_drawingColor, m_drawingSize));
                    horizontalLine->setZValue(10);
                    scene->addItem(horizontalLine);
                    m_drawnLines.append(horizontalLine);

                    return true;
                }
            }
        }
    }  // <-- LLAVE DE CIERRE del if (obj == view->viewport())

    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::onZoomSliderChanged(int value) {
    qreal t = value / 100.0;
    qreal newZoom = minZoom * std::pow(maxZoom / minZoom, t);
    updateZoom(newZoom);
}

void MainWindow::updateZoom(qreal newZoom) {
    qreal scaleFactor = newZoom / currentZoom;
    view->scale(scaleFactor, scaleFactor);
    currentZoom = newZoom;

    updateProtractorPosition();
}

void MainWindow::updateProtractorPosition() {
    if (m_protractor && m_protractor->isVisible()) {
        QPointF centerView = view->mapToScene(view->viewport()->rect().center());
        m_protractorScenePos = centerView;
        m_protractor->setPos(m_protractorScenePos);
    }
}

void MainWindow::deleteCanvas() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmar reinicio",
                                                              "¿Está seguro de que desea reiniciar el mapa?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    // Limpiar líneas
    qDeleteAll(m_drawnLines);
    m_drawnLines.clear();

    // Limpiar puntos
    qDeleteAll(m_drawnPoints);
    m_drawnPoints.clear();

    // Limpiar textos
    qDeleteAll(m_drawnTexts);
    m_drawnTexts.clear();

    // Limpiar arcos
    qDeleteAll(m_compassArcs);
    m_compassArcs.clear();

    if (m_currentLineItem) {
        delete m_currentLineItem;
        m_currentLineItem = nullptr;
    }

    if (m_protractorMode){
        m_protractorMode = false;
        m_protractor->setVisible(false);
        ui->actionProtractor->setChecked(false);
    }

    if (m_rulerMode){
        m_rulerMode = false;
        m_ruler->setVisible(false);
        ui->actionRuler->setChecked(false);
    }

    if (m_compassMode){
        m_compassMode = false;
        m_compass->setVisible(false);
        ui->actionCompass->setChecked(false);

    }


    disableAllTools();

    view->setCursor(Qt::ArrowCursor);
    scene->update();
}

void MainWindow::drawCompassFinalArc() {
    if (!m_compass || !m_compassHasRadius || !m_compassPivotSet) return;

    QPointF pivot = m_compassPivotPoint;  // Usar el pivote establecido
    qreal radius = m_compassRadius;
    qreal startAngle = m_compassStartAngle;
    qreal endAngle = m_compass->rotation();

    QRectF rect(pivot.x() - radius, pivot.y() - radius, radius * 2, radius * 2);

    QPainterPath path;
    qreal startForArc = -startAngle;
    qreal spanForArc = -(endAngle - startAngle);

    path.arcMoveTo(rect, startForArc);
    path.arcTo(rect, startForArc, spanForArc);

    QGraphicsPathItem *arc = new QGraphicsPathItem(path);
    arc->setPen(QPen(m_drawingColor, qMax(2.0, (qreal)m_drawingSize / 2.0)));
    arc->setZValue(8);
    scene->addItem(arc);
    m_compassArcs.append(arc);

    m_compassRotationInProgress = false;
}

void MainWindow::loadDataBase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    // Ruta relativa dentro del proyecto
    QString path = QDir::currentPath() + "/navdb.sqlite";

    if (!QFile::exists(path)) {
        qDebug() << "No se encontró la base de datos en:" << path;
    }

    db.setDatabaseName(path);

    if (!db.open()) {
        qDebug() << "Error al abrir la base de datos:" << db.lastError();
    } else {
        qDebug() << "Base de datos cargada desde:" << path;
    }
}

void MainWindow::goToMapPage()
{
    goToPage(0);
    if (m_appTitle) m_appTitle->setText("POLICHART");
}

void MainWindow::goToPage(int index)
{
    if (!m_pages)return;
    if (index >= 0 && index < m_pages->count()){
        m_pages->setCurrentIndex(index);
    }
}

void MainWindow::showEditProfile()
{
    if (!m_currentUser) {
        m_currentUser = nav.findUser(m_currentUsername);
        if (!m_currentUser) {
            QMessageBox::warning(this, "Error", "No se pudo cargar el usuario actual.");
            return;
        }
    }

    EditProfileWindow editDialog(m_currentUser, this);

    if (editDialog.exec() == QDialog::Accepted && editDialog.wasModified()) {

        User* refreshedUser = nav.findUser(m_currentUsername);

        if (!refreshedUser) {
            QMessageBox::information(this, "Perfil Actualizado",
                                     "Has cambiado tu nombre de usuario.\n"
                                     "Por seguridad, inicia sesión con tus nuevas credenciales.");
            QApplication::quit();
            return;
        }

        m_currentUser = refreshedUser;

        m_usernameLabel->setText(m_currentUser->nickName());

        QImage avatar = m_currentUser->avatar();
        if (!avatar.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(avatar).scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap rounded(35, 35);
            rounded.fill(Qt::transparent);

            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(0, 0, 35, 35);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, pixmap);

            m_avatarLabel->setPixmap(rounded);
            m_avatarLabel->setStyleSheet("QLabel { border-radius: 17px; }");
        } else {
            QString initials = m_currentUsername.left(2).toUpper();
            m_avatarLabel->setText(initials);
            m_avatarLabel->setStyleSheet(
                "QLabel {"
                "   background-color: #3498DB;"
                "   color: white;"
                "   border-radius: 17px;"
                "   font-size: 14px;"
                "   font-weight: bold;"
                "}"
                );
        }

        if (m_historyWindow) {
            m_historyWindow->refreshData();
        }
    }
}

void MainWindow::onProblemAnswered(int problemIndex, bool correct)
{
    if (correct) {
        m_sessionHits++;
    } else {
        m_sessionFaults++;
    }
    // Debug output to verify it's working in the "Application Output" tab
    qDebug() << "Session Update -> Hits:" << m_sessionHits << " Faults:" << m_sessionFaults;
}

void MainWindow::showControlsHelp() {
    QString helpText =
        "<h3>Controles de POLICHART</h3>"

        "<h4>Modos de ratón:</h4>"
        "<ul>"
        "<li><b>Modo Move:</b> Podrás moverte por el mapa y mover las herramientas por el mapa</li>"
        "<li><b>Modo Clic:</b> Podrás utilizar todas las herramientas de dibujo en el mapa y girar las herramientas utilizando Shift+rueda del ratón</li>"
        "</ul>"

        "<h4>Métodos de dibujo:</h4>"
        "<ul>"
        "<li><b>Puntos:</b> Haz clic en el mapa para dibujarlo</li>"
        "<li><b>Rayas:</b> Haz clic en el mapa y vuelve a hacer clic donde quieras que termine la línea</li>"
        "<li><b>Texto:</b> Haz clic en el mapa y escribe el texto que quieras</li>"
        "<li><b>Arcos:</b> Utiliza el compás para dibujar arcos</li>"
        "<li><b>Goma borrar:</b> Haz clic en el dibujo que quieras eliminar. En el caso de los arcos puede ser que necesites eliminarlo por trozos</li>"
        "<li><b>Cambiar color:</b> Selecciona esta opción y elige el color que quieras</li>"
        "</ul>"

        "<h4>Herramientas de dibujo:</h4>"
        "<ul>"
        "<li><b>Coordenadas:</b> Muestra los puntos de corte con un click en el mapa</li>"
        "<li><b>Transportador:</b> Habilita el transportador y con este puedes medir ángulos</li>"
        "<li><b>Regla:</b> Habilita la regla, y con este puedes medir distancias</li>"
        "<li><b>Compás:</b> Aparece con tamaño fijo. Clic: define longitud de ambas patas. "
        "Rueda sobre cada pata: la gira. Shift+Rueda: pinta arco. Modo Move: mueve el compás completo.</li>"
        "Clic derecho: reinicia el tamaño del compás."
        "<li><b>Borrar:</b> Podrás borrar todos los trazos realizados reiniciando el mapa</li>"
        "</ul>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Información de Controles");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(helpText);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}
