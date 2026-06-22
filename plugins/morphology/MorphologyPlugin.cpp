#include "ErodeProcessor.h"
#include "DilateProcessor.h"
#include "OpenCloseProcessor.h"
#include "GradientProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

class MorphologyPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "morphology.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new ErodeProcessor(),
            new DilateProcessor(),
            new OpenCloseProcessor(),
            new GradientProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.morphology.erode",
            "org.svw.morphology.dilate",
            "org.svw.morphology.openclose",
            "org.svw.morphology.gradient"
        };
    }
};

#include "MorphologyPlugin.moc"
