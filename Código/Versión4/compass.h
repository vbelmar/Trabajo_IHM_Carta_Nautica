#ifndef COMPASS_H
#define COMPASS_H

#include <QGraphicsSvgItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QSizeF>
#include <QPointF>

class Compass : public QGraphicsSvgItem
{

public:
    explicit Compass(const QString& svgResourcePath,
                     QGraphicsItem* parent = nullptr);

    void setSize(const QSizeF& sizePx);
    void setAngleDegrees(qreal angle);

    QPointF getPivotPoint() const;
    qreal getRadius() const { return m_currentRadius; }

signals:
    void angleChanged(qreal newAngle);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    void applyInitialScale();
    void updateOrigin();
    QPointF getLeftTipPosition() const;
    QPointF getRightTipPosition() const;

    QSizeF m_targetSizePx;
    qreal m_uniformScale;
    qreal m_baseWidth;
    qreal m_currentRadius;
    qreal m_angleDeg;

    QGraphicsPathItem *m_compassArc;
};

#endif // COMPASS_H
