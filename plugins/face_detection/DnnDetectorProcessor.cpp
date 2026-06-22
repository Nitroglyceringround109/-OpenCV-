#include "DnnDetectorProcessor.h"
#include "FaceDetectionUtils.h"
#include "utils/ModelPaths.h"
#include <opencv2/imgproc.hpp>
#include <QDebug>

PluginMetadata DnnDetectorProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.face_detection.dnn",
        "DNN Face Detection",
        "Detect faces using a deep neural network (Caffe SSD)",
        "Face Detection"
    );
}

QList<ParameterDescriptor> DnnDetectorProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::doubleParam(
            "confidenceThreshold", "Confidence Threshold",
            0.30, 0.1, 1.0, 0.05,
            {},
            tr("Minimum model confidence (0–1) to keep a detection. "
               "Higher = fewer false positives; lower = more faces found. Recommended: 0.25–0.40.")
        )
    };
}

void DnnDetectorProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "confidenceThreshold")
        m_confidenceThreshold = value.toDouble();
}

QVariant DnnDetectorProcessor::parameter(const QString& id) const
{
    if (id == "confidenceThreshold")
        return m_confidenceThreshold;
    return {};
}

void DnnDetectorProcessor::resetParameters()
{
    m_confidenceThreshold = 0.30;
}

ImageData DnnDetectorProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    if (!m_modelAttempted) {
        m_modelAttempted = true;
        const ModelPaths::DnnFaceModelPaths modelPaths = ModelPaths::resolveDnnFaceModel();
        if (modelPaths.isValid()) {
            try {
                m_net = cv::dnn::readNetFromCaffe(
                    modelPaths.prototxt.toStdString(),
                    modelPaths.weights.toStdString());
                m_modelLoaded = !m_net.empty();
                if (m_modelLoaded) {
                    m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
                    m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
                    qDebug() << "DnnDetectorProcessor: loaded model from"
                             << modelPaths.prototxt << modelPaths.weights;
                }
            } catch (const cv::Exception& e) {
                qWarning() << "DnnDetectorProcessor: failed to load Caffe model:"
                           << e.what();
                m_modelLoaded = false;
            }
        } else {
            qWarning() << "DnnDetectorProcessor: model files not found.";
        }
    }

    if (!m_modelLoaded) {
        ImageData result(input.mat().clone(), input.sourceId());
        result.setTimestamp(input.timestamp());
        result.appendToHistory(metadata().id);
        result.setMetadata("warning",
            tr("DNN model not available. Run scripts/download_dnn_models.bat or place model files in models/."));
        return result;
    }

    const std::vector<FaceBox> faces = FaceDetectionUtils::detectDnn(
        m_net, input.mat(), static_cast<float>(m_confidenceThreshold));

    cv::Mat output = input.mat().clone();
    FaceDetectionUtils::drawFaces(output, faces, true);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    result.setMetadata("face_count", static_cast<int>(faces.size()));
    return result;
}

QJsonObject DnnDetectorProcessor::parameterState() const
{
    return {
        {"confidenceThreshold", m_confidenceThreshold}
    };
}

void DnnDetectorProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("confidenceThreshold"))
        m_confidenceThreshold = state["confidenceThreshold"].toDouble();
}
