#include "AppSettings.h"
#include "app/Application.h"
#include <QSettings>
#include <QFile>

namespace AppSettings {

namespace {
constexpr const char* kOrg = "SmartVisionWorkbench";
constexpr const char* kApp = "Smart Vision Workbench";
}

void initialize()
{
    QSettings settings(kOrg, kApp);
    if (!settings.contains(QStringLiteral("ui/theme")))
        settings.setValue(QStringLiteral("ui/theme"), static_cast<int>(Theme::Dark));
    if (!settings.contains(QStringLiteral("history/maxItems")))
        settings.setValue(QStringLiteral("history/maxItems"), 50);
    if (!settings.contains(QStringLiteral("camera/width")))
        settings.setValue(QStringLiteral("camera/width"), 1280);
    if (!settings.contains(QStringLiteral("camera/height")))
        settings.setValue(QStringLiteral("camera/height"), 720);
    if (!settings.contains(QStringLiteral("view/compareMode")))
        settings.setValue(QStringLiteral("view/compareMode"), 0);
    if (!settings.contains(QStringLiteral("ui/language")))
        settings.setValue(QStringLiteral("ui/language"),
                         static_cast<int>(Language::English));
}

Language language()
{
    QSettings settings(kOrg, kApp);
    return static_cast<Language>(settings.value(QStringLiteral("ui/language"),
                                                  static_cast<int>(Language::English)).toInt());
}

void setLanguage(Language value)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("ui/language"), static_cast<int>(value));
}

Theme theme()
{
    QSettings settings(kOrg, kApp);
    return static_cast<Theme>(settings.value(QStringLiteral("ui/theme"),
                                             static_cast<int>(Theme::Dark)).toInt());
}

void setTheme(Theme value)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("ui/theme"), static_cast<int>(value));
}

QString customStylesheetPath()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("ui/stylesheet")).toString();
}

void setCustomStylesheetPath(const QString& path)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("ui/stylesheet"), path);
}

QString customPluginDirectory()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("plugins/directory")).toString();
}

void setCustomPluginDirectory(const QString& path)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("plugins/directory"), path);
}

QString customModelsDirectory()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("models/directory")).toString();
}

void setCustomModelsDirectory(const QString& path)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("models/directory"), path);
}

int historyMaxItems()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("history/maxItems"), 50).toInt();
}

void setHistoryMaxItems(int count)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("history/maxItems"), count);
}

int cameraWidth()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("camera/width"), 1280).toInt();
}

void setCameraWidth(int width)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("camera/width"), width);
}

int cameraHeight()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("camera/height"), 720).toInt();
}

void setCameraHeight(int height)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("camera/height"), height);
}

int defaultCompareMode()
{
    QSettings settings(kOrg, kApp);
    return settings.value(QStringLiteral("view/compareMode"), 0).toInt();
}

void setDefaultCompareMode(int mode)
{
    QSettings settings(kOrg, kApp);
    settings.setValue(QStringLiteral("view/compareMode"), mode);
}

void applyTheme(Application* app)
{
    if (!app)
        return;

    switch (theme()) {
    case Theme::Light:
        app->loadLightTheme();
        break;
    case Theme::Custom: {
        const QString path = customStylesheetPath();
        if (!path.isEmpty() && QFile::exists(path))
            app->loadStylesheet(path);
        else
            app->loadDefaultTheme();
        break;
    }
    case Theme::Dark:
    default:
        app->loadDefaultTheme();
        break;
    }
}

} // namespace AppSettings
