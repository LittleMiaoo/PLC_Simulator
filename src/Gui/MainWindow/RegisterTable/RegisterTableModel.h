/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef REGISTERTABLEMODEL_H
#define REGISTERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include <QElapsedTimer>
#include <QColor>
#include <QPersistentModelIndex>
#include <vector>

#include "Core/DataTypeConvert.h"
#include "RegisterTableTypes.h"

class RegisterStore;

// 寄存器表数据模型:承载数据访问、校验/写回、显示格式化与闪烁状态。
// 文本按需经 data() 计算;store 变更只对变化格 emit dataChanged(定向刷新);
// 编辑保护 = 对正在编辑的 index 不发 dataChanged(真实 index 由委托上报)。
class RegisterTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // store: 数据源; dialogParent: 校验弹框父窗口
    explicit RegisterTableModel(RegisterStore* store, QWidget* dialogParent, QObject* parent = nullptr);

    // 设置维度(地址/值成对,colCount 须偶数);分配快照与闪烁戳并整表重置
    void setDimensions(int rowCount, int colCount);

    // QAbstractTableModel 覆写
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    // 视图状态 setter(由控制器在对应控件变化时转发;静默刷新,不闪)
    void setDataType(RegisterDataType type);
    void setStartAddr(int startAddr);
    void setNumberBase(bool hex);

    // 闪烁叠加色(供 RegisterItemDelegate::paint 调用):未闪/已过期返回无效 QColor
    QColor flashOverlay(int row, int col) const;

    // 编辑 index 跟踪(由委托 createEditor/destroyEditor 调用):刷新时对其不发 dataChanged
    void setEditingIndex(const QModelIndex& index);
    void clearEditingIndex();
    bool isEditing() const { return m_editIndex.isValid(); }  // 编辑中(供 resize 重置避让)

    void refreshHeaders();

signals:
    // 闪烁定时器节拍:控制器据此 viewport()->update() 触发重绘
    void flashTick();
    // 编辑结束(委托 destroyEditor→clearEditingIndex):控制器据此补齐编辑期被跳过的维度自适应
    void editingFinished();

private slots:
    void onStoreChanged();   // store dataChanged → 差异化刷新 + 闪烁
    void onFlashTick();      // 重绘节拍:发 flashTick、清过期、无活跃则停表

private:
    void refreshSnapshot();                                   // 从 store 重读窗口入 m_registerVals
    void refreshAll();                                        // 静默全表刷新(改类型/地址/进制)
    QString formatCell(RegisterDataType type, int k) const;  // 按类型格式化第 k 个寄存器
    void writeCell(RegisterDataType type, int k, const QString& text);  // 解析文本写回缓存联合体
    int registersPerValue(RegisterDataType type) const;      // 每值占几个 Int16
    void stampFlash(int row, int col);                       // 打闪烁时间戳并懒启动定时器
    int flashIndex(int row, int col) const;                  // (row,col)→m_flashStartMs 线性下标;越界/无列返回 -1
    // 值锚点格判定(data/flags/onStoreChanged 共用):值列 && 在快照范围内 && 落在锚点(k%rpv==0)
    bool isAnchorValueCell(int row, int col) const;

    RegisterStore* m_store;           // 数据源
    QWidget* m_dialogParent;          // 校验弹框父窗口
    RegisterCellLayout m_layout;      // cell<->寄存器映射(单一真相源,含维度)

    RegisterDataType m_currentType;   // 当前数据类型
    int m_startAddr;                  // 起始地址
    int m_intStat;                    // 整数显示状态 0=十进制, 1=十六进制
    std::vector<DataTypeConvert> m_registerVals;  // 窗口快照(差异基线 + data() 取数源)
    std::vector<DataTypeConvert> m_prevVals;      // onStoreChanged 差异基线的复用缓冲(避免每拍堆分配)

    std::vector<qint64> m_flashStartMs;  // 每格闪起时刻(m_clock 毫秒,0=不闪);索引 row*colCount+col
    QElapsedTimer m_clock;               // 单调时钟
    QTimer* m_flashTimer;                // 单个共享重绘定时器(父=this)
    QColor m_flashColor;                 // 缓存 @flashBg,随 themeChanged 刷新

    QPersistentModelIndex m_editIndex;   // 正在编辑的格(无效=无);刷新时对其不发 dataChanged
};

#endif // REGISTERTABLEMODEL_H
