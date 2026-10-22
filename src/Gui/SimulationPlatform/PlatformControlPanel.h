/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef PLATFORMCONTROLPANEL_H
#define PLATFORMCONTROLPANEL_H

#include <QWidget>
#include "Core/PlatformTypes.h"

class PlatformScene;
class CollapsibleGroupBox;
class QLineEdit;
class QCheckBox;
class QLabel;

/**
 * @brief 模拟平台右侧控制面板:5 组可折叠控件,读写 PlatformScene
 */
class PlatformControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PlatformControlPanel(PlatformScene* scene, QWidget* parent = nullptr);

    CollapsibleGroupBox* baseGroup() const { return m_baseGroup; }
    CollapsibleGroupBox* liveGroup() const { return m_liveGroup; }
    CollapsibleGroupBox* baseMarkGroup() const { return m_baseMarkGroup; }
    CollapsibleGroupBox* liveMarkGroup() const { return m_liveMarkGroup; }
    CollapsibleGroupBox* virtualMarkGroup() const { return m_virtualMarkGroup; }

private slots:
    void pushBaseToScene();
    void pushLiveToScene();
    void pushBaseMarkToScene();
    void pushLiveMarkToScene();
    void pushVirtualMarkToScene();
    void refreshFromScene();   // scene.changed → 回填输入框(blockSignals 防抖)

protected:
    void changeEvent(QEvent* event) override;

private:
    void buildUi();
    void retranslateUi();      // 可翻译文案唯一设置处(构建末尾 + 语言切换共用)
    void wireConnections();
    void setupValidators();

    PlatformScene* m_scene;

    CollapsibleGroupBox* m_baseGroup = nullptr;
    CollapsibleGroupBox* m_liveGroup = nullptr;
    CollapsibleGroupBox* m_baseMarkGroup = nullptr;
    CollapsibleGroupBox* m_liveMarkGroup = nullptr;
    CollapsibleGroupBox* m_virtualMarkGroup = nullptr;

    QLineEdit* m_baseXEdit = nullptr;  QLineEdit* m_baseYEdit = nullptr;  QLineEdit* m_baseAngleEdit = nullptr;
    QCheckBox* m_showBaseCheck = nullptr;
    QLineEdit* m_liveXEdit = nullptr;  QLineEdit* m_liveYEdit = nullptr;  QLineEdit* m_liveAngleEdit = nullptr;
    QCheckBox* m_showLiveCheck = nullptr;
    QLineEdit* m_baseMarkXEdit = nullptr;  QLineEdit* m_baseMarkYEdit = nullptr;  QLineEdit* m_baseMarkAngleEdit = nullptr;
    QCheckBox* m_baseMarkFollowCheck = nullptr;  QCheckBox* m_showBaseMarkCheck = nullptr;
    QLineEdit* m_liveMarkXEdit = nullptr;  QLineEdit* m_liveMarkYEdit = nullptr;  QLineEdit* m_liveMarkAngleEdit = nullptr;
    QCheckBox* m_liveMarkFollowCheck = nullptr;  QCheckBox* m_showLiveMarkCheck = nullptr;
    QLineEdit* m_virtualMarkXEdit = nullptr;  QLineEdit* m_virtualMarkYEdit = nullptr;
    QCheckBox* m_showVirtualMarkCheck = nullptr;
    QLabel* m_virtualMarkXLabel = nullptr;  QLabel* m_virtualMarkYLabel = nullptr;
};

#endif // PLATFORMCONTROLPANEL_H
