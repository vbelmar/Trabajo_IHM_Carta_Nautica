#include "regla.h"
#include <QtMath>
#include <QApplication>
#include <QGraphicsScene>
#include <QSvgRenderer>


Tool::ToolRuler(const QString& svgResourcePath, QGraphicsItem* parent)
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

void Tool::setToolSizeRuler(const QSizeF& sizePx)
{
    m_targetSizePx = sizePx;
    applyInitialScale();
}

void Tool::applyInitialScaleRuler()
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

void Tool::updateOriginRuler()
{
    setTransformOriginPoint(boundingRect().center());
}

void Tool::wheelEventRuler(QGraphicsSceneWheelEvent *event)
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
}

