#ifndef CANNYPROCESSOR_H
#define CANNYPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class CannyProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    CannyProcessor();
    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;

private:
    double m_threshold1;
    double m_threshold2;
    int m_apertureSize;
    bool m_L2gradient;

    static constexpr double DEFAULT_THRESHOLD1 = 80.0;
    static constexpr double DEFAULT_THRESHOLD2 = 150.0;
    static constexpr int DEFAULT_APERTURE_SIZE = 3;
    static constexpr bool DEFAULT_L2GRADIENT = false;
};

#endif // CANNYPROCESSOR_H
