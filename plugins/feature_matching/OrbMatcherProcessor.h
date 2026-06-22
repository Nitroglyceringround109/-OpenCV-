#ifndef ORBMATCHERPROCESSOR_H
#define ORBMATCHERPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/features2d.hpp>

class OrbMatcherProcessor : public QObject, public IImageProcessor
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
    bool requiresSecondImage() const override { return true; }

private:
    int m_nFeatures = 500;
    double m_scaleFactor = 1.2;
    int m_nLevels = 8;
    double m_ratioThreshold = 0.75;
    int m_maxMatches = 50;
};

#endif // ORBMATCHERPROCESSOR_H
