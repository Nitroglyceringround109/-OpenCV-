#ifndef DNNDETECTORPROCESSOR_H
#define DNNDETECTORPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/dnn.hpp>

class DnnDetectorProcessor : public QObject, public IImageProcessor
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
    double m_confidenceThreshold = 0.30;

    cv::dnn::Net m_net;
    bool m_modelLoaded = false;
    bool m_modelAttempted = false;
};

#endif // DNNDETECTORPROCESSOR_H
