#include "CanvasWidget.h"
#include "SimulationPlatform.h"

CanvasWidget::CanvasWidget(SimulationPlatform* parent)
    : QWidget(parent)
    , m_parent(parent)
{
    setStyleSheet("background-color: #FFFFFF; border: 1px solid #CCCCCC;");
}

void CanvasWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置背景为白色
    painter.fillRect(rect(), Qt::white);

    // 调用父类的绘制方法
    if (m_parent) {
        m_parent->drawCanvas(painter);
    }

    QWidget::paintEvent(event);
}
