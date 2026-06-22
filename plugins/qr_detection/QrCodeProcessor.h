#ifndef QRCODEPROCESSOR_H
#define QRCODEPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/objdetect.hpp>

class QrCodeProcessor : public QObject, public IImageProcessor
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
    bool m_showDecodedText = true;
};

#endif // QRCODEPROCESSOR_H
