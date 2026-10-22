/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef REGISTERTABLECONTROLLER_H
#define REGISTERTABLECONTROLLER_H

#include <QObject>

#include "RegisterTableTypes.h"

class QTableView;
class RegisterStore;
class RegisterTableModel;

// 寄存器表控制器:装配 QTableView + RegisterTableModel,并把 MainWindow 的视图状态
// 变更(数据类型/起始地址/进制)转发给 model。数据/校验/格式化/闪烁均在 model 内。
class RegisterTableController : public QObject
{
    Q_OBJECT

public:
    // view: 寄存器表视图; store: 寄存器数据存储; parent: 父对象(亦作弹框父窗口)
    explicit RegisterTableController(QTableView* view, RegisterStore* store, QWidget* parent = nullptr);

    ~RegisterTableController() override = default;

    // 初始化:建 model、setModel、装配视图(列宽/交替行色/委托/闪烁重绘)
    void initTable(int rowCount, int colCount);

    // 三个语义 setter:由 MainWindow 在对应控件变化时推入,内部转发 model(自动刷新)
    void setDataType(RegisterDataType type);
    void setStartAddr(int startAddr);
    void setNumberBase(bool hex);

    void retranslateHeaders();

protected:
    // 监听视图视口 resize → 按窗口尺寸重算行列
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    void applyViewportDimensions();   // 据视口尺寸算行列,变化且非编辑中才重置 model

    static constexpr int kCellWidth = 80;   // 单元格列宽(行列反推与列默认段宽共用)

    QTableView* m_view;             // 寄存器表视图
    RegisterStore* m_store;         // 寄存器数据存储
    QWidget* m_parentWidget;        // 父窗口(弹框父)
    RegisterTableModel* m_model;    // 数据模型(父=this)
};

#endif // REGISTERTABLECONTROLLER_H
