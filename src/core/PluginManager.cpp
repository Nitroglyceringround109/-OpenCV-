#include "PluginManager.h"
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    unloadAll();
}

void PluginManager::scanPlugins(const QString& pluginDir)
{
    QDir dir(pluginDir);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << pluginDir;
        return;
    }

    const QStringList filters =
#ifdef Q_OS_WIN
        QStringList("*.dll");
#elif defined(Q_OS_MAC)
        QStringList("*.dylib");
#else
        QStringList("*.so");
#endif

    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fi : files) {
        QString filePath = fi.absoluteFilePath();

        if (m_plugins.contains(filePath))
            continue;

        auto* loader = new QPluginLoader(filePath, this);
        QObject* pluginObj = loader->instance();
        if (!pluginObj) {
            qWarning() << "Failed to load plugin:" << filePath
                       << "\nError:" << loader->errorString();
            loader->deleteLater();
            continue;
        }

        IPluginFactory* factory = qobject_cast<IPluginFactory*>(pluginObj);
        if (!factory) {
            IImageProcessor* singleProc = qobject_cast<IImageProcessor*>(pluginObj);
            if (singleProc) {
                PluginInfo info;
                info.filePath = filePath;
                QString procId = singleProc->metadata().id;
                info.processorIds << procId;
                info.metadataList << singleProc->metadata();
                m_plugins[filePath] = info;
                m_processorToPlugin[procId] = filePath;
                m_metadata[procId] = singleProc->metadata();
                m_loaders[filePath] = loader;
                qDebug() << "Loaded single-processor plugin:" << procId;
            } else {
                qWarning() << "Plugin does not implement IPluginFactory or IImageProcessor:"
                           << filePath;
                loader->unload();
                loader->deleteLater();
            }
            continue;
        }

        PluginInfo info;
        info.filePath = filePath;
        info.processorIds = factory->processorIds();

        QList<IImageProcessor*> tempProcs = factory->createProcessors();
        for (IImageProcessor* proc : tempProcs) {
            PluginMetadata meta = proc->metadata();
            info.metadataList << meta;
            m_processorToPlugin[meta.id] = filePath;
            m_metadata[meta.id] = meta;
            delete proc;
        }

        m_plugins[filePath] = info;
        m_loaders[filePath] = loader;
        qDebug() << "Loaded plugin:" << filePath
                 << "with" << info.processorIds.size() << "processors";
    }
}

IImageProcessor* PluginManager::createProcessor(const QString& processorId)
{
    if (!m_processorToPlugin.contains(processorId))
        return nullptr;

    QString filePath = m_processorToPlugin[processorId];
    QPluginLoader* loader = m_loaders.value(filePath, nullptr);
    if (!loader)
        return nullptr;

    QObject* pluginObj = loader->instance();
    if (!pluginObj)
        return nullptr;

    IPluginFactory* factory = qobject_cast<IPluginFactory*>(pluginObj);
    if (factory) {
        QList<IImageProcessor*> procs = factory->createProcessors();
        for (IImageProcessor* proc : procs) {
            if (proc->metadata().id == processorId)
                return proc;
            delete proc;
        }
    }

    IImageProcessor* singleProc = qobject_cast<IImageProcessor*>(pluginObj);
    if (singleProc && singleProc->metadata().id == processorId)
        return singleProc;

    return nullptr;
}

QList<PluginMetadata> PluginManager::availableProcessors() const
{
    return m_metadata.values();
}

QList<PluginMetadata> PluginManager::processorsByCategory(const QString& category) const
{
    QList<PluginMetadata> result;
    for (const auto& meta : m_metadata) {
        if (meta.category == category)
            result << meta;
    }
    return result;
}

QStringList PluginManager::categories() const
{
    QSet<QString> cats;
    for (const auto& meta : m_metadata)
        cats << meta.category;
    QStringList result = cats.values();
    result.sort();
    return result;
}

bool PluginManager::hasPlugins() const
{
    return !m_plugins.isEmpty();
}

void PluginManager::unloadAll()
{
    for (QPluginLoader* loader : m_loaders)
        loader->unload();
    m_loaders.clear();
    m_plugins.clear();
    m_processorToPlugin.clear();
    m_metadata.clear();
}
