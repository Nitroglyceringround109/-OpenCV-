#include "GaussianBlurProcessor.h"
#include "MedianBlurProcessor.h"
#include "BilateralFilterProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

class FilteringPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "filtering.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new GaussianBlurProcessor(),
            new MedianBlurProcessor(),
            new BilateralFilterProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.filtering.gaussian",
            "org.svw.filtering.median",
            "org.svw.filtering.bilateral"
        };
    }
};

#include "FilteringPlugin.moc"
