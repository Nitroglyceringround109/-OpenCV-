#include "GrayscaleProcessor.h"
#include "HsvConvertProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

/**
 * @brief Plugin entry point for Color Conversion processors.
 *
 * Implements IPluginFactory to provide Grayscale and HSV processors.
 */
class ColorConversionPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "color_conversion.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new GrayscaleProcessor(),
            new HsvConvertProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.color_conversion.grayscale",
            "org.svw.color_conversion.hsv"
        };
    }
};

#include "ColorConversionPlugin.moc"
