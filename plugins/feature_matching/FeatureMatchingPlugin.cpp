#include "OrbMatcherProcessor.h"
#include "SiftMatcherProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

class FeatureMatchingPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "feature_matching.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new OrbMatcherProcessor(),
            new SiftMatcherProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.feature_matching.orb",
            "org.svw.feature_matching.sift"
        };
    }
};

#include "FeatureMatchingPlugin.moc"
