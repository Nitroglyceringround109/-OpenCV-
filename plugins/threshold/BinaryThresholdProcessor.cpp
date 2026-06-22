#include "BinaryThresholdProcessor.h"

BinaryThresholdProcessor::BinaryThresholdProcessor()
    : m_thresholdValue(128)
    , m_maxValue(255)
    , m_thresholdType(0)
{
}

PluginMetadata BinaryThresholdProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.threshold.binary",
        "Binary Threshold",
        "Apply binary thresholding to an image",
        "Threshold"
    );
}

QList<ParameterDescriptor> BinaryThresholdProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "thresholdValue", "Threshold Value",
            128, 0, 255, 1
        ),
        ParameterDescriptor::intParam(
            "maxValue", "Max Value",
            255, 0, 255, 1
        ),
        ParameterDescriptor::enumParam(
            "thresholdType", "Threshold Type",
            {"Binary", "Binary Inv", "Trunc", "To Zero", "To Zero Inv"},
            0
        )
    };
}

void BinaryThresholdProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "thresholdValue")
        m_thresholdValue = value.toInt();
    else if (id == "maxValue")
        m_maxValue = value.toInt();
    else if (id == "thresholdType")
        m_thresholdType = value.toInt();
}

QVariant BinaryThresholdProcessor::parameter(const QString& id) const
{
    if (id == "thresholdValue")
        return m_thresholdValue;
    else if (id == "maxValue")
        return m_maxValue;
    else if (id == "thresholdType")
        return m_thresholdType;
    return {};
}

void BinaryThresholdProcessor::resetParameters()
{
    m_thresholdValue = 128;
    m_maxValue = 255;
    m_thresholdType = 0;
}

ImageData BinaryThresholdProcessor::process(const ImageData& input)
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

    // Map enum index to OpenCV threshold type flag
    static const int typeFlags[] = {
        cv::THRESH_BINARY,
        cv::THRESH_BINARY_INV,
        cv::THRESH_TRUNC,
        cv::THRESH_TOZERO,
        cv::THRESH_TOZERO_INV
    };

    int type = (m_thresholdType >= 0 && m_thresholdType <= 4)
        ? typeFlags[m_thresholdType]
        : cv::THRESH_BINARY;

    cv::Mat output;
    cv::threshold(gray, output, m_thresholdValue, m_maxValue, type);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject BinaryThresholdProcessor::parameterState() const
{
    return {
        {"thresholdValue", m_thresholdValue},
        {"maxValue", m_maxValue},
        {"thresholdType", m_thresholdType}
    };
}

void BinaryThresholdProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("thresholdValue"))
        m_thresholdValue = state["thresholdValue"].toInt();
    if (state.contains("maxValue"))
        m_maxValue = state["maxValue"].toInt();
    if (state.contains("thresholdType"))
        m_thresholdType = state["thresholdType"].toInt();
}
