#ifndef APPPATHS_H
#define APPPATHS_H

#include <QString>

/**
 * @brief Resolves runtime paths for plugins, models, and other bundled assets.
 */
namespace AppPaths {

QString applicationDirectory();
QString pluginDirectory();
QString modelsDirectory();
QString resolvePluginDirectory();
QString resolveModelsDirectory();

} // namespace AppPaths

#endif // APPPATHS_H
