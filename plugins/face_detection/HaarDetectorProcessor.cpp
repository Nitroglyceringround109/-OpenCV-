#include "HaarDetectorProcessor.h"
#include "FaceDetectionUtils.h"
#include "utils/ModelPaths.h"
#include <opencv2/imgproc.hpp>
#include <QDebug>

PluginMetadata HaarDetectorProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.face_detection.haar",
        "Haar Face Detection",
        "Detect faces using Haar cascade classifiers (frontal + profile)",
        "Face Detection"
    );
}

QList<ParameterDescriptor> HaarDetectorProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::doubleParam(
            "scaleFactor", "Scale Factor",
            1.05, 1.01, 3.0, 0.01,
            {},
            tr("Image pyramid shrink ratio per step. Closer to 1.01 = finer search (slower, more accurate). "
               "1.05 is a good default.")
        ),
        ParameterDescriptor::intParam(
            "minNeighbors", "Min Neighbors",
            4, 0, 20, 1,
            {},
            tr("How many overlapping detections are required to confirm a face. "
               "Higher = fewer false positives but may miss real faces. Try 3–5.")
        ),
        ParameterDescriptor::intParam(
            "minFaceWidth", "Min Face Width",
            40, 10, 500, 1,
            {},
            tr("Minimum face width in pixels (full-resolution image). "
               "Increase to ignore small false detections.")
        ),
        ParameterDescriptor::intParam(
            "minFaceHeight", "Min Face Height",
            40, 10, 500, 1,
            {},
            tr("Minimum face height in pixels (full-resolution image). "
               "Increase to ignore small false detections.")
        )
    };
}

void HaarDetectorProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "scaleFactor")       m_scaleFactor = value.toDouble();
    else if (id == "minNeighbors") m_minNeighbors = value.toInt();
    else if (id == "minFaceWidth") m_minFaceWidth = value.toInt();
    else if (id == "minFaceHeight")m_minFaceHeight = value.toInt();
}

QVariant HaarDetectorProcessor::parameter(const QString& id) const
{
    if (id == "scaleFactor")       return m_scaleFactor;
    if (id == "minNeighbors")      return m_minNeighbors;
    if (id == "minFaceWidth")      return m_minFaceWidth;
    if (id == "minFaceHeight")     return m_minFaceHeight;
    return {};
}

void HaarDetectorProcessor::resetParameters()
{
    m_scaleFactor = 1.05;
    m_minNeighbors = 4;
    m_minFaceWidth = 40;
    m_minFaceHeight = 40;
}

bool HaarDetectorProcessor::loadCascades()
{
    if (m_cascadeAttempted)
        return m_frontalLoaded || m_profileLoaded;
    m_cascadeAttempted = true;

    const QString frontalPath = ModelPaths::resolveHaarFaceCascade();
    if (!frontalPath.isEmpty()) {
        m_frontalLoaded = m_frontalCascade.load(frontalPath.toStdString());
        if (m_frontalLoaded)
            qDebug() << "HaarDetectorProcessor: loaded frontal cascade from" << frontalPath;
        else
            qWarning() << "HaarDetectorProcessor: failed to load frontal cascade from" << frontalPath;
    }

    const QString profilePath = ModelPaths::resolveHaarProfileCascade();
    if (!profilePath.isEmpty()) {
        m_profileLoaded = m_profileCascade.load(profilePath.toStdString());
        if (m_profileLoaded)
            qDebug() << "HaarDetectorProcessor: loaded profile cascade from" << profilePath;
    }

    return m_frontalLoaded || m_profileLoaded;
}

ImageData HaarDetectorProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    if (!loadCascades()) {
        ImageData result(input.mat().clone(), input.sourceId());
        result.setTimestamp(input.timestamp());
        result.appendToHistory(metadata().id);
        result.setMetadata("warning",
            tr("Haar cascade not available. Check models/cascades/ or rebuild the project."));
        return result;
    }

    const cv::Mat gray = FaceDetectionUtils::prepareGray(input.mat());
    const int minFaceSize = std::max(m_minFaceWidth, m_minFaceHeight);

    std::vector<cv::CascadeClassifier*> cascades;
    if (m_frontalLoaded) cascades.push_back(&m_frontalCascade);
    if (m_profileLoaded) cascades.push_back(&m_profileCascade);

    const std::vector<FaceBox> faces = FaceDetectionUtils::detectHaar(
        gray, cascades, m_scaleFactor, m_minNeighbors, minFaceSize);

    cv::Mat output = input.mat().clone();
    FaceDetectionUtils::drawFaces(output, faces, false);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    result.setMetadata("face_count", static_cast<int>(faces.size()));
    return result;
}

QJsonObject HaarDetectorProcessor::parameterState() const
{
    return {
        {"scaleFactor", m_scaleFactor},
        {"minNeighbors", m_minNeighbors},
        {"minFaceWidth", m_minFaceWidth},
        {"minFaceHeight", m_minFaceHeight}
    };
}

void HaarDetectorProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("scaleFactor"))  m_scaleFactor  = state["scaleFactor"].toDouble();
    if (state.contains("minNeighbors")) m_minNeighbors = state["minNeighbors"].toInt();
    if (state.contains("minFaceWidth")) m_minFaceWidth = state["minFaceWidth"].toInt();
    if (state.contains("minFaceHeight"))m_minFaceHeight= state["minFaceHeight"].toInt();
}
