#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>

class Application;

/**
 * @brief Persistent application settings backed by QSettings.
 */
namespace AppSettings {

enum class Theme {
    Dark,
    Light,
    Custom
};

enum class Language {
    English,
    Chinese
};

void initialize();

Language language();
void setLanguage(Language language);

Theme theme();
void setTheme(Theme theme);

QString customStylesheetPath();
void setCustomStylesheetPath(const QString& path);

QString customPluginDirectory();
void setCustomPluginDirectory(const QString& path);

QString customModelsDirectory();
void setCustomModelsDirectory(const QString& path);

int historyMaxItems();
void setHistoryMaxItems(int count);

int cameraWidth();
void setCameraWidth(int width);

int cameraHeight();
void setCameraHeight(int height);

int defaultCompareMode();
void setDefaultCompareMode(int mode);

void applyTheme(Application* app);

} // namespace AppSettings

#endif // APPSETTINGS_H
