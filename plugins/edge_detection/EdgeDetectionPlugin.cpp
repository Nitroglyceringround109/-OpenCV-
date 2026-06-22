#include "CannyProcessor.h"
#include "SobelProcessor.h"
#include "LaplacianProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

/**
 * @brief Plugin entry point for Edge Detection processors.
 *
 * Implements IPluginFactory to provide Canny, Sobel, and Laplacian processors.
 */
class EdgeDetectionPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "edge_detection.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new CannyProcessor(),
            new SobelProcessor(),
            new LaplacianProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.edge_detection.canny",
            "org.svw.edge_detection.sobel",
            "org.svw.edge_detection.laplacian"
        };
    }
};

#include "EdgeDetectionPlugin.moc"
