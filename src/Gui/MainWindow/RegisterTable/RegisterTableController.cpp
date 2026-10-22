/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "RegisterTableController.h"
#include "RegisterTableModel.h"
#include "RegisterItemDelegate.h"

#include <QTableView>
#include <QHeaderView>
#include <QEvent>
#include <algorithm>

RegisterTableController::RegisterTableController(QTableView* view, RegisterStore* store, QWidget* parent)
    : QObject(parent)
    , m_view(view)
    , m_store(store)
    , m_parentWidget(parent)
    , m_model(nullptr)
{
}

void RegisterTableController::initTable(int rowCount, int colCount)
{
    if (!m_view) return;

    m_model = new RegisterTableModel(m_store, m_parentWidget, this);
    m_model->setDimensions(rowCount, colCount);
    m_view->setModel(m_model);

    // 装配视图:数据/表头来自 model,此处只配外观与交互
    // 列拉伸填满视口(消除末尾死白);列宽浮动 [kCellWidth, ~2*kCellWidth),
    // kCellWidth 仅用于反推列数(见 applyViewportDimensions)
    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->setAlternatingRowColors(true);
    // 编辑触发沿用 QAbstractItemView 默认(双击/F2),与迁移前 QTableWidget 一致,无需显式设置
    m_view->setItemDelegate(new RegisterItemDelegate(m_model, m_view));

    // 闪烁节拍 → 重绘 viewport(model 不持 view,经信号解耦)
    connect(m_model, &RegisterTableModel::flashTick, this, [this]
    {
        m_view->viewport()->update();
    });

    // 监听视口 resize:行列随窗口尺寸自适应
    m_view->viewport()->installEventFilter(this);

    // 编辑结束后补齐编辑期被跳过的维度(Queued:待委托 destroyEditor 返回后再重置,避免重入)
    connect(m_model, &RegisterTableModel::editingFinished, this, [this]
    {
        applyViewportDimensions();
    }, Qt::QueuedConnection);
}

bool RegisterTableController::eventFilter(QObject* obj, QEvent* ev)
{
    if (m_view && obj == m_view->viewport() && ev->type() == QEvent::Resize)
        applyViewportDimensions();
    return QObject::eventFilter(obj, ev);
}

void RegisterTableController::applyViewportDimensions()
{
    if (!m_view || !m_model) return;

    const int rowHeight = m_view->verticalHeader()->defaultSectionSize();
    if (kCellWidth <= 0 || rowHeight <= 0) return;

    const QSize vp = m_view->viewport()->size();

    int cols = vp.width() / kCellWidth;
    cols -= cols % 2;                 // 取偶:地址列+值列成对
    cols = std::max(2, cols);
    const int rows = std::max(1, vp.height() / rowHeight);

    // 去抖:整数行列没变则不重置(只在跨越整格边界时才 beginResetModel)
    if (rows == m_model->rowCount() && cols == m_model->columnCount())
        return;

    // 编辑保护:编辑中重置会吞掉未提交输入,延到下次 resize 再应用
    if (m_model->isEditing())
        return;

    m_model->setDimensions(rows, cols);
}

void RegisterTableController::setDataType(RegisterDataType type)
{
    if (m_model) m_model->setDataType(type);
}

void RegisterTableController::setStartAddr(int startAddr)
{
    if (m_model) m_model->setStartAddr(startAddr);
}

void RegisterTableController::setNumberBase(bool hex)
{
    if (m_model) m_model->setNumberBase(hex);
}

void RegisterTableController::retranslateHeaders()
{
    if (m_model)
        m_model->refreshHeaders();
}
