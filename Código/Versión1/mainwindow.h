#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include "Transportador.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    //Modos de raton
    void setMoveMode();
    void setClickMode();

    // Herramientas de dibujo
    void toggleLineMode();
    void toggleProtractorMode();

    void onZoomSliderChanged(int value);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;

    qreal currentZoom;
    const qreal maxZoom = 3.0;
    const qreal minZoom = 0.1;

    // Estados de las herramientas
    bool m_drawLineMode;
    bool m_protractorMode;

    // Para dibujar líneas
    QPointF m_lineStart;
    QGraphicsLineItem *m_currentLineItem;
    bool m_drawingLine;  // Para controlar si estamos dibujando una línea

    // Transportador
    Tool *m_protractor;
    QPointF m_protractorScenePos;  // Guardar posición en coordenadas de escena

    void updateZoom(qreal newZoom);
    void disableAllTools();  // Desactiva todas las herramientas
    void updateProtractorPosition();  // Actualizar posición después del zoom
};

#endif // MAINWINDOW_H
