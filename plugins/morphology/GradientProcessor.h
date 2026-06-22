#ifndef GRADIENTPROCESSOR_H
#define GRADIENTPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/imgproc.hpp>

class GradientProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    static constexpr int DEFAULT_KERNEL_SIZE = 3;
    static constexpr int DEFAULT_KERNEL_SHAPE = 0;
    static constexpr int DEFAULT_ITERATIONS = 1;

    GradientProcessor()
        : m_kernelSize(DEFAULT_KERNEL_SIZE)
        , m_kernelShape(DEFAULT_KERNEL_SHAPE)
        , m_iterations(DEFAULT_ITERATIONS) {}

    PluginMetadata metadata() const override
    {
        return PluginMetadata("org.svw.morphology.gradient",
            "Morphological Gradient", "Difference between dilation and erosion",
            "Morphology");
    }

    QList<ParameterDescriptor> parameterDescriptors() const override
    {
        return {
            ParameterDescriptor::intParam("kernelSize", "Kernel Size",
                DEFAULT_KERNEL_SIZE, 1, 31, 2),
            ParameterDescriptor::enumParam("kernelShape", "Kernel Shape",
                {"Rect", "Cross", "Ellipse"}, DEFAULT_KERNEL_SHAPE),
            ParameterDescriptor::intParam("iterations", "Iterations",
                DEFAULT_ITERATIONS, 1, 20)
        };
    }

    void setParameter(const QString& id, const QVariant& value) override
    {
        if (id == "kernelSize") m_kernelSize = value.toInt();
        else if (id == "kernelShape") m_kernelShape = value.toInt();
        else if (id == "iterations") m_iterations = value.toInt();
    }

    QVariant parameter(const QString& id) const override
    {
        if (id == "kernelSize") return m_kernelSize;
        if (id == "kernelShape") return m_kernelShape;
        if (id == "iterations") return m_iterations;
        return {};
    }

    void resetParameters() override
    {
        m_kernelSize = DEFAULT_KERNEL_SIZE;
        m_kernelShape = DEFAULT_KERNEL_SHAPE;
        m_iterations = DEFAULT_ITERATIONS;
    }

    ImageData process(const ImageData& input) override
    {
        if (input.isEmpty()) return input;
        int ks = m_kernelSize | 1; // ensure odd
        int shape = m_kernelShape == 1 ? cv::MORPH_CROSS :
                    m_kernelShape == 2 ? cv::MORPH_ELLIPSE : cv::MORPH_RECT;
        cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(ks, ks));
        cv::Mat output;
        cv::morphologyEx(input.mat(), output, cv::MORPH_GRADIENT, kernel,
                         cv::Point(-1, -1), m_iterations);
        ImageData result(output, input.sourceId());
        result.setTimestamp(input.timestamp());
        result.appendToHistory(metadata().id);
        return result;
    }

    QJsonObject parameterState() const override
    {
        return {{"kernelSize", m_kernelSize}, {"kernelShape", m_kernelShape},
                {"iterations", m_iterations}};
    }

    void setParameterState(const QJsonObject& state) override
    {
        if (state.contains("kernelSize")) m_kernelSize = state["kernelSize"].toInt();
        if (state.contains("kernelShape")) m_kernelShape = state["kernelShape"].toInt();
        if (state.contains("iterations")) m_iterations = state["iterations"].toInt();
    }

private:
    int m_kernelSize;
    int m_kernelShape;
    int m_iterations;
};

#endif // GRADIENTPROCESSOR_H
