#ifndef HAARDETECTORPROCESSOR_H
#define HAARDETECTORPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/objdetect.hpp>

class HaarDetectorProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;
    bool isAnnotationProcessor() const override { return true; }

private:
    bool loadCascades();

    double m_scaleFactor = 1.05;
    int m_minNeighbors = 4;
    int m_minFaceWidth = 40;
    int m_minFaceHeight = 40;

    cv::CascadeClassifier m_frontalCascade;
    cv::CascadeClassifier m_profileCascade;
    bool m_frontalLoaded = false;
    bool m_profileLoaded = false;
    bool m_cascadeAttempted = false;
};

#endif // HAARDETECTORPROCESSOR_H
