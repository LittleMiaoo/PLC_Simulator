#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QPainter>

// 前置声明
class SimulationPlatform;

/**
 * 20251225 wm 从SimulationPlatform.cpp拆分
 * @brief 画布控件
 *
 * 用于绘制模拟平台的坐标系、平台和标记点。
 * 通过持有父类 SimulationPlatform 的指针来调用其绘制方法。
 */
class CanvasWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件，必须是 SimulationPlatform 类型
     */
    explicit CanvasWidget(SimulationPlatform* parent);
    ~CanvasWidget() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    SimulationPlatform* m_parent;  ///< 父控件指针
};

#endif // CANVASWIDGET_H
