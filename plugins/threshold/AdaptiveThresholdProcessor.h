#ifndef ADAPTIVETHRESHOLDPROCESSOR_H
#define ADAPTIVETHRESHOLDPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class AdaptiveThresholdProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    AdaptiveThresholdProcessor();

    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;

private:
    int m_maxValue;
    int m_adaptiveMethod;  // enum index: 0=Gaussian, 1=Mean
    int m_thresholdType;   // enum index: 0=Binary, 1=Binary Inv
    int m_blockSize;
    double m_C;
};

#endif // ADAPTIVETHRESHOLDPROCESSOR_H
