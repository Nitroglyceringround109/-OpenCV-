#include "AdaptiveThresholdProcessor.h"

AdaptiveThresholdProcessor::AdaptiveThresholdProcessor()
    : m_maxValue(255)
    , m_adaptiveMethod(0)
    , m_thresholdType(0)
    , m_blockSize(11)
    , m_C(2.0)
{
}

PluginMetadata AdaptiveThresholdProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.threshold.adaptive",
        "Adaptive Threshold",
        "Apply adaptive thresholding to an image",
        "Threshold"
    );
}

QList<ParameterDescriptor> AdaptiveThresholdProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "maxValue", "Max Value",
            255, 0, 255, 1
        ),
        ParameterDescriptor::enumParam(
            "adaptiveMethod", "Adaptive Method",
            {"Gaussian", "Mean"},
            0
        ),
        ParameterDescriptor::enumParam(
            "thresholdType", "Threshold Type",
            {"Binary", "Binary Inv"},
            0
        ),
        ParameterDescriptor::intParam(
            "blockSize", "Block Size",
            11, 3, 99, 2
        ),
        ParameterDescriptor::doubleParam(
            "C", "Constant C",
            2.0, -50.0, 50.0, 0.5
        )
    };
}

void AdaptiveThresholdProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "maxValue")
        m_maxValue = value.toInt();
    else if (id == "adaptiveMethod")
        m_adaptiveMethod = value.toInt();
    else if (id == "thresholdType")
        m_thresholdType = value.toInt();
    else if (id == "blockSize")
        m_blockSize = value.toInt();
    else if (id == "C")
        m_C = value.toDouble();
}

QVariant AdaptiveThresholdProcessor::parameter(const QString& id) const
{
    if (id == "maxValue")
        return m_maxValue;
    else if (id == "adaptiveMethod")
        return m_adaptiveMethod;
    else if (id == "thresholdType")
        return m_thresholdType;
    else if (id == "blockSize")
        return m_blockSize;
    else if (id == "C")
        return m_C;
    return {};
}

void AdaptiveThresholdProcessor::resetParameters()
{
    m_maxValue = 255;
    m_adaptiveMethod = 0;
    m_thresholdType = 0;
    m_blockSize = 11;
    m_C = 2.0;
}

ImageData AdaptiveThresholdProcessor::process(const ImageData& input)
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

    // Ensure blockSize is odd (required by cv::adaptiveThreshold)
    int blockSize = m_blockSize | 1;  // force odd
    if (blockSize < 3) blockSize = 3;

    int adaptiveFlag = (m_adaptiveMethod == 1)
        ? cv::ADAPTIVE_THRESH_MEAN_C
        : cv::ADAPTIVE_THRESH_GAUSSIAN_C;

    int typeFlag = (m_thresholdType == 1)
        ? cv::THRESH_BINARY_INV
        : cv::THRESH_BINARY;

    cv::Mat output;
    cv::adaptiveThreshold(gray, output, m_maxValue, adaptiveFlag, typeFlag, blockSize, m_C);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject AdaptiveThresholdProcessor::parameterState() const
{
    return {
        {"maxValue", m_maxValue},
        {"adaptiveMethod", m_adaptiveMethod},
        {"thresholdType", m_thresholdType},
        {"blockSize", m_blockSize},
        {"C", m_C}
    };
}

void AdaptiveThresholdProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("maxValue"))
        m_maxValue = state["maxValue"].toInt();
    if (state.contains("adaptiveMethod"))
        m_adaptiveMethod = state["adaptiveMethod"].toInt();
    if (state.contains("thresholdType"))
        m_thresholdType = state["thresholdType"].toInt();
    if (state.contains("blockSize"))
        m_blockSize = state["blockSize"].toInt();
    if (state.contains("C"))
        m_C = state["C"].toDouble();
}
