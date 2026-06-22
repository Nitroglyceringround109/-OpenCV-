#ifndef FACEDETECTIONUTILS_H
#define FACEDETECTIONUTILS_H

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>

struct FaceBox {
    cv::Rect rect;
    float score = 1.0f;
};

namespace FaceDetectionUtils {

cv::Mat prepareGray(const cv::Mat& input);
void nonMaximumSuppression(std::vector<FaceBox>& faces, float overlapThreshold);
void filterFaceGeometry(std::vector<FaceBox>& faces, const cv::Size& imageSize);
int annotationLineWidth(const cv::Size& imageSize);
void drawFaces(cv::Mat& output, const std::vector<FaceBox>& faces, bool showScores);

std::vector<FaceBox> detectDnn(
    cv::dnn::Net& net,
    const cv::Mat& bgr,
    float confidenceThreshold);

std::vector<FaceBox> detectHaar(
    const cv::Mat& gray,
    const std::vector<cv::CascadeClassifier*>& cascades,
    double scaleFactor,
    int minNeighbors,
    int minFaceSizePx);

} // namespace FaceDetectionUtils

#endif // FACEDETECTIONUTILS_H
