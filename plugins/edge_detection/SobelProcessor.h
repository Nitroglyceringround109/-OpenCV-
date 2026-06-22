#ifndef SOBELPROCESSOR_H
#define SOBELPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class SobelProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    SobelProcessor();
    PluginMetadata metadata() const override;
    QList<ParameterDescriptor> parameterDescriptors() const override;
    void setParameter(const QString& id, const QVariant& value) override;
    QVariant parameter(const QString& id) const override;
    void resetParameters() override;
    ImageData process(const ImageData& input) override;
    QJsonObject parameterState() const override;
    void setParameterState(const QJsonObject& state) override;

private:
    int m_dx;
    int m_dy;
    int m_ksize;
    double m_scale;

    static constexpr int DEFAULT_DX = 1;
    static constexpr int DEFAULT_DY = 1;
    static constexpr int DEFAULT_KSIZE = 3;
    static constexpr double DEFAULT_SCALE = 1.0;
};

#endif // SOBELPROCESSOR_H
