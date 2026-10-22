/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QString>

class QApplication;

enum class AppLanguage {
    Chinese,
    English,
};

QString languageToCode(AppLanguage lang);
AppLanguage languageFromCode(const QString& code);

// 启动语言:有语言配置用配置;无语言配置时老用户(已有其他配置)用中文,全新安装中文系统用中文、其他用英文
AppLanguage resolveStartupLanguage();

AppLanguage currentLanguage();

void applyLanguage(QApplication& app, AppLanguage lang);
void switchLanguage(QApplication& app, AppLanguage lang);

#endif // LANGUAGEMANAGER_H
