#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QGraphicsItem>
#include <QLabel>
#include <QSlider>
#include <QToolBar>
#include <QStackedWidget>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QFrame>
#include <QTableWidget>
#include <QTimer>
#include <QColor>
#include <QStackedWidget>
#include "tool.h"      // Clase Tool para transportador y regla
#include "compass.h"   // Clase Compass para el compás
#include "navigation.h"
#include "navtypes.h"
#include "question_manager.h"
#include "editprofilewindow.h"
#include "historywindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &username, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Modos de ratón
    void setMoveMode();
    void setClickMode();

    // Herramientas de dibujo
    void togglePointMode();
    void toggleLineMode();
    void toggleTextMode();
    void toggleProtractorMode();
    void toggleRulerMode();
    void toggleRubberMode();
    void toggleCompassMode();
    void toggleCoordinateMode();
    void showControlsHelp();

    // Otras acciones
    void selectColor();
    void deleteCanvas();

    // Zoom y tamaño
    void onZoomSliderChanged(int value);
    void onSizeSliderChanged(int value);

    // Menú lateral y preguntas

    void logout();
    void showHistory();
    void showEditProfile();

    // Compás
    void drawCompassFinalArc();

    // Navegación por páginas
    void goToMapPage();
    void goToPage(int index);
    void onProblemAnswered(int problemIndex, bool correct);

private:
    Ui::MainWindow *ui;
    //COMPONENTES DIBUJO------------------
    QGraphicsScene *scene;
    QGraphicsView *view;

    // Zoom
    qreal currentZoom;
    const qreal minZoom = 0.1;
    const qreal maxZoom = 3.0;

    // Estados de herramientas
    bool m_drawPointMode = false;
    bool m_drawLineMode = false;
    bool m_drawTextMode = false;
    bool m_protractorMode = false;
    bool m_rulerMode = false;
    bool m_rubberMode = false;
    bool m_compassMode = false;
    bool m_coordinateMode = false;
    bool m_compassPivotSet = false; //PARA EL NUEVO COMPAS

    // Variables de dibujo
    QPointF m_lineStart;
    QGraphicsLineItem *m_currentLineItem = nullptr;
    QGraphicsLineItem *m_currentLineItemBorder = nullptr;
    bool m_drawingLine = false;

    // Listas de elementos dibujados
    QList<QGraphicsLineItem*>   m_drawnLines;
    QList<QGraphicsEllipseItem*> m_drawnPoints;
    QList<QGraphicsItem*>       m_drawnTexts;
    QList<QGraphicsPathItem*>   m_compassArcs;

    // Herramientas
    Tool *m_protractor = nullptr;
    Tool *m_ruler = nullptr;
    Compass *m_compass;
    Compass *m_compassFixedLeg;
    QPointF m_compassPivotPoint;

    // Posiciones y escalas
    QPointF m_protractorScenePos;
    QPointF m_rulerScenePos;
    QPointF m_compassScenePos;
    qreal m_protractorBaseScale = 1000.0;
    qreal m_rulerBaseScale = 1000.0;

    // Compás
    QTimer *m_compassStopTimer = nullptr;
    bool m_compassRotationInProgress = false;
    qreal m_compassStartAngle = 0.0;
    qreal m_compassRadius = 0.0;
    bool m_compassHasRadius = false;
    QGraphicsPathItem *m_compassArc = nullptr;

    // Color y tamaño
    QColor m_drawingColor = Qt::red;
    int m_drawingSize = 8;

    // Widgets de interfaz
    QSlider *m_sizeSlider = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QGraphicsTextItem *m_coordLabel = nullptr;

    //COMPONENTES INTERFAZ
    // Header
    QWidget *m_headerWidget = nullptr;
    QLabel *m_avatarLabel = nullptr;
    QLabel *m_usernameLabel = nullptr;
    QLabel *m_appTitle = nullptr;
    QPushButton *m_historyButton = nullptr;
    QPushButton *m_logoutButton = nullptr;

    // Selección de problemas
    QWidget *m_problemSelectionPage = nullptr;
    QPushButton *m_randomModeButton = nullptr;
    QPushButton *m_listModeButton = nullptr;
    QTableWidget *m_problemTable = nullptr;
    QPushButton *m_backButton = nullptr;

    // Toolbar del mapa
    QToolBar *m_mapToolbar = nullptr;

    // Cambio página
    QString m_currentUsername;
    QStackedWidget *m_pages;
    QWidget *m_mapPage;
    QWidget *m_historyPage;

    // Gestor de preguntas
    QuestionManager *m_questionManager;

    //Base de datos
    Navigation &nav;
    User *m_currentUser;
    HistoryWindow *m_historyWindow;
    int m_sessionHits = 0;
    int m_sessionFaults = 0;

    // ==================== MÉTODOS PRIVADOS ====================
    void loadStyleSheet();
    void setupHeader();
    void setupMapToolbar();

    // Métodos auxiliares de dibujo
    void disableAllTools();
    void updateZoom(qreal newZoom);
    void updateProtractorPosition();


    // Base de datos
    void loadDataBase();
};

#endif // MAINWINDOW_H
