#include "FaceDetectionUtils.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace FaceDetectionUtils {

namespace {

float boxIoU(const cv::Rect& a, const cv::Rect& b)
{
    const int xLeft = std::max(a.x, b.x);
    const int yTop = std::max(a.y, b.y);
    const int xRight = std::min(a.x + a.width, b.x + b.width);
    const int yBottom = std::min(a.y + a.height, b.y + b.height);

    const int interW = std::max(0, xRight - xLeft);
    const int interH = std::max(0, yBottom - yTop);
    const float interArea = static_cast<float>(interW * interH);
    const float unionArea = static_cast<float>(a.area() + b.area()) - interArea;
    if (unionArea <= 0.0f)
        return 0.0f;
    return interArea / unionArea;
}

void appendDnnDetections(
    const cv::Mat& region,
    const cv::Rect& regionInImage,
    cv::dnn::Net& net,
    float minConfidence,
    std::vector<FaceBox>& out)
{
    cv::Mat blob = cv::dnn::blobFromImage(
        region, 1.0, cv::Size(300, 300),
        cv::Scalar(104.0, 177.0, 123.0), false, false);
    net.setInput(blob);

    cv::Mat detections = net.forward("detection_out");
    if (detections.empty() || detections.dims != 4
        || detections.size[0] != 1 || detections.size[1] != 1) {
        return;
    }

    const int numDetections = detections.size[2];
    const float* data = detections.ptr<float>();

    for (int i = 0; i < numDetections; ++i) {
        const int classId = static_cast<int>(data[i * 7 + 1]);
        const float confidence = data[i * 7 + 2];
        if (classId != 1 || confidence < minConfidence)
            continue;

        int xLeft = static_cast<int>(data[i * 7 + 3] * region.cols);
        int yTop = static_cast<int>(data[i * 7 + 4] * region.rows);
        int xRight = static_cast<int>(data[i * 7 + 5] * region.cols);
        int yBottom = static_cast<int>(data[i * 7 + 6] * region.rows);

        xLeft = std::max(0, std::min(xLeft, region.cols - 1));
        yTop = std::max(0, std::min(yTop, region.rows - 1));
        xRight = std::max(0, std::min(xRight, region.cols - 1));
        yBottom = std::max(0, std::min(yBottom, region.rows - 1));

        if (xRight <= xLeft || yBottom <= yTop)
            continue;

        cv::Rect local(xLeft, yTop, xRight - xLeft, yBottom - yTop);
        local.x += regionInImage.x;
        local.y += regionInImage.y;
        out.push_back({local, confidence});
    }
}

cv::Mat scaledGray(const cv::Mat& gray, double scale, cv::Mat& buffer)
{
    if (scale >= 0.999)
        return gray;
    cv::resize(gray, buffer, cv::Size(), scale, scale, cv::INTER_AREA);
    return buffer;
}

cv::Mat enhanceForDetection(const cv::Mat& bgr)
{
    if (bgr.channels() != 3)
        return bgr;

    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(channels[2], channels[2]);
    cv::merge(channels, hsv);
    cv::Mat enhanced;
    cv::cvtColor(hsv, enhanced, cv::COLOR_HSV2BGR);
    return enhanced;
}

} // namespace

cv::Mat prepareGray(const cv::Mat& input)
{
    cv::Mat gray;
    if (input.channels() == 3)
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    else if (input.channels() == 4)
        cv::cvtColor(input, gray, cv::COLOR_BGRA2GRAY);
    else
        gray = input;

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, gray);
    return gray;
}

void nonMaximumSuppression(std::vector<FaceBox>& faces, float overlapThreshold)
{
    if (faces.size() < 2)
        return;

    std::sort(faces.begin(), faces.end(),
              [](const FaceBox& a, const FaceBox& b) { return a.score > b.score; });

    std::vector<FaceBox> kept;
    kept.reserve(faces.size());

    std::vector<bool> suppressed(faces.size(), false);
    for (size_t i = 0; i < faces.size(); ++i) {
        if (suppressed[i])
            continue;
        kept.push_back(faces[i]);
        for (size_t j = i + 1; j < faces.size(); ++j) {
            if (suppressed[j])
                continue;
            if (boxIoU(faces[i].rect, faces[j].rect) > overlapThreshold)
                suppressed[j] = true;
        }
    }

    faces.swap(kept);
}

void filterFaceGeometry(std::vector<FaceBox>& faces, const cv::Size& imageSize)
{
    const int minDim = std::min(imageSize.width, imageSize.height);
    const int minSide = std::max(24, static_cast<int>(minDim * 0.012));
    const int maxSide = static_cast<int>(minDim * 0.42);

    std::vector<FaceBox> filtered;
    filtered.reserve(faces.size());

    for (const FaceBox& face : faces) {
        const cv::Rect& r = face.rect;
        if (r.width < minSide || r.height < minSide)
            continue;
        if (r.width > maxSide || r.height > maxSide)
            continue;

        const float aspect = static_cast<float>(r.width) / static_cast<float>(r.height);
        if (aspect < 0.55f || aspect > 1.75f)
            continue;

        const float centerY = r.y + r.height * 0.5f;
        const float relY = centerY / static_cast<float>(imageSize.height);
        if (relY > 0.88f && aspect > 1.2f)
            continue;
        if (relY > 0.82f && r.height < minDim * 0.02 && aspect > 1.35f)
            continue;

        if (r.x + r.width < 0 || r.y + r.height < 0)
            continue;
        if (r.x >= imageSize.width || r.y >= imageSize.height)
            continue;

        filtered.push_back(face);
    }

    faces.swap(filtered);
}

