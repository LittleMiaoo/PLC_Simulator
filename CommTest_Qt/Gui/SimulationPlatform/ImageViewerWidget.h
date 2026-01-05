/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPointF>

/**
 * 20251225 wm 从SimulationPlatform.cpp拆分
 * @brief 图像查看器控件
 *
 * 支持以下功能：
 * - 鼠标滚轮缩放
 * - 鼠标拖拽平移
 * - 双击恢复完整显示
 * - 灰白棋盘格背景
 * - 窗口缩放时保持显示比例
 */
class ImageViewerWidget : public QWidget
{
    Q_OBJECT

signals:
    /**
     * @brief 缩放比例变化信号
     * @param scale 新的缩放比例
     */
    void scaleChanged(double scale);

public:
    explicit ImageViewerWidget(QWidget* parent = nullptr);
    ~ImageViewerWidget() override = default;

    /**
     * @brief 设置要显示的图像
     * @param image 图像对象
     */
    void setImage(const QImage& image);

    /**
     * @brief 使图像适应窗口大小（完整显示）
     */
    void fitToWindow();

    /**
     * @brief 检查是否已加载图像
     * @return 如果已加载图像返回 true
     */
    bool hasImage() const;

    /**
     * @brief 获取当前图像
     * @return 当前图像的常量引用
     */
    const QImage& image() const;

    /**
     * @brief 获取当前缩放比例
     * @return 缩放比例
     */
    double scale() const;

    /**
     * @brief 设置缩放比例
     * @param scale 新的缩放比例
     */
    void setScale(double scale);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief 绘制灰白棋盘格背景
     * @param painter 绘图对象
     */
    void drawCheckerboard(QPainter& painter);

private:
    QImage m_image;           ///< 当前显示的图像
    double m_scale;           ///< 当前缩放比例
    double m_minScale;        ///< 最小缩放比例
    double m_maxScale;        ///< 最大缩放比例
    QPointF m_offset;         ///< 图像偏移量
    QPointF m_lastMousePos;   ///< 上次鼠标位置
    bool m_dragging;          ///< 是否正在拖拽
};

#endif // IMAGEVIEWERWIDGET_H
