#include "GaussianBlurProcessor.h"

GaussianBlurProcessor::GaussianBlurProcessor()
    : m_ksize(5)
    , m_sigmaX(0.0)
    , m_sigmaY(0.0)
{
}

PluginMetadata GaussianBlurProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.filtering.gaussian",
        "Gaussian Blur",
        "Apply Gaussian blur to the image",
        "Filtering"
    );
}

QList<ParameterDescriptor> GaussianBlurProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "ksize", "Kernel Size (odd)",
            5, 1, 31, 2
        ),
        ParameterDescriptor::doubleParam(
            "sigmaX", "Sigma X",
            0.0, 0.0, 100.0, 0.5
        ),
        ParameterDescriptor::doubleParam(
            "sigmaY", "Sigma Y",
            0.0, 0.0, 100.0, 0.5
        )
    };
}

void GaussianBlurProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "ksize") {
        int v = value.toInt();
        // Ensure odd kernel size
        if (v % 2 == 0) v++;
        m_ksize = qBound(1, v, 31);
    } else if (id == "sigmaX") {
        m_sigmaX = qBound(0.0, value.toDouble(), 100.0);
    } else if (id == "sigmaY") {
        m_sigmaY = qBound(0.0, value.toDouble(), 100.0);
    }
}

QVariant GaussianBlurProcessor::parameter(const QString& id) const
{
    if (id == "ksize") return m_ksize;
    if (id == "sigmaX") return m_sigmaX;
    if (id == "sigmaY") return m_sigmaY;
    return {};
}

void GaussianBlurProcessor::resetParameters()
{
    m_ksize = 5;
    m_sigmaX = 0.0;
    m_sigmaY = 0.0;
}

ImageData GaussianBlurProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat output;
    cv::GaussianBlur(input.mat(), output, cv::Size(m_ksize, m_ksize), m_sigmaX, m_sigmaY);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject GaussianBlurProcessor::parameterState() const
{
    return {
        {"ksize", m_ksize},
        {"sigmaX", m_sigmaX},
        {"sigmaY", m_sigmaY}
    };
}

void GaussianBlurProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("ksize")) m_ksize = state["ksize"].toInt();
    if (state.contains("sigmaX")) m_sigmaX = state["sigmaX"].toDouble();
    if (state.contains("sigmaY")) m_sigmaY = state["sigmaY"].toDouble();
}
