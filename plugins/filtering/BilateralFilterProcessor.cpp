#include "BilateralFilterProcessor.h"

BilateralFilterProcessor::BilateralFilterProcessor()
    : m_d(9)
    , m_sigmaColor(75.0)
    , m_sigmaSpace(75.0)
{
}

PluginMetadata BilateralFilterProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.filtering.bilateral",
        "Bilateral Filter",
        "Apply bilateral filter to the image",
        "Filtering"
    );
}

QList<ParameterDescriptor> BilateralFilterProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "d", "Neighborhood Diameter",
            9, 1, 50, 1
        ),
        ParameterDescriptor::doubleParam(
            "sigmaColor", "Sigma Color",
            75.0, 1.0, 500.0, 1.0
        ),
        ParameterDescriptor::doubleParam(
            "sigmaSpace", "Sigma Space",
            75.0, 1.0, 500.0, 1.0
        )
    };
}

void BilateralFilterProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "d") {
        m_d = qBound(1, value.toInt(), 50);
    } else if (id == "sigmaColor") {
        m_sigmaColor = qBound(1.0, value.toDouble(), 500.0);
    } else if (id == "sigmaSpace") {
        m_sigmaSpace = qBound(1.0, value.toDouble(), 500.0);
    }
}

QVariant BilateralFilterProcessor::parameter(const QString& id) const
{
    if (id == "d") return m_d;
    if (id == "sigmaColor") return m_sigmaColor;
    if (id == "sigmaSpace") return m_sigmaSpace;
    return {};
}

void BilateralFilterProcessor::resetParameters()
{
    m_d = 9;
    m_sigmaColor = 75.0;
    m_sigmaSpace = 75.0;
}

ImageData BilateralFilterProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat output;
    cv::bilateralFilter(input.mat(), output, m_d, m_sigmaColor, m_sigmaSpace);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject BilateralFilterProcessor::parameterState() const
{
    return {
        {"d", m_d},
        {"sigmaColor", m_sigmaColor},
        {"sigmaSpace", m_sigmaSpace}
    };
}

void BilateralFilterProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("d")) m_d = state["d"].toInt();
    if (state.contains("sigmaColor")) m_sigmaColor = state["sigmaColor"].toDouble();
    if (state.contains("sigmaSpace")) m_sigmaSpace = state["sigmaSpace"].toDouble();
}
