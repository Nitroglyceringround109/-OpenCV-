#include "OrbMatcherProcessor.h"
#include <opencv2/imgproc.hpp>

PluginMetadata OrbMatcherProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.feature_matching.orb",
        "ORB Feature Matching",
        "Detect and match ORB features between two images",
        "Feature Matching"
    );
}

QList<ParameterDescriptor> OrbMatcherProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::intParam(
            "nFeatures", "Number of Features",
            500, 100, 5000, 100
        ),
        ParameterDescriptor::doubleParam(
            "scaleFactor", "Scale Factor",
            1.2, 1.01, 2.0, 0.01
        ),
        ParameterDescriptor::intParam(
            "nLevels", "Pyramid Levels",
            8, 1, 20
        ),
        ParameterDescriptor::doubleParam(
            "ratioThreshold", "Lowe's Ratio Threshold",
            0.75, 0.1, 1.0, 0.05
        ),
        ParameterDescriptor::intParam(
            "maxMatches", "Max Matches to Display",
            50, 1, 500
        )
    };
}

void OrbMatcherProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "nFeatures")        m_nFeatures = value.toInt();
    else if (id == "scaleFactor") m_scaleFactor = value.toDouble();
    else if (id == "nLevels")     m_nLevels = value.toInt();
    else if (id == "ratioThreshold") m_ratioThreshold = value.toDouble();
    else if (id == "maxMatches")  m_maxMatches = value.toInt();
}

QVariant OrbMatcherProcessor::parameter(const QString& id) const
{
    if (id == "nFeatures")        return m_nFeatures;
    if (id == "scaleFactor")      return m_scaleFactor;
    if (id == "nLevels")          return m_nLevels;
    if (id == "ratioThreshold")   return m_ratioThreshold;
    if (id == "maxMatches")       return m_maxMatches;
    return {};
}

void OrbMatcherProcessor::resetParameters()
{
    m_nFeatures = 500;
    m_scaleFactor = 1.2;
    m_nLevels = 8;
    m_ratioThreshold = 0.75;
    m_maxMatches = 50;
}

ImageData OrbMatcherProcessor::process(const ImageData& input)
{
    if (input.isEmpty()) return input;

    // Check for second image
    if (!input.allMetadata().contains("_secondImage")) {
        ImageData result = input;
        result.setMetadata("warning", "No second image provided for feature matching");
        return result;
    }

    QVariant secondImageVar = input.metadata("_secondImage");
    if (!secondImageVar.canConvert<cv::Mat>()) {
        ImageData result = input;
        result.setMetadata("warning", "Second image data is invalid");
        return result;
    }

    cv::Mat img1 = input.mat().clone();
    cv::Mat img2 = secondImageVar.value<cv::Mat>();

    // Ensure both images are grayscale for ORB
    if (img1.channels() > 1)
        cv::cvtColor(img1, img1, cv::COLOR_BGR2GRAY);
    if (img2.channels() > 1)
        cv::cvtColor(img2, img2, cv::COLOR_BGR2GRAY);

    // Create ORB detector
    auto orb = cv::ORB::create(m_nFeatures, m_scaleFactor, m_nLevels);

    // Detect keypoints and compute descriptors
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;
    orb->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
    orb->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);

    if (descriptors1.empty() || descriptors2.empty()) {
        ImageData result = input;
        result.setMetadata("warning", "No features detected in one or both images");
        return result;
    }

    // Match features using BFMatcher with Hamming distance
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher.knnMatch(descriptors1, descriptors2, knnMatches, 2);

    // Apply Lowe's ratio test
    std::vector<cv::DMatch> goodMatches;
    for (size_t i = 0; i < knnMatches.size(); ++i) {
        if (knnMatches[i].size() >= 2) {
            if (knnMatches[i][0].distance < m_ratioThreshold * knnMatches[i][1].distance) {
                goodMatches.push_back(knnMatches[i][0]);
            }
        }
    }

    // Sort by distance and limit to maxMatches
    std::sort(goodMatches.begin(), goodMatches.end());
    if (static_cast<int>(goodMatches.size()) > m_maxMatches)
        goodMatches.resize(m_maxMatches);

    // Draw matches - need color versions of the images
    cv::Mat img1Color = input.mat().clone();
    cv::Mat img2Color = secondImageVar.value<cv::Mat>().clone();
    if (img1Color.channels() == 1)
        cv::cvtColor(img1Color, img1Color, cv::COLOR_GRAY2BGR);
    if (img2Color.channels() == 1)
        cv::cvtColor(img2Color, img2Color, cv::COLOR_GRAY2BGR);

    cv::Mat matchImage;
    cv::drawMatches(img1Color, keypoints1, img2Color, keypoints2,
                    goodMatches, matchImage,
                    cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255));

    ImageData result(matchImage, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);
    result.setMetadata("keypoints1", static_cast<int>(keypoints1.size()));
    result.setMetadata("keypoints2", static_cast<int>(keypoints2.size()));
    result.setMetadata("goodMatches", static_cast<int>(goodMatches.size()));
    return result;
}

QJsonObject OrbMatcherProcessor::parameterState() const
{
    return {
        {"nFeatures", m_nFeatures},
        {"scaleFactor", m_scaleFactor},
        {"nLevels", m_nLevels},
        {"ratioThreshold", m_ratioThreshold},
        {"maxMatches", m_maxMatches}
    };
}

void OrbMatcherProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("nFeatures"))        m_nFeatures = state["nFeatures"].toInt();
    if (state.contains("scaleFactor"))      m_scaleFactor = state["scaleFactor"].toDouble();
    if (state.contains("nLevels"))          m_nLevels = state["nLevels"].toInt();
    if (state.contains("ratioThreshold"))   m_ratioThreshold = state["ratioThreshold"].toDouble();
    if (state.contains("maxMatches"))       m_maxMatches = state["maxMatches"].toInt();
}