int annotationLineWidth(const cv::Size& imageSize)
{
    return std::max(2, std::min(imageSize.width, imageSize.height) / 800);
}

void drawFaces(cv::Mat& output, const std::vector<FaceBox>& faces, bool showScores)
{
    const int lineWidth = annotationLineWidth(output.size());
    const double fontScale = std::max(0.5, lineWidth * 0.4);

    for (const FaceBox& face : faces) {
        cv::rectangle(output, face.rect, cv::Scalar(0, 255, 0), lineWidth);
        if (!showScores)
            continue;

        char label[32];
        std::snprintf(label, sizeof(label), "%.1f%%", face.score * 100.0f);
        cv::putText(output, label,
                    cv::Point(face.rect.x, std::max(face.rect.y - lineWidth, lineWidth)),
                    cv::FONT_HERSHEY_SIMPLEX, fontScale,
                    cv::Scalar(0, 255, 0), lineWidth);
    }
}

std::vector<FaceBox> detectDnn(
    cv::dnn::Net& net,
    const cv::Mat& bgr,
    float confidenceThreshold)
{
    const cv::Mat enhanced = enhanceForDetection(bgr);
    std::vector<FaceBox> candidates;
    const float internalThreshold = std::max(0.15f, confidenceThreshold * 0.75f);
    const cv::Rect full(0, 0, enhanced.cols, enhanced.rows);

    appendDnnDetections(enhanced, full, net, internalThreshold, candidates);

    const int maxDim = std::max(enhanced.cols, enhanced.rows);
    if (maxDim > 1200) {
        const int tileSize = std::min(1400, maxDim);
        const int stride = static_cast<int>(tileSize * 0.6);
        for (int y = 0; y < enhanced.rows; y += stride) {
            for (int x = 0; x < enhanced.cols; x += stride) {
                const int tw = std::min(tileSize, enhanced.cols - x);
                const int th = std::min(tileSize, enhanced.rows - y);
                if (tw < 320 || th < 320)
                    continue;
                const cv::Rect roi(x, y, tw, th);
                appendDnnDetections(enhanced(roi), roi, net, internalThreshold, candidates);
            }
        }

        const int marginX = enhanced.cols / 12;
        const int marginY = enhanced.rows / 12;
        const cv::Rect centerCrop(
            marginX, marginY,
            enhanced.cols - 2 * marginX,
            enhanced.rows - 2 * marginY);
        if (centerCrop.width > 320 && centerCrop.height > 320)
            appendDnnDetections(enhanced(centerCrop), centerCrop, net, internalThreshold, candidates);
    }

    nonMaximumSuppression(candidates, 0.35f);
    filterFaceGeometry(candidates, bgr.size());

    const float finalThreshold = confidenceThreshold;
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
                       [finalThreshold](const FaceBox& f) {
                           return f.score < finalThreshold;
                       }),
        candidates.end());

    return candidates;
}

std::vector<FaceBox> detectHaar(
    const cv::Mat& gray,
    const std::vector<cv::CascadeClassifier*>& cascades,
    double scaleFactor,
    int minNeighbors,
    int minFaceSizePx)
{
    std::vector<FaceBox> candidates;
    cv::Mat scaledBuffer;

    const int maxDim = std::max(gray.cols, gray.rows);
    const int scaleTargets[] = {1920, 1280, 960};

    for (int targetDim : scaleTargets) {
        if (targetDim > maxDim && targetDim != scaleTargets[0])
            continue;

        const double detectScale = maxDim > targetDim
            ? static_cast<double>(targetDim) / maxDim
            : 1.0;
        const cv::Mat& detectGray = scaledGray(gray, detectScale, scaledBuffer);

        const int scaledMinFace = std::max(
            18,
            static_cast<int>(minFaceSizePx * detectScale));

        for (cv::CascadeClassifier* cascade : cascades) {
            if (!cascade || cascade->empty())
                continue;

            std::vector<cv::Rect> faces;
            cascade->detectMultiScale(
                detectGray, faces,
                scaleFactor,
                minNeighbors,
                cv::CASCADE_SCALE_IMAGE,
                cv::Size(scaledMinFace, scaledMinFace));

            const double invScale = 1.0 / detectScale;
            for (const cv::Rect& face : faces) {
                cv::Rect mapped(
                    static_cast<int>(face.x * invScale),
                    static_cast<int>(face.y * invScale),
                    static_cast<int>(face.width * invScale),
                    static_cast<int>(face.height * invScale));
                candidates.push_back({mapped, 1.0f});
            }
        }

        if (candidates.size() >= 4)
            break;
    }

    nonMaximumSuppression(candidates, 0.25f);
    filterFaceGeometry(candidates, gray.size());
    return candidates;
}

} // namespace FaceDetectionUtils
