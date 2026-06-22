#include "QrCodeProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

/**
 * @brief Plugin entry point for QR Detection processors.
 *
 * Implements IPluginFactory to provide QrCodeProcessor.
 */
class QrDetectionPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "qr_detection.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new QrCodeProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.qr_detection.qr"
        };
    }
};

#include "QrDetectionPlugin.moc"
