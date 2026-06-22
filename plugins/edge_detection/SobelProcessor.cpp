#include "SobelProcessor.h"

SobelProcessor::SobelProcessor()
    : m_dx(DEFAULT_DX)
    , m_dy(DEFAULT_DY)
    , m_ksize(DEFAULT_KSIZE)
    , m_scale(DEFAULT_SCALE)
{
}

PluginMetadata SobelProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.edge_detection.sobel",
        "Sobel Edge Detection",
        "Detect edges using the Sobel operator",
        "Edge Detection"
    );
}

QList<ParameterDescriptor> SobelProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "dx", "Derivative X order",
            DEFAULT_DX, 0, 2, 1,
            {}, "Order of the derivative in X direction"
        ),
        ParameterDescriptor::intParam(
            "dy", "Derivative Y order",
            DEFAULT_DY, 0, 2, 1,
            {}, "Order of the derivative in Y direction"
        ),
        ParameterDescriptor::intParam(
            "ksize", "Kernel Size",
            DEFAULT_KSIZE, 1, 7, 2,
            {}, "Size of the Sobel kernel (must be 1, 3, 5, or 7)"
        ),
        ParameterDescriptor::doubleParam(
            "scale", "Scale",
            DEFAULT_SCALE, 0.1, 10.0, 0.1,
            {}, "Scale factor for the computed derivative values"
        )
    };
}

void SobelProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "dx") m_dx = value.toInt();
    else if (id == "dy") m_dy = value.toInt();
    else if (id == "ksize") m_ksize = value.toInt();
    else if (id == "scale") m_scale = value.toDouble();
}

QVariant SobelProcessor::parameter(const QString& id) const
{
    if (id == "dx") return m_dx;
    if (id == "dy") return m_dy;
    if (id == "ksize") return m_ksize;
    if (id == "scale") return m_scale;
    return {};
}

void SobelProcessor::resetParameters()
{
    m_dx = DEFAULT_DX;
    m_dy = DEFAULT_DY;
    m_ksize = DEFAULT_KSIZE;
    m_scale = DEFAULT_SCALE;
}

ImageData SobelProcessor::process(const ImageData& input)
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

    cv::Mat sobelOutput;
    cv::Sobel(gray, sobelOutput, CV_64F, m_dx, m_dy, m_ksize, m_scale);

    cv::Mat output;
    cv::convertScaleAbs(sobelOutput, output);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject SobelProcessor::parameterState() const
{
    return {
        {"dx", m_dx},
        {"dy", m_dy},
        {"ksize", m_ksize},
        {"scale", m_scale}
    };
}

void SobelProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("dx")) m_dx = state["dx"].toInt();
    if (state.contains("dy")) m_dy = state["dy"].toInt();
    if (state.contains("ksize")) m_ksize = state["ksize"].toInt();
    if (state.contains("scale")) m_scale = state["scale"].toDouble();
}
