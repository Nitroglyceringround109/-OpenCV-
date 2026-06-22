#ifndef ERODEPROCESSOR_H
#define ERODEPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class ErodeProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    ErodeProcessor();

    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;

private:
    int m_kernelSize;
    int m_kernelShape;  // 0=Rect, 1=Cross, 2=Ellipse
    int m_iterations;

    cv::Mat createKernel() const;
};

#endif // ERODEPROCESSOR_H
