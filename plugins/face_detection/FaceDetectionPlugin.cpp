#include "HaarDetectorProcessor.h"
#include "DnnDetectorProcessor.h"
#include "core/IPluginFactory.h"
#include <QObject>

/**
 * @brief Plugin entry point for Face Detection processors.
 *
 * Implements IPluginFactory to provide Haar cascade and DNN-based
 * face detection processors.
 */
class FaceDetectionPlugin : public QObject, public IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginFactory_IID FILE "face_detection.json")
    Q_INTERFACES(IPluginFactory)

public:
    QList<IImageProcessor*> createProcessors(QObject* parent = nullptr) override
    {
        Q_UNUSED(parent)
        return {
            new HaarDetectorProcessor(),
            new DnnDetectorProcessor()
        };
    }

    QStringList processorIds() const override
    {
        return {
            "org.svw.face_detection.haar",
            "org.svw.face_detection.dnn"
        };
    }
};

#include "FaceDetectionPlugin.moc"
