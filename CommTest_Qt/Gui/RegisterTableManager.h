/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao <mao.wang.dev@foxmail.com>
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef REGISTERTABLEMANAGER_H
#define REGISTERTABLEMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <vector>
#include <cfloat>

#include "../MainFlow/MainWorkFlow.h"

// 表格常量定义
#define REGISTER_TABLE_COLUMN_COUNT 10
#define REGISTER_TABLE_ROW_COUNT    21

/**
 * 20251225 wm 从CommTest_Qt.cpp拆分
 * @brief 寄存器表格管理器
 *
 * 负责管理寄存器数据的显示、编辑和验证。
 * 从 CommTest_Qt 中拆分出来，专注于表格相关逻辑。
 */
class RegisterTableManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param tableWidget 寄存器表格控件
     * @param dataTypeCombo 数据类型选择下拉框
     * @param addrEdit 起始地址输入框
     * @param workFlow 工作流指针
     * @param parent 父对象
     */
    explicit RegisterTableManager(
        QTableWidget* tableWidget,
        QComboBox* dataTypeCombo,
        QLineEdit* addrEdit,
        MainWorkFlow* workFlow,
        QWidget* parent = nullptr
    );

    ~RegisterTableManager() override = default;

    /**
     * @brief 初始化寄存器表格
     */
    void initTable();

    /**
     * @brief 更新表格信息
     * @param nStart 起始地址
     * @param bInitialize 是否初始化
     */
    void updateTableInfo(int nStart, bool bInitialize = false);

    /**
     * @brief 从工作流获取寄存器数据
     * @param nStart 起始地址
     */
    void getRegisterVals(int nStart);

    /**
     * @brief 将数据写入工作流
     * @param nStart 起始地址
     */
    void setRegisterVals(int nStart);

    /**
     * @brief 更新单个单元格对应的寄存器值
     * @param pItem 表格项指针
     */
    void updateRegisterVals(QTableWidgetItem* pItem);

    /**
     * @brief 显示寄存器数据
     */
    void displayRegisterVals();

    /**
     * @brief 设置整数显示状态（十进制/十六进制）
     * @param stat 0=十进制, 1=十六进制
     */
    void setIntDisplayStat(int stat) { m_nIntStat = stat; }

    /**
     * @brief 获取整数显示状态
     * @return 0=十进制, 1=十六进制
     */
    int intDisplayStat() const { return m_nIntStat; }

    /**
     * @brief 设置是否允许闪烁效果
     * @param enable 是否启用
     */
    void setShouldFlash(bool enable) { m_bShouldFlash = enable; }

    /**
     * @brief 获取是否允许闪烁效果
     * @return 是否启用闪烁效果
     */
    bool shouldFlash() const { return m_bShouldFlash; }

    /**
     * @brief 获取寄存器数据缓存
     * @return 数据缓存引用
     */
    std::vector<DataTypeConvert>& registerValCache() { return m_vecRegisterVal; }

private:
    /**
     * @brief 检查输入合法性
     * @param pItem 表格项指针
     * @return 是否合法
     */
    bool checkInput(QTableWidgetItem* pItem);

    /**
     * @brief 检查字符串输入
     */
    bool checkInput_str(QTableWidgetItem* pItem, const QString& text);

    /**
     * @brief 检查整数输入
     */
    bool checkInput_int(QTableWidgetItem* pItem, const QString& text, int32_t minVal, int32_t maxVal);

    /**
     * @brief 检查浮点数输入
     */
    bool checkInput_float(QTableWidgetItem* pItem, const QString& text, double minVal, double maxVal);

    /**
     * @brief 检查十六进制整数输入
     */
    bool checkInput_int_Hex(QTableWidgetItem* pItem, const RegisterDataType& type);

    // 各类型数据显示方法
    void displayRegisterVals_Char8();
    void displayRegisterVals_Int16();
    void displayRegisterVals_Int32();
    void displayRegisterVals_Float();
    void displayRegisterVals_Double();

private:
    QTableWidget* m_tableWidget;      ///< 寄存器表格控件
    QComboBox* m_dataTypeCombo;       ///< 数据类型选择下拉框
    QLineEdit* m_addrEdit;            ///< 起始地址输入框
    MainWorkFlow* m_workFlow;         ///< 工作流指针
    QWidget* m_parentWidget;          ///< 父窗口（用于显示消息框）

    std::vector<DataTypeConvert> m_vecRegisterVal;  ///< 寄存器数据缓存
    int m_nIntStat;                   ///< 整数显示状态 0=十进制, 1=十六进制
    bool m_bShouldFlash;              ///< 是否允许闪烁效果
};

#endif // REGISTERTABLEMANAGER_H
