#include "BinaryThresholdProcessor.h"
#include "AdaptiveThresholdProcessor.h"
#include "OtsuThresholdProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

/**
 * @brief Plugin entry point for Threshold processors.
 *
 * Implements IPluginFactory to provide Binary, Adaptive, and Otsu threshold processors.
 */
class ThresholdPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "threshold.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new BinaryThresholdProcessor(),
            new AdaptiveThresholdProcessor(),
            new OtsuThresholdProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.threshold.binary",
            "org.svw.threshold.adaptive",
            "org.svw.threshold.otsu"
        };
    }
};

#include "ThresholdPlugin.moc"
