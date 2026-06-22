#include "OpenCloseProcessor.h"

OpenCloseProcessor::OpenCloseProcessor()
    : m_operation(0)
    , m_kernelSize(3)
    , m_kernelShape(0)
    , m_iterations(1)
{
}

PluginMetadata OpenCloseProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.morphology.openclose",
        "Open / Close",
        "Morphological opening or closing operation",
        "Morphology"
    );
}

QList<ParameterDescriptor> OpenCloseProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::enumParam(
            "operation", "Operation",
            {"Open", "Close"}, 0,
            {}, "Morphological operation type"
        ),
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
            {}, "Number of times the operation is applied"
        )
    };
}

void OpenCloseProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "operation") {
        m_operation = qBound(0, value.toInt(), 1);
    } else if (id == "kernelSize") {
        int v = value.toInt();
        if (v % 2 == 0) v++;
        m_kernelSize = qBound(1, v, 31);
    } else if (id == "kernelShape") {
        m_kernelShape = qBound(0, value.toInt(), 2);
    } else if (id == "iterations") {
        m_iterations = qBound(1, value.toInt(), 20);
    }
}

QVariant OpenCloseProcessor::parameter(const QString& id) const
{
    if (id == "operation")     return m_operation;
    if (id == "kernelSize")    return m_kernelSize;
    if (id == "kernelShape")   return m_kernelShape;
    if (id == "iterations")    return m_iterations;
    return {};
}

void OpenCloseProcessor::resetParameters()
{
    m_operation = 0;
    m_kernelSize = 3;
    m_kernelShape = 0;
    m_iterations = 1;
}

cv::Mat OpenCloseProcessor::createKernel() const
{
    int shape = cv::MORPH_RECT;
    if (m_kernelShape == 1)      shape = cv::MORPH_CROSS;
    else if (m_kernelShape == 2) shape = cv::MORPH_ELLIPSE;

    return cv::getStructuringElement(shape, cv::Size(m_kernelSize, m_kernelSize));
}

ImageData OpenCloseProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    cv::Mat kernel = createKernel();
    int morphOp = (m_operation == 0) ? cv::MORPH_OPEN : cv::MORPH_CLOSE;
    cv::Mat output;
    cv::morphologyEx(input.mat(), output, morphOp, kernel, cv::Point(-1, -1), m_iterations);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    return result;
}

QJsonObject OpenCloseProcessor::parameterState() const
{
    return {
        {"operation", m_operation},
        {"kernelSize", m_kernelSize},
        {"kernelShape", m_kernelShape},
        {"iterations", m_iterations}
    };
}

void OpenCloseProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("operation"))    m_operation = state["operation"].toInt();
    if (state.contains("kernelSize"))   m_kernelSize = state["kernelSize"].toInt();
    if (state.contains("kernelShape"))  m_kernelShape = state["kernelShape"].toInt();
    if (state.contains("iterations"))   m_iterations = state["iterations"].toInt();
}
