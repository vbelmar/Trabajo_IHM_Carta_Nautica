#include "Compass.h"
#include <QtMath>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>   // <- PARA evitar "incomplete type"
#include <QWidget>         // <- window()
#include <QMetaObject>     // <- invokeMethod

Compass::Compass(const QString& svgResourcePath, QGraphicsItem* parent)
    : QGraphicsSvgItem(svgResourcePath, parent)
    , m_uniformScale(1.0)
    , m_baseWidth(100.0)
    , m_currentRadius(50.0)
    , m_angleDeg(0.0)
{
    setFlags(QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsGeometryChanges);

    updateOrigin();
    m_targetSizePx = boundingRect().size();
    m_baseWidth = boundingRect().width();
    applyInitialScale();

    setAcceptHoverEvents(true);
}

void Compass::setSize(const QSizeF& sizePx)
{
    m_targetSizePx = sizePx;
    applyInitialScale();
}

void Compass::setAngleDegrees(qreal angle)
{
    m_angleDeg = angle;
    setRotation(m_angleDeg);

}

void Compass::applyInitialScale()
{
    const QRectF br = boundingRect();
    if (br.isEmpty())
        return;

    const double sx = m_targetSizePx.width()  / br.width();
    const double sy = m_targetSizePx.height() / br.height();
    m_uniformScale = std::min(sx, sy);

    setScale(m_uniformScale);
    m_currentRadius = m_targetSizePx.width();
    updateOrigin();

    if (scene())
        scene()->update();
}

void Compass::updateOrigin()
{
    QRectF br = boundingRect();

    // Pivote exactamente en el extremo izquierdo del SVG
    setTransformOriginPoint(br.left(), br.center().y());
}


QPointF Compass::getLeftTipPosition() const
{
    // Punta izquierda (fija)
    QRectF br = boundingRect();
    QPointF localLeft(br.left() + br.width() * 0.1, br.center().y());
    return mapToScene(localLeft);
}

QPointF Compass::getRightTipPosition() const
{
    // Punta derecha (móvil)
    QRectF br = boundingRect();
    QPointF localRight(br.left() + br.width() * 0.9, br.center().y());
    return mapToScene(localRight);
}

QPointF Compass::getPivotPoint() const
{
    QRectF b = boundingRect();
    return QPointF(b.left() + 5, b.center().y());
}

void Compass::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // Girar la pata sobre la que está el cursor
    double delta = (event->delta() > 0) ? 5.0 : -5.0;
    m_angleDeg += delta;
    setRotation(m_angleDeg);

    event->accept();
}


