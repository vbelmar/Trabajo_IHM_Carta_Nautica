#ifndef TRANSPORTADOR_H
#define TRANSPORTADOR_H

#include <QGraphicsSvgItem>
#include <QGraphicsSceneWheelEvent>
#include <QSizeF>

class Tool : public QGraphicsSvgItem
{
public:
    explicit Tool(const QString& svgResourcePath,
                  QGraphicsItem* parent = nullptr);

    // Escala uniforme de la regla a un tamaño objetivo (en píxeles de escena)
    void setToolSize(const QSizeF& sizePx);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    void applyInitialScale();
    void updateOrigin();

    QSizeF m_targetSizePx;
    double m_uniformScale = 10.0;
    double m_angleDeg     = 0.0;
};

#endif // TRANSPORTADOR_H

