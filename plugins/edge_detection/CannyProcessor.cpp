#include "CannyProcessor.h"

CannyProcessor::CannyProcessor()
    : m_threshold1(DEFAULT_THRESHOLD1)
    , m_threshold2(DEFAULT_THRESHOLD2)
    , m_apertureSize(DEFAULT_APERTURE_SIZE)
    , m_L2gradient(DEFAULT_L2GRADIENT)
{
}

PluginMetadata CannyProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.edge_detection.canny",
        "Canny Edge Detection",
        "Detect edges using the Canny algorithm",
        "Edge Detection"
    );
}

QList<ParameterDescriptor> CannyProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::doubleParam(
            "threshold1", "Lower Threshold",
            DEFAULT_THRESHOLD1, 0.0, 500.0, 1.0,
            {}, "Lower threshold for hysteresis"
        ),
        ParameterDescriptor::doubleParam(
            "threshold2", "Upper Threshold",
            DEFAULT_THRESHOLD2, 0.0, 500.0, 1.0,
            {}, "Upper threshold for hysteresis"
        ),
        ParameterDescriptor::intParam(
            "apertureSize", "Aperture Size",
            DEFAULT_APERTURE_SIZE, 3, 7, 2,
            {}, "Sobel operator aperture size (must be 3, 5, or 7)"
        ),
        ParameterDescriptor::boolParam(
            "L2gradient", "Use L2 Gradient",
            DEFAULT_L2GRADIENT,
            {}, "Use L2 norm for gradient magnitude instead of L1"
        )
    };
}

void CannyProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "threshold1") m_threshold1 = value.toDouble();
    else if (id == "threshold2") m_threshold2 = value.toDouble();
    else if (id == "apertureSize") m_apertureSize = value.toInt();
    else if (id == "L2gradient") m_L2gradient = value.toBool();
}

QVariant CannyProcessor::parameter(const QString& id) const
{
    if (id == "threshold1") return m_threshold1;
    if (id == "threshold2") return m_threshold2;
    if (id == "apertureSize") return m_apertureSize;
    if (id == "L2gradient") return m_L2gradient;
    return {};
}

void CannyProcessor::resetParameters()
{
    m_threshold1 = DEFAULT_THRESHOLD1;
    m_threshold2 = DEFAULT_THRESHOLD2;
    m_apertureSize = DEFAULT_APERTURE_SIZE;
    m_L2gradient = DEFAULT_L2GRADIENT;
}

ImageData CannyProcessor::process(const ImageData& input)
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

    cv::Mat output;
    cv::Canny(gray, output, m_threshold1, m_threshold2, m_apertureSize, m_L2gradient);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject CannyProcessor::parameterState() const
{
    return {
        {"threshold1", m_threshold1},
        {"threshold2", m_threshold2},
        {"apertureSize", m_apertureSize},
        {"L2gradient", m_L2gradient}
    };
}

void CannyProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("threshold1")) m_threshold1 = state["threshold1"].toDouble();
    if (state.contains("threshold2")) m_threshold2 = state["threshold2"].toDouble();
    if (state.contains("apertureSize")) m_apertureSize = state["apertureSize"].toInt();
    if (state.contains("L2gradient")) m_L2gradient = state["L2gradient"].toBool();
}
