#ifndef SIFTMATCHERPROCESSOR_H
#define SIFTMATCHERPROCESSOR_H

#include "core/IImageProcessor.h"
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

class SiftMatcherProcessor : public QObject, public IImageProcessor
{
    Q_OBJECT
    Q_INTERFACES(IImageProcessor)

public:
    static constexpr int DEFAULT_N_FEATURES = 500;
    static constexpr int DEFAULT_N_OCTAVE_LAYERS = 3;
    static constexpr double DEFAULT_CONTRAST_THRESHOLD = 0.04;
    static constexpr double DEFAULT_RATIO_THRESHOLD = 0.75;
    static constexpr int DEFAULT_MAX_MATCHES = 50;

    SiftMatcherProcessor()
        : m_nFeatures(DEFAULT_N_FEATURES)
        , m_nOctaveLayers(DEFAULT_N_OCTAVE_LAYERS)
        , m_contrastThreshold(DEFAULT_CONTRAST_THRESHOLD)
        , m_ratioThreshold(DEFAULT_RATIO_THRESHOLD)
        , m_maxMatches(DEFAULT_MAX_MATCHES) {}

    PluginMetadata metadata() const override
    {
        return PluginMetadata("org.svw.feature_matching.sift",
            "SIFT Feature Matching",
            "Detect and match SIFT features between two images",
            "Feature Matching");
    }

    QList<ParameterDescriptor> parameterDescriptors() const override
    {
        return {
            ParameterDescriptor::intParam("nFeatures", "Number of Features",
                DEFAULT_N_FEATURES, 100, 5000, 100),
            ParameterDescriptor::intParam("nOctaveLayers", "Octave Layers",
                DEFAULT_N_OCTAVE_LAYERS, 1, 10),
            ParameterDescriptor::doubleParam("contrastThreshold", "Contrast Threshold",
                DEFAULT_CONTRAST_THRESHOLD, 0.01, 1.0, 0.01),
            ParameterDescriptor::doubleParam("ratioThreshold", "Lowe's Ratio Threshold",
                DEFAULT_RATIO_THRESHOLD, 0.1, 1.0, 0.05),
            ParameterDescriptor::intParam("maxMatches", "Max Matches to Display",
                DEFAULT_MAX_MATCHES, 1, 500)
        };
    }

    void setParameter(const QString& id, const QVariant& value) override
    {
        if (id == "nFeatures") m_nFeatures = value.toInt();
        else if (id == "nOctaveLayers") m_nOctaveLayers = value.toInt();
        else if (id == "contrastThreshold") m_contrastThreshold = value.toDouble();
        else if (id == "ratioThreshold") m_ratioThreshold = value.toDouble();
        else if (id == "maxMatches") m_maxMatches = value.toInt();
    }

    QVariant parameter(const QString& id) const override
    {
        if (id == "nFeatures") return m_nFeatures;
        if (id == "nOctaveLayers") return m_nOctaveLayers;
        if (id == "contrastThreshold") return m_contrastThreshold;
        if (id == "ratioThreshold") return m_ratioThreshold;
        if (id == "maxMatches") return m_maxMatches;
        return {};
    }

    void resetParameters() override
    {
        m_nFeatures = DEFAULT_N_FEATURES;
        m_nOctaveLayers = DEFAULT_N_OCTAVE_LAYERS;
        m_contrastThreshold = DEFAULT_CONTRAST_THRESHOLD;
        m_ratioThreshold = DEFAULT_RATIO_THRESHOLD;
        m_maxMatches = DEFAULT_MAX_MATCHES;
    }

    bool requiresSecondImage() const override { return true; }

    ImageData process(const ImageData& input) override
    {
        if (input.isEmpty()) return input;

        // Get second image from metadata
        cv::Mat secondMat;
        if (input.metadata("_secondImage").isValid()) {
            secondMat = input.metadata("_secondImage").value<cv::Mat>();
        }
        if (secondMat.empty()) {
            ImageData result = input.deepCopy();
            result.setMetadata("warning", "No second image provided for matching");
            result.appendToHistory(metadata().id);
            return result;
        }

        cv::Mat img1 = input.mat();
        cv::Mat img2 = secondMat;

        // Convert to grayscale
        cv::Mat gray1, gray2;
        if (img1.channels() > 1) cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
        else gray1 = img1;
        if (img2.channels() > 1) cv::cvtColor(img2, gray2, cv::COLOR_BGR2GRAY);
        else gray2 = img2;

        // Detect SIFT features
        auto sift = cv::SIFT::create(m_nFeatures, m_nOctaveLayers, m_contrastThreshold);
        std::vector<cv::KeyPoint> kp1, kp2;
        cv::Mat desc1, desc2;
        sift->detectAndCompute(gray1, cv::noArray(), kp1, desc1);
        sift->detectAndCompute(gray2, cv::noArray(), kp2, desc2);

        if (desc1.empty() || desc2.empty()) {
            ImageData result = input.deepCopy();
            result.setMetadata("warning", "No features detected");
            result.appendToHistory(metadata().id);
            return result;
        }

        // Match features
        cv::BFMatcher matcher(cv::NORM_L2);
        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher.knnMatch(desc1, desc2, knnMatches, 2);

        // Apply Lowe's ratio test
        std::vector<cv::DMatch> goodMatches;
        for (const auto& match : knnMatches) {
            if (match.size() >= 2 && match[0].distance < m_ratioThreshold * match[1].distance) {
                goodMatches.push_back(match[0]);
            }
        }

        // Sort by distance and keep top N
        std::sort(goodMatches.begin(), goodMatches.end());
        if (static_cast<int>(goodMatches.size()) > m_maxMatches)
            goodMatches.resize(m_maxMatches);

        // Draw matches
        cv::Mat output;
        cv::drawMatches(img1, kp1, img2, kp2, goodMatches, output,
                        cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255));

        ImageData result(output, input.sourceId());
        result.setTimestamp(input.timestamp());
        result.setMetadata("matchCount", static_cast<int>(goodMatches.size()));
        result.appendToHistory(metadata().id);
        return result;
    }

    QJsonObject parameterState() const override
    {
        return {{"nFeatures", m_nFeatures}, {"nOctaveLayers", m_nOctaveLayers},
                {"contrastThreshold", m_contrastThreshold}, {"ratioThreshold", m_ratioThreshold},
                {"maxMatches", m_maxMatches}};
    }

    void setParameterState(const QJsonObject& state) override
    {
        if (state.contains("nFeatures")) m_nFeatures = state["nFeatures"].toInt();
        if (state.contains("nOctaveLayers")) m_nOctaveLayers = state["nOctaveLayers"].toInt();
        if (state.contains("contrastThreshold")) m_contrastThreshold = state["contrastThreshold"].toDouble();
        if (state.contains("ratioThreshold")) m_ratioThreshold = state["ratioThreshold"].toDouble();
        if (state.contains("maxMatches")) m_maxMatches = state["maxMatches"].toInt();
    }

private:
    int m_nFeatures;
    int m_nOctaveLayers;
    double m_contrastThreshold;
    double m_ratioThreshold;
    int m_maxMatches;
};

#endif // SIFTMATCHERPROCESSOR_H
