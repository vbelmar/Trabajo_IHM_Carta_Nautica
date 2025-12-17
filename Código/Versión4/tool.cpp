#include "tool.h"
#include <QtMath>
#include <QApplication>
#include <QGraphicsScene>
#include <QSvgRenderer>

Tool::Tool(const QString& svgResourcePath, QGraphicsItem* parent)
    : QGraphicsSvgItem(svgResourcePath, parent)
{
    // Flags para poder moverla, seleccionarla - SIN ItemIgnoresTransformations
    setFlags(QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsGeometryChanges);

    // Origen de rotación = centro del SVG
    updateOrigin();

    // Tamaño inicial = tamaño natural del SVG
    m_targetSizePx = boundingRect().size();
    applyInitialScale();
}

void Tool::setToolSize(const QSizeF& sizePx)
{
    m_targetSizePx = sizePx;
    applyInitialScale();
}

void Tool::applyInitialScale()
{
    const QRectF br = boundingRect();
    if (br.isEmpty())
        return;

    const double sx = m_targetSizePx.width()  / br.width();
    const double sy = m_targetSizePx.height() / br.height();
    m_uniformScale = std::min(sx, sy);

    setScale(m_uniformScale);
    updateOrigin();

    if (scene())
        scene()->update();
}

void Tool::updateOrigin()
{
    setTransformOriginPoint(boundingRect().center());
}

void Tool::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        event->ignore();
        return;
    }
    double degrees = (event->delta() > 0) ? 5.0 : -5.0;
    // Posición del ratón en coordenadas locales
    QPointF clickPosLocal = mapFromScene(event->scenePos());
    // Guardamos la posición en escena del punto de pivote ANTES de cambiar nada
    QPointF oldPivotScene = mapToScene(clickPosLocal);
    // Cambiamos el origen de transformación al punto donde está el ratón
    setTransformOriginPoint(clickPosLocal);
    // Aplicamos la rotación incremental
    setRotation(rotation() + degrees);
    // Calculamos la nueva posición en escena del punto de pivote DESPUÉS de rotar
    QPointF newPivotScene = mapToScene(clickPosLocal);
    // Compensamos la posición del ítem para que el punto de pivote quede FIJO en la escena
    setPos(pos() + (oldPivotScene - newPivotScene));
    m_angleDeg += degrees;
    event->accept();
}

/*void Tool::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // Solo rotar si está pulsado Shift
    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        event->ignore();
        return;
    }

    const int delta = event->delta(); // Qt5: 120 por "clic" de rueda
    if (delta == 0) {
        event->ignore();
        return;
    }

    // Rotación suave
    double deltaDegrees = (delta / 8.0) * 0.1; // ≈ 1.5° por "clic"
    m_angleDeg += deltaDegrees;
    setRotation(m_angleDeg);

    if (scene())
        scene()->update();

    event->accept();
}*/
