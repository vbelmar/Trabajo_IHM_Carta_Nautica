#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSlider>
#include <QMouseEvent>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , view(new QGraphicsView(this))
    , currentZoom(0.1)
    , m_drawLineMode(false)
    , m_protractorMode(false)
    , m_currentLineItem(nullptr)
    , m_drawingLine(false)
    , m_protractor(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Carta Nautica");

    view->setScene(scene);

    QPixmap pm(":/new/prefix1/resources/carta_nautica.jpg");
    QGraphicsPixmapItem *item = scene->addPixmap(pm);
    item->setZValue(0);

    view->scale(currentZoom, currentZoom);

    // Modo click por defecto
    view->setDragMode(QGraphicsView::NoDrag);

    //Modos del raton
    // Configurar los actions como checkables con QActionGroup
    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->addAction(ui->actionMoveMode);
    modeGroup->addAction(ui->actionClickMode);

    ui->actionMoveMode->setCheckable(true);
    ui->actionClickMode->setCheckable(true);
    ui->actionClickMode->setChecked(true);

    //Herramientas dibujo
    QActionGroup *toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(ui->actionDrawLine);
    toolsGroup->addAction(ui->actionProtractor);

    ui->actionDrawLine->setCheckable(true);
    ui->actionProtractor->setCheckable(true);

    // Permitir desmarcar todas las herramientas
    toolsGroup->setExclusive(false);  // Importante para poder desactivar todas

    //Codigo slider
    // Crear el slider de zoom por código
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(100);
    zoomSlider->setFixedWidth(300);

    // Calcular valor inicial del slider basado en currentZoom
    int initialValue = (std::log(currentZoom / minZoom) / std::log(maxZoom / minZoom)) * 100;
    zoomSlider->setValue(initialValue);

    // Layout horizontal para centrar el slider
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addStretch();
    sliderLayout->addWidget(zoomSlider);
    sliderLayout->addStretch();
    sliderLayout->setContentsMargins(0, 5, 0, 5);

    // Layout vertical principal: view arriba, slider abajo
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(view);
    mainLayout->addLayout(sliderLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Widget central con el layout
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Conectar modos raton
    connect(ui->actionMoveMode, &QAction::triggered, this, &MainWindow::setMoveMode);
    connect(ui->actionClickMode, &QAction::triggered, this, &MainWindow::setClickMode);

    //Conectar herramientas dibujo
    connect(ui->actionDrawLine, &QAction::triggered, this, &MainWindow::toggleLineMode);
    connect(ui->actionProtractor, &QAction::triggered, this, &MainWindow::toggleProtractorMode);

    // Conectar el slider
    connect(zoomSlider, &QSlider::valueChanged, this, &MainWindow::onZoomSliderChanged);

    // Instalar el event filter para capturar eventos del viewport
    view->viewport()->installEventFilter(this);

    // Crear el transportador una sola vez al inicio (invisible)
    m_protractor = new Tool(":/new/prefix1/resources/icons/transportador.svg");
    m_protractor->setToolSize(QSizeF(500, 500));
    m_protractor->setZValue(100);
    m_protractor->setVisible(false);
    m_protractor->setFlag(QGraphicsItem::ItemIsMovable, false);
    scene->addItem(m_protractor);

    // Guardar posición inicial en coordenadas de escena
    m_protractorScenePos = view->mapToScene(view->viewport()->rect().center());
    m_protractor->setPos(m_protractorScenePos);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setMoveMode()
{
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->actionMoveMode->setChecked(true);
    ui->actionClickMode->setChecked(false);
}

void MainWindow::setClickMode()
{
    view->setDragMode(QGraphicsView::NoDrag);
    ui->actionClickMode->setChecked(true);
    ui->actionMoveMode->setChecked(false);
}

void MainWindow::disableAllTools()
{
    m_drawLineMode = false;
    m_protractorMode = false;
    m_currentLineItem = nullptr;
    m_drawingLine = false;

    ui->actionDrawLine->setChecked(false);
    ui->actionProtractor->setChecked(false);
}

void MainWindow::toggleLineMode()
{
    if (m_drawLineMode) {
        // Si ya está activo, desactivar
        disableAllTools();
    } else {
        // Activar modo línea y desactivar otros
        disableAllTools();
        m_drawLineMode = true;
        ui->actionDrawLine->setChecked(true);
    }
}

void MainWindow::toggleProtractorMode()
{
    if (m_protractorMode) {
        // Si ya está activo, desactivar
        m_protractorMode = false;
        ui->actionProtractor->setChecked(false);

        // Hacer invisible y no movible
        if (m_protractor) {
            m_protractor->setVisible(false);
            m_protractor->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    } else {
        // Activar modo transportador
        m_protractorMode = true;
        ui->actionProtractor->setChecked(true);

        // Hacer visible y movible
        if (m_protractor) {
            m_protractor->setVisible(true);
            m_protractor->setFlag(QGraphicsItem::ItemIsMovable, true);

            // Actualizar posición al centro actual de la vista
            updateProtractorPosition();
        }
    }
}

void MainWindow::updateProtractorPosition()
{
    if (!m_protractor)
        return;

    // Solo actualizar si el transportador está visible
    if (m_protractor->isVisible()) {
        // Obtener el centro actual de la vista en coordenadas de escena
        QPointF centerView = view->mapToScene(view->viewport()->rect().center());
        m_protractorScenePos = centerView;
        m_protractor->setPos(m_protractorScenePos);
    }
}

// ===== EVENT FILTER PARA DIBUJAR LÍNEAS =====

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view->viewport()) {
        // Solo procesar eventos de dibujo de líneas si estamos en modo línea Y en modo click
        if (m_drawLineMode && ui->actionClickMode->isChecked()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (e->button() == Qt::LeftButton && !m_drawingLine) {
                    // Primer click: iniciar línea
                    QPointF scenePos = view->mapToScene(e->pos());
                    m_lineStart = scenePos;
                    QPen pen(Qt::red, 8);
                    m_currentLineItem = new QGraphicsLineItem();
                    m_currentLineItem->setZValue(10);
                    m_currentLineItem->setPen(pen);
                    m_currentLineItem->setLine(QLineF(m_lineStart, m_lineStart));
                    scene->addItem(m_currentLineItem);
                    m_drawingLine = true;
                    return true;
                }
                else if (e->button() == Qt::LeftButton && m_drawingLine) {
                    // Segundo click: finalizar línea
                    QPointF p2 = view->mapToScene(e->pos());
                    if (m_currentLineItem) {
                        m_currentLineItem->setLine(QLineF(m_lineStart, p2));
                    }
                    m_currentLineItem = nullptr;
                    m_drawingLine = false;
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseMove) {
                auto *e = static_cast<QMouseEvent*>(event);
                if (m_drawingLine && m_currentLineItem) {
                    // Actualizar la línea mientras se mueve el ratón
                    QPointF p2 = view->mapToScene(e->pos());
                    m_currentLineItem->setLine(QLineF(m_lineStart, p2));
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::onZoomSliderChanged(int value)
{
    qreal t = value / 100.0;
    qreal newZoom = minZoom * std::pow(maxZoom / minZoom, t);
    updateZoom(newZoom);
}

void MainWindow::updateZoom(qreal newZoom)
{
    qreal scaleFactor = newZoom / currentZoom;
    view->scale(scaleFactor, scaleFactor);
    currentZoom = newZoom;

    // Actualizar posición del transportador después del zoom
    updateProtractorPosition();
}
