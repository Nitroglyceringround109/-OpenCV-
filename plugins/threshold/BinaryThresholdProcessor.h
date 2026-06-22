#ifndef BINARYTHRESHOLDPROCESSOR_H
#define BINARYTHRESHOLDPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class BinaryThresholdProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    BinaryThresholdProcessor();

    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;

private:
    int m_thresholdValue;
    int m_maxValue;
    int m_thresholdType;  // enum index
};

#endif // BINARYTHRESHOLDPROCESSOR_H
