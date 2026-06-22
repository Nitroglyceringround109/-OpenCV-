#include "HsvConvertProcessor.h"

PluginMetadata HsvConvertProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.color_conversion.hsv",
        "HSV Conversion",
        "Convert image from BGR to HSV color space",
        "Color Conversion"
    );
}

QList<ParameterDescriptor> HsvConvertProcessor::parameterDescriptors() const
{
    return {};
}

void HsvConvertProcessor::setParameter(const QString& id, const QVariant& value)
{
    Q_UNUSED(id) Q_UNUSED(value)
}

QVariant HsvConvertProcessor::parameter(const QString& id) const
{
    Q_UNUSED(id)
    return {};
}

void HsvConvertProcessor::resetParameters()
{
}

ImageData HsvConvertProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat output;
    if (input.channels() == 3) {
        cv::cvtColor(input.mat(), output, cv::COLOR_BGR2HSV);
    } else if (input.channels() == 1) {
        cv::cvtColor(input.mat(), output, cv::COLOR_GRAY2BGR);
        cv::cvtColor(output, output, cv::COLOR_BGR2HSV);
    } else {
        output = input.mat().clone();
    }

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject HsvConvertProcessor::parameterState() const
{
    return {};
}

void HsvConvertProcessor::setParameterState(const QJsonObject& state)
{
    Q_UNUSED(state)
}
