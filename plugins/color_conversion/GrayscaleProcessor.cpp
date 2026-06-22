#include "GrayscaleProcessor.h"

PluginMetadata GrayscaleProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.color_conversion.grayscale",
        "Grayscale",
        "Convert image to grayscale",
        "Color Conversion"
    );
}

QList<ParameterDescriptor> GrayscaleProcessor::parameterDescriptors() const
{
    return {};  // No parameters for grayscale conversion
}

void GrayscaleProcessor::setParameter(const QString& id, const QVariant& value)
{
    Q_UNUSED(id) Q_UNUSED(value)
}

QVariant GrayscaleProcessor::parameter(const QString& id) const
{
    Q_UNUSED(id)
    return {};
}

void GrayscaleProcessor::resetParameters()
{
}

ImageData GrayscaleProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat output;
    if (input.channels() == 3) {
        cv::cvtColor(input.mat(), output, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input.mat(), output, cv::COLOR_BGRA2GRAY);
    } else {
        output = input.mat().clone();
    }

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject GrayscaleProcessor::parameterState() const
{
    return {};
}

void GrayscaleProcessor::setParameterState(const QJsonObject& state)
{
    Q_UNUSED(state)
}
