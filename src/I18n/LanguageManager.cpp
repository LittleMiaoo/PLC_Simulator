/*
 * PLC Simulator - Industrial Communication Protocol Testing Tool
 * Copyright (c) 2025-2026 Wang Mao
 *
 * This file is part of PLC Simulator.
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "LanguageManager.h"
#include "Config/ConfigStore.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

namespace {
QTranslator s_appTranslator;
QTranslator s_qtTranslator;
AppLanguage s_currentLanguage = AppLanguage::Chinese;
bool s_appTranslatorInstalled = false;
bool s_qtTranslatorInstalled = false;

void uninstallTranslators(QApplication& app)
{
    if (s_appTranslatorInstalled) {
        app.removeTranslator(&s_appTranslator);
        s_appTranslatorInstalled = false;
    }
    if (s_qtTranslatorInstalled) {
        app.removeTranslator(&s_qtTranslator);
        s_qtTranslatorInstalled = false;
    }
}

void installTranslatorsFor(QApplication& app, AppLanguage lang)
{
    uninstallTranslators(app);
    if (lang != AppLanguage::English)
        return;

    if (s_appTranslator.load(QStringLiteral(":/i18n/plc_simulator_en")))
    {
        app.installTranslator(&s_appTranslator);
        s_appTranslatorInstalled = true;
    }

    if (s_qtTranslator.load(QStringLiteral("qtbase_en"),
                            QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&s_qtTranslator);
        s_qtTranslatorInstalled = true;
    }
}
} // namespace

QString languageToCode(AppLanguage lang)
{
    return lang == AppLanguage::English ? QStringLiteral("en") : QStringLiteral("zh");
}

AppLanguage languageFromCode(const QString& code)
{
    if (code == QLatin1String("en"))
        return AppLanguage::English;
    return AppLanguage::Chinese;
}

AppLanguage resolveStartupLanguage()
{
    const ConfigStore store;   // 仅读取(ConfigStore 改一项写一次,析构不写盘)
    QString code;
    if (store.LoadLanguagePref(code))
        return languageFromCode(code);

    // 无语言配置:已有其他配置的老用户保持升级前的中文界面,不因系统语言突变;
    // 全新安装按系统语言——中文系统用中文,其他一律英文
    if (!store.IsEmpty())
        return AppLanguage::Chinese;
    return QLocale::system().language() == QLocale::Chinese
               ? AppLanguage::Chinese
               : AppLanguage::English;
}

AppLanguage currentLanguage()
{
    return s_currentLanguage;
}

void applyLanguage(QApplication& app, AppLanguage lang)
{
    s_currentLanguage = lang;
    installTranslatorsFor(app, lang);
}

void switchLanguage(QApplication& app, AppLanguage lang)
{
    if (lang == s_currentLanguage)
        return;

    s_currentLanguage = lang;
    // 安装/移除翻译器时 Qt 会向所有顶层窗口投递 LanguageChange 并逐级传给子控件,无需手动广播
    installTranslatorsFor(app, lang);
}
