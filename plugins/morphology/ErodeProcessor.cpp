#include "ErodeProcessor.h"

ErodeProcessor::ErodeProcessor()
    : m_kernelSize(3)
    , m_kernelShape(0)
    , m_iterations(1)
{
}

PluginMetadata ErodeProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.morphology.erode",
        "Erode",
        "Erode image using a structuring element",
        "Morphology"
    );
}

QList<ParameterDescriptor> ErodeProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "kernelSize", "Kernel Size",
            3, 1, 31, 2,
            {}, "Size of the structuring element (must be odd)"
        ),
        ParameterDescriptor::enumParam(
            "kernelShape", "Kernel Shape",
            {"Rect", "Cross", "Ellipse"}, 0,
            {}, "Shape of the structuring element"
        ),
        ParameterDescriptor::intParam(
            "iterations", "Iterations",
            1, 1, 20, 1,
            {}, "Number of times erosion is applied"
        )
    };
}

void ErodeProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "kernelSize") {
        int v = value.toInt();
        if (v % 2 == 0) v++;
        m_kernelSize = qBound(1, v, 31);
    } else if (id == "kernelShape") {
        m_kernelShape = qBound(0, value.toInt(), 2);
    } else if (id == "iterations") {
        m_iterations = qBound(1, value.toInt(), 20);
    }
}

QVariant ErodeProcessor::parameter(const QString& id) const
{
    if (id == "kernelSize")     return m_kernelSize;
    if (id == "kernelShape")   return m_kernelShape;
    if (id == "iterations")    return m_iterations;
    return {};
}

void ErodeProcessor::resetParameters()
{
    m_kernelSize = 3;
    m_kernelShape = 0;
    m_iterations = 1;
}

cv::Mat ErodeProcessor::createKernel() const
{
    int shape = cv::MORPH_RECT;
    if (m_kernelShape == 1)      shape = cv::MORPH_CROSS;
    else if (m_kernelShape == 2) shape = cv::MORPH_ELLIPSE;

    return cv::getStructuringElement(shape, cv::Size(m_kernelSize, m_kernelSize));
}

ImageData ErodeProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat kernel = createKernel();
    cv::Mat output;
    cv::erode(input.mat(), output, kernel, cv::Point(-1, -1), m_iterations);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject ErodeProcessor::parameterState() const
{
    return {
        {"kernelSize", m_kernelSize},
        {"kernelShape", m_kernelShape},
        {"iterations", m_iterations}
    };
}

void ErodeProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("kernelSize"))   m_kernelSize = state["kernelSize"].toInt();
    if (state.contains("kernelShape"))  m_kernelShape = state["kernelShape"].toInt();
    if (state.contains("iterations"))   m_iterations = state["iterations"].toInt();
}
