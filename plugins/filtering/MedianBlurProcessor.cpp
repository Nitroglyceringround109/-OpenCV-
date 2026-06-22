#include "MedianBlurProcessor.h"

MedianBlurProcessor::MedianBlurProcessor()
    : m_ksize(5)
{
}

PluginMetadata MedianBlurProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.filtering.median",
        "Median Blur",
        "Apply median blur to the image",
        "Filtering"
    );
}

QList<ParameterDescriptor> MedianBlurProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "ksize", "Kernel Size (odd)",
            5, 1, 31, 2
        )
    };
}

void MedianBlurProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "ksize") {
        int v = value.toInt();
        // Ensure odd kernel size
        if (v % 2 == 0) v++;
        m_ksize = qBound(1, v, 31);
    }
}

QVariant MedianBlurProcessor::parameter(const QString& id) const
{
    if (id == "ksize") return m_ksize;
    return {};
}

void MedianBlurProcessor::resetParameters()
{
    m_ksize = 5;
}

ImageData MedianBlurProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat output;
    cv::medianBlur(input.mat(), output, m_ksize);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject MedianBlurProcessor::parameterState() const
{
    return {
        {"ksize", m_ksize}
    };
}

void MedianBlurProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("ksize")) m_ksize = state["ksize"].toInt();
}
