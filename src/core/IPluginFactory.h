#ifndef IPLUGINFACTORY_H
#define IPLUGINFACTORY_H

#include <QObject>
#include <QList>
#include "IImageProcessor.h"

/**
 * @brief Factory interface for plugins that provide multiple processors.
 *
 * Each plugin shared library implements this interface via QPluginLoader.
 * The host calls createProcessors() to enumerate all available processors
 * from a single DLL.
 */
class IPluginFactory
{
public:
    virtual ~IPluginFactory() = default;

    /// Create and return all processor instances this plugin provides.
    /// The caller takes ownership of the returned pointers.
    virtual QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) = 0;

    /// Return the IDs of all processors this plugin can create.
    virtual QStringList processorIds() const = 0;
};

#define IPluginFactory_IID "org.svw.IPluginFactory"

Q_DECLARE_INTERFACE(IPluginFactory, IPluginFactory_IID)

#endif // IPLUGINFACTORY_H
