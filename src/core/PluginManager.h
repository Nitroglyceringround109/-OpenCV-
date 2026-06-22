#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "IImageProcessor.h"
#include "IPluginFactory.h"

class QPluginLoader;

/**
 * @brief Scans plugin directories, loads shared libraries via QPluginLoader,
 *        and maintains a registry of available processors.
 *
 * This is the runtime plugin discovery and loading system.
 * It demonstrates Qt's meta-object system and plugin architecture.
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();

    /// Scan the given directory for plugin DLLs and load them.
    void scanPlugins(const QString& pluginDir);

    /// Create a new processor instance by ID.
    /// Returns nullptr if the ID is not found.
    /// The caller takes ownership of the returned pointer.
    IImageProcessor* createProcessor(const QString& processorId);

    /// Get metadata for all discovered processors (for menu population)
    QList<PluginMetadata> availableProcessors() const;

    /// Get metadata for processors in a specific category
    QList<PluginMetadata> processorsByCategory(const QString& category) const;

    /// Get all unique categories
    QStringList categories() const;

    /// Check if any plugins are loaded
    bool hasPlugins() const;

    /// Unload all plugins
    void unloadAll();

private:
    struct PluginInfo {
        QString filePath;
        QStringList processorIds;
        QList<PluginMetadata> metadataList;
    };

    QMap<QString, PluginInfo> m_plugins;  ///< filePath -> PluginInfo
    QMap<QString, QString> m_processorToPlugin;  ///< processorId -> pluginFilePath
    QMap<QString, PluginMetadata> m_metadata;  ///< processorId -> metadata
    QMap<QString, QPluginLoader*> m_loaders;  ///< Keep DLLs loaded for app lifetime
};

#endif // PLUGINMANAGER_H
