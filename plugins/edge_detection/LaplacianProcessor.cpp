#include "LaplacianProcessor.h"

LaplacianProcessor::LaplacianProcessor()
    : m_ksize(DEFAULT_KSIZE)
    , m_scale(DEFAULT_SCALE)
{
}

PluginMetadata LaplacianProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.edge_detection.laplacian",
        "Laplacian Edge Detection",
        "Detect edges using the Laplacian operator",
        "Edge Detection"
    );
}

QList<ParameterDescriptor> LaplacianProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "ksize", "Kernel Size",
            DEFAULT_KSIZE, 1, 7, 2,
            {}, "Size of the Laplacian kernel (must be 1, 3, 5, or 7)"
        ),
        ParameterDescriptor::doubleParam(
            "scale", "Scale",
            DEFAULT_SCALE, 0.1, 10.0, 0.1,
            {}, "Scale factor for the computed Laplacian values"
        )
    };
}

void LaplacianProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "ksize") m_ksize = value.toInt();
    else if (id == "scale") m_scale = value.toDouble();
}

QVariant LaplacianProcessor::parameter(const QString& id) const
{
    if (id == "ksize") return m_ksize;
    if (id == "scale") return m_scale;
    return {};
}

void LaplacianProcessor::resetParameters()
{
    m_ksize = DEFAULT_KSIZE;
    m_scale = DEFAULT_SCALE;
}

ImageData LaplacianProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input.mat(), gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input.mat(), gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = input.mat();
    }

    cv::Mat laplacianOutput;
    cv::Laplacian(gray, laplacianOutput, CV_64F, m_ksize, m_scale);

    cv::Mat output;
    cv::convertScaleAbs(laplacianOutput, output);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject LaplacianProcessor::parameterState() const
{
    return {
        {"ksize", m_ksize},
        {"scale", m_scale}
    };
}

void LaplacianProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("ksize")) m_ksize = state["ksize"].toInt();
    if (state.contains("scale")) m_scale = state["scale"].toDouble();
}
