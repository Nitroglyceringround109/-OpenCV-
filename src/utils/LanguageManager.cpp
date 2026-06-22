#include "LanguageManager.h"
#include "AppSettings.h"
#include "AppPaths.h"
#include <QApplication>
#include <QEvent>
#include <QTranslator>
#include <QWidget>

namespace LanguageManager {

namespace {

QTranslator s_appTranslator;

bool loadChineseTranslator()
{
    const QStringList candidates = {
        QStringLiteral(":/i18n/svw_zh_CN.qm"),
        AppPaths::applicationDirectory() + QStringLiteral("/translations/svw_zh_CN.qm"),
    };

    for (const QString& path : candidates) {
        if (s_appTranslator.load(path))
            return true;
    }
    return false;
}

} // namespace

void notifyLanguageChanged(QApplication* app)
{
    if (!app)
        return;

    QEvent event(QEvent::LanguageChange);
    for (QWidget* widget : app->topLevelWidgets())
        QApplication::sendEvent(widget, &event);
}

void install(QApplication* app, AppSettings::Language language)
{
    if (!app)
        return;

    app->removeTranslator(&s_appTranslator);

    if (language == AppSettings::Language::Chinese)
        loadChineseTranslator();

    if (!s_appTranslator.isEmpty())
        app->installTranslator(&s_appTranslator);

    notifyLanguageChanged(app);
}

} // namespace LanguageManager
