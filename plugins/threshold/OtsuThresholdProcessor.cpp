#include "OtsuThresholdProcessor.h"

OtsuThresholdProcessor::OtsuThresholdProcessor()
    : m_maxValue(255)
{
}

PluginMetadata OtsuThresholdProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.threshold.otsu",
        "Otsu Threshold",
        "Apply Otsu's automatic thresholding to an image",
        "Threshold"
    );
}

QList<ParameterDescriptor> OtsuThresholdProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "maxValue", "Max Value",
            255, 0, 255, 1
        )
    };
}

void OtsuThresholdProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "maxValue")
        m_maxValue = value.toInt();
}

QVariant OtsuThresholdProcessor::parameter(const QString& id) const
{
    if (id == "maxValue")
        return m_maxValue;
    return {};
}

void OtsuThresholdProcessor::resetParameters()
{
    m_maxValue = 255;
}

ImageData OtsuThresholdProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input.mat(), gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input.mat(), gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = input.mat().clone();
    }

    cv::Mat output;
    cv::threshold(gray, output, 0, m_maxValue, cv::THRESH_BINARY | cv::THRESH_OTSU);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject OtsuThresholdProcessor::parameterState() const
{
    return {
        {"maxValue", m_maxValue}
    };
}

void OtsuThresholdProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("maxValue"))
        m_maxValue = state["maxValue"].toInt();
}
