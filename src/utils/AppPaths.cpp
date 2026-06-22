#include "AppPaths.h"
#include "AppSettings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace AppPaths {

QString applicationDirectory()
{
    return QCoreApplication::applicationDirPath();
}

QString pluginDirectory()
{
    return resolvePluginDirectory();
}

QString modelsDirectory()
{
    return resolveModelsDirectory();
}

QString resolvePluginDirectory()
{
    const QString custom = AppSettings::customPluginDirectory();
    if (!custom.isEmpty()) {
        QDir dir(custom);
        if (dir.exists())
            return dir.absolutePath();
    }

    const QString appDir = applicationDirectory();
    QStringList candidates = {
        appDir + QStringLiteral("/plugins"),
        QDir(appDir).filePath(QStringLiteral("../plugins")),
    };

#ifdef PLUGIN_DIR
    candidates << QStringLiteral(PLUGIN_DIR);
#endif

    for (const QString& path : candidates) {
        QDir dir(path);
        if (dir.exists())
            return dir.absolutePath();
    }

    return appDir + QStringLiteral("/plugins");
}

QString resolveModelsDirectory()
{
    const QString custom = AppSettings::customModelsDirectory();
    if (!custom.isEmpty()) {
        QDir dir(custom);
        if (dir.exists())
            return dir.absolutePath();
    }

    const QString appDir = applicationDirectory();
    QStringList candidates = {
        appDir + QStringLiteral("/models"),
        QDir(appDir).filePath(QStringLiteral("../resources/models")),
    };

#ifdef SVW_MODELS_DIR
    candidates << QStringLiteral(SVW_MODELS_DIR);
#endif

    for (const QString& path : candidates) {
        QDir dir(path);
        if (dir.exists())
            return dir.absolutePath();
    }

    return appDir + QStringLiteral("/models");
}

} // namespace AppPaths
