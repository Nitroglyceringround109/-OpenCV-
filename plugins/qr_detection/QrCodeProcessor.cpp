#include "QrCodeProcessor.h"
#include "utils/ModelPaths.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/wechat_qrcode.hpp>
#include <algorithm>
#include <cmath>
#include <memory>

namespace {

QString decodeUtf8(const std::string& bytes)
{
    if (bytes.empty())
        return {};
    return QString::fromUtf8(bytes.data(), static_cast<int>(bytes.size()));
}

cv::Mat toGray(const cv::Mat& input)
{
    cv::Mat gray;
    if (input.channels() == 1) {
        gray = input;
    } else if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, gray, cv::COLOR_BGRA2GRAY);
    } else {
        input.convertTo(gray, CV_8U);
    }
    if (!gray.isContinuous())
        gray = gray.clone();
    return gray;
}

cv::Mat toBgr(const cv::Mat& input)
{
    cv::Mat bgr;
    if (input.channels() == 1) {
        cv::cvtColor(input, bgr, cv::COLOR_GRAY2BGR);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, bgr, cv::COLOR_BGRA2BGR);
    } else if (input.channels() == 3) {
        bgr = input.clone();
    } else {
        cv::Mat converted;
        input.convertTo(converted, CV_8UC3);
        bgr = converted;
    }
    if (!bgr.isContinuous())
        bgr = bgr.clone();
    return bgr;
}

cv::Mat coloredQrGray(const cv::Mat& bgr)
{
    if (bgr.empty() || bgr.channels() < 3)
        return {};

    std::vector<cv::Mat> channels;
    cv::split(bgr, channels);
    cv::Mat emphasis;
    cv::max(channels[2], channels[1], emphasis);
    cv::subtract(emphasis, channels[0], emphasis);
    cv::normalize(emphasis, emphasis, 0, 255, cv::NORM_MINMAX, CV_8U);
    return emphasis;
}

std::vector<cv::Mat> buildBinaryVariants(const cv::Mat& gray)
{
    std::vector<cv::Mat> variants;
    if (gray.empty())
        return variants;

    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

    cv::Mat otsu;
    cv::threshold(blurred, otsu, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    variants.push_back(otsu);
    variants.push_back(255 - otsu);

    const int blockSizes[] = {15, 21, 31, 51};
    for (int block : blockSizes) {
        if (block >= gray.cols || block >= gray.rows)
            continue;
        cv::Mat adaptive;
        cv::adaptiveThreshold(blurred, adaptive, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                              cv::THRESH_BINARY, block, 2);
        variants.push_back(adaptive);
        variants.push_back(255 - adaptive);
    }

    return variants;
}

void appendUniqueMats(std::vector<cv::Mat>& dst, const std::vector<cv::Mat>& src)
{
    for (const cv::Mat& mat : src) {
        if (!mat.empty())
            dst.push_back(mat.isContinuous() ? mat : mat.clone());
    }
}

std::vector<cv::Mat> buildDetectionCandidates(const cv::Mat& gray, const cv::Mat& bgr)
{
    std::vector<cv::Mat> candidates;
    candidates.push_back(gray);

    cv::Mat claheGray;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, claheGray);
    candidates.push_back(claheGray);

    const cv::Mat colored = coloredQrGray(bgr);
    if (!colored.empty())
        candidates.push_back(colored);

    appendUniqueMats(candidates, buildBinaryVariants(gray));
    if (!colored.empty())
        appendUniqueMats(candidates, buildBinaryVariants(colored));

    if (gray.cols < 900 || gray.rows < 900) {
        cv::Mat enlarged;
        cv::resize(gray, enlarged, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
        candidates.push_back(enlarged);
        appendUniqueMats(candidates, buildBinaryVariants(enlarged));
    }

    return candidates;
}

std::vector<cv::Point> extractCornerPoints(const cv::Mat& cornersMat, int cols, int rows)
{
    std::vector<cv::Point> points;
    if (cornersMat.empty() || cols <= 0 || rows <= 0)
        return points;

    auto clampPoint = [cols, rows](int x, int y) {
        return cv::Point(
            std::clamp(x, 0, cols - 1),
            std::clamp(y, 0, rows - 1));
    };

    auto addPoint = [&](float x, float y) {
        points.emplace_back(clampPoint(
            static_cast<int>(std::lround(x)),
            static_cast<int>(std::lround(y))));
    };

    // OpenCV commonly returns 1x4 CV_32FC2 for four corners.
    if (cornersMat.rows == 1 && cornersMat.cols == 4 && cornersMat.type() == CV_32FC2) {
        points.reserve(4);
        for (int i = 0; i < 4; ++i) {
            const cv::Point2f pt = cornersMat.at<cv::Point2f>(0, i);
            addPoint(pt.x, pt.y);
        }
        return points;
    }

    if (cornersMat.rows == 4 && cornersMat.cols == 1 && cornersMat.type() == CV_32FC2) {
        points.reserve(4);
        for (int i = 0; i < 4; ++i) {
            const cv::Point2f pt = cornersMat.at<cv::Point2f>(i, 0);
            addPoint(pt.x, pt.y);
        }
        return points;
    }

    if (cornersMat.rows == 4 && cornersMat.cols == 2) {
        points.reserve(4);
        for (int i = 0; i < 4; ++i) {
            cv::Point2f pt;
            if (cornersMat.type() == CV_32FC2) {
                pt = cornersMat.at<cv::Point2f>(i, 0);
            } else if (cornersMat.type() == CV_32F) {
                pt.x = cornersMat.at<float>(i, 0);
                pt.y = cornersMat.at<float>(i, 1);
            } else if (cornersMat.type() == CV_32SC2) {
                const cv::Point ip = cornersMat.at<cv::Point>(i, 0);
                pt = cv::Point2f(static_cast<float>(ip.x), static_cast<float>(ip.y));
            } else {
                pt.x = static_cast<float>(cornersMat.at<int>(i, 0));
                pt.y = static_cast<float>(cornersMat.at<int>(i, 1));
            }
            addPoint(pt.x, pt.y);
        }
        return points;
    }

    if (cornersMat.rows == 1 && cornersMat.cols == 8 && cornersMat.type() == CV_32F) {
        points.reserve(4);
        for (int i = 0; i < 4; ++i) {
            addPoint(cornersMat.at<float>(0, i * 2), cornersMat.at<float>(0, i * 2 + 1));
        }
        return points;
    }

    if (cornersMat.checkVector(2, CV_32F) >= 4) {
        const cv::Point2f* pts = cornersMat.ptr<cv::Point2f>();
        points.reserve(4);
        for (int i = 0; i < 4; ++i)
            addPoint(pts[i].x, pts[i].y);
    }

    return points;
}

void rebuildCornersMat(const std::vector<cv::Point>& corners, cv::Mat& cornersMat)
{
    // OpenCV decode() expects 1x4 CV_32FC2 (not 4x2 CV_32F).
    cornersMat.create(1, 4, CV_32FC2);
    for (int i = 0; i < 4; ++i) {
        cornersMat.at<cv::Point2f>(0, i) = cv::Point2f(
            static_cast<float>(corners[i].x),
            static_cast<float>(corners[i].y));
    }
}

std::vector<cv::Point> scaleCornersToOriginal(const std::vector<cv::Point>& corners,
                                              const cv::Size& candidateSize,
                                              const cv::Size& originalSize)
{
    if (corners.size() < 4)
        return corners;
    if (candidateSize == originalSize)
        return corners;

    const float sx = static_cast<float>(originalSize.width) / candidateSize.width;
    const float sy = static_cast<float>(originalSize.height) / candidateSize.height;

    std::vector<cv::Point> scaled;
    scaled.reserve(4);
    for (const cv::Point& pt : corners) {
        scaled.emplace_back(
            std::clamp(static_cast<int>(std::lround(pt.x * sx)), 0, originalSize.width - 1),
            std::clamp(static_cast<int>(std::lround(pt.y * sy)), 0, originalSize.height - 1));
    }
    return scaled;
}

cv::Mat warpQrPatch(const cv::Mat& image, const std::vector<cv::Point>& corners, int size = 420)
{
    if (corners.size() < 4)
        return {};

    std::vector<cv::Point2f> src(4);
    for (int i = 0; i < 4; ++i)
        src[i] = cv::Point2f(static_cast<float>(corners[i].x), static_cast<float>(corners[i].y));

    const std::vector<cv::Point2f> dst = {
        cv::Point2f(0.f, static_cast<float>(size)),
        cv::Point2f(0.f, 0.f),
        cv::Point2f(static_cast<float>(size), 0.f),
        cv::Point2f(static_cast<float>(size), static_cast<float>(size))
    };

    cv::Mat transform = cv::getPerspectiveTransform(src, dst);
    cv::Mat warped;
    cv::warpPerspective(image, warped, transform, cv::Size(size, size),
                        cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    return warped;
}

void drawPolygon(cv::Mat& bgr, const std::vector<cv::Point>& points)
{
    if (points.size() < 4)
        return;

    for (size_t i = 0; i < 4; ++i) {
        cv::line(bgr, points[i], points[(i + 1) % 4],
                 cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }
}

int minCornerY(const std::vector<cv::Point>& points)
{
    int minY = points.front().y;
    for (size_t i = 1; i < points.size(); ++i)
        minY = std::min(minY, points[i].y);
    return minY;
}

int minCornerX(const std::vector<cv::Point>& points)
{
    int minX = points.front().x;
    for (size_t i = 1; i < points.size(); ++i)
        minX = std::min(minX, points[i].x);
    return minX;
}

struct QrDetectionResult {
    std::string decodedBytes;
    std::vector<cv::Point> corners;
};

std::string decodeWithCorners(cv::QRCodeDetector& detector,
                              const cv::Mat& image,
                              cv::Mat& cornersMat)
{
    if (cornersMat.empty())
        return {};

    cv::Mat straight;
    std::string decoded = detector.decode(image, cornersMat, straight);
    if (!decoded.empty())
        return decoded;

    if (!straight.empty()) {
        decoded = detector.detectAndDecode(straight);
        if (!decoded.empty())
            return decoded;

        const cv::Mat straightGray = toGray(straight);
        for (const cv::Mat& variant : buildBinaryVariants(straightGray)) {
            decoded = detector.detectAndDecode(variant);
            if (!decoded.empty())
                return decoded;
        }
    }

    return {};
}

std::string decodeWarpedPatch(cv::QRCodeDetector& detector, const cv::Mat& image,
                              const std::vector<cv::Point>& corners)
{
    const cv::Mat warped = warpQrPatch(image, corners);
    if (warped.empty())
        return {};

    cv::Mat patchCorners;
    std::string decoded = detector.detectAndDecode(warped, patchCorners);
    if (!decoded.empty())
        return decoded;

    for (const cv::Mat& variant : buildBinaryVariants(toGray(warped))) {
        decoded = detector.detectAndDecode(variant, patchCorners);
        if (!decoded.empty())
            return decoded;
    }

    return {};
}

QrDetectionResult tryDetectAndDecode(cv::QRCodeDetector& detector, const cv::Mat& image)
{
    QrDetectionResult result;
    cv::Mat cornersMat;
    result.decodedBytes = detector.detectAndDecode(image, cornersMat);
    result.corners = extractCornerPoints(cornersMat, image.cols, image.rows);

    if (result.decodedBytes.empty() && result.corners.size() >= 4 && !cornersMat.empty()) {
        cv::Mat cornersClone = cornersMat.clone();
        result.decodedBytes = decodeWithCorners(detector, image, cornersClone);
    }

    return result;
}

QrDetectionResult detectWithOpenCv(const cv::Mat& gray, const cv::Mat& bgr)
{
    QrDetectionResult best;
    cv::QRCodeDetector detector;
    const cv::Size originalSize(gray.cols, gray.rows);
    const std::vector<cv::Mat> candidates = buildDetectionCandidates(gray, bgr);

    // Fast path: standard black/white QR codes.
    for (const cv::Mat& image : {gray, bgr}) {
        cv::QRCodeDetector fastDetector;
        QrDetectionResult simple = tryDetectAndDecode(fastDetector, image);
        if (!simple.decodedBytes.empty())
            return simple;

        if (simple.corners.size() >= 4) {
            best.corners = simple.corners;
            cv::Mat cornersMat;
            rebuildCornersMat(simple.corners, cornersMat);
            cv::QRCodeDetector decodeDetector;
            best.decodedBytes = decodeWithCorners(decodeDetector, image, cornersMat);
            if (!best.decodedBytes.empty())
                return best;
        }
    }

    for (const cv::Mat& candidate : candidates) {
        cv::QRCodeDetector candidateDetector;
        cv::Mat cornersMat;
        if (!candidateDetector.detect(candidate, cornersMat))
            continue;

        std::vector<cv::Point> corners = extractCornerPoints(
            cornersMat, candidate.cols, candidate.rows);
        if (corners.size() < 4)
            continue;

        corners = scaleCornersToOriginal(corners, candidate.size(), originalSize);
        rebuildCornersMat(corners, cornersMat);

        if (best.corners.empty())
            best.corners = corners;

        for (const cv::Mat& decodeImage : candidates) {
            cv::QRCodeDetector decodeDetector;
            cv::Mat decodeCorners = cornersMat.clone();
            best.decodedBytes = decodeWithCorners(decodeDetector, decodeImage, decodeCorners);
            if (!best.decodedBytes.empty()) {
                best.corners = corners;
                return best;
            }
        }

        cv::QRCodeDetector warpDetector;
        best.decodedBytes = decodeWarpedPatch(warpDetector, gray, corners);
        if (!best.decodedBytes.empty()) {
            best.corners = corners;
            return best;
        }

        best.decodedBytes = decodeWarpedPatch(warpDetector, coloredQrGray(bgr), corners);
        if (!best.decodedBytes.empty()) {
            best.corners = corners;
            return best;
        }
    }

    cv::Mat curvedCorners;
    best.decodedBytes = detector.detectAndDecodeCurved(gray, curvedCorners);
    best.corners = extractCornerPoints(curvedCorners, gray.cols, gray.rows);
    if (!best.decodedBytes.empty())
        return best;

    for (const cv::Mat& candidate : candidates) {
        curvedCorners.release();
        best.decodedBytes = detector.detectAndDecodeCurved(candidate, curvedCorners);
        if (best.decodedBytes.empty())
            continue;

        best.corners = extractCornerPoints(
            curvedCorners, candidate.cols, candidate.rows);
        best.corners = scaleCornersToOriginal(best.corners, candidate.size(), originalSize);
        return best;
    }

    return best;
}

cv::wechat_qrcode::WeChatQRCode* weChatDetector()
{
    static std::unique_ptr<cv::wechat_qrcode::WeChatQRCode> detector;
    static bool initAttempted = false;
    if (initAttempted)
        return detector.get();

    initAttempted = true;
    const ModelPaths::WeChatQrModelPaths paths = ModelPaths::resolveWeChatQrModels();
    if (!paths.isValid())
        return nullptr;

    try {
        detector = std::make_unique<cv::wechat_qrcode::WeChatQRCode>(
            paths.detectProto.toStdString(),
            paths.detectModel.toStdString(),
            paths.srProto.toStdString(),
            paths.srModel.toStdString());
    } catch (...) {
        detector.reset();
    }

    return detector.get();
}

QrDetectionResult tryWeChatOnImage(cv::wechat_qrcode::WeChatQRCode* detector,
                                   const cv::Mat& image,
                                   const cv::Size& originalSize)
{
    QrDetectionResult result;
    if (!detector || image.empty())
        return result;

    std::vector<cv::Mat> pointsList;
    const std::vector<std::string> decodedList = detector->detectAndDecode(image, pointsList);
    if (decodedList.empty())
        return result;

    result.decodedBytes = decodedList.front();
    if (!pointsList.empty()) {
        result.corners = extractCornerPoints(pointsList.front(), image.cols, image.rows);
        result.corners = scaleCornersToOriginal(result.corners, image.size(), originalSize);
    }

    return result;
}

QrDetectionResult detectWithWeChat(const cv::Mat& gray, const cv::Mat& bgr)
{
    QrDetectionResult result;
    cv::wechat_qrcode::WeChatQRCode* detector = weChatDetector();
    if (!detector)
        return result;

    const cv::Size originalSize(bgr.cols, bgr.rows);
    std::vector<cv::Mat> candidates;
    candidates.push_back(bgr);

    const cv::Mat colored = coloredQrGray(bgr);
    if (!colored.empty()) {
        cv::Mat coloredBgr;
        cv::cvtColor(colored, coloredBgr, cv::COLOR_GRAY2BGR);
        candidates.push_back(coloredBgr);
    }

    if (bgr.cols < 900 || bgr.rows < 900) {
        cv::Mat enlarged;
        cv::resize(bgr, enlarged, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
        candidates.push_back(enlarged);
    }

    cv::Mat claheGray;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, claheGray);
    cv::Mat claheBgr;
    cv::cvtColor(claheGray, claheBgr, cv::COLOR_GRAY2BGR);
    candidates.push_back(claheBgr);

    for (const cv::Mat& candidate : candidates) {
        result = tryWeChatOnImage(detector, candidate, originalSize);
        if (!result.decodedBytes.empty())
            return result;
    }

    return result;
}

QrDetectionResult detectQr(const cv::Mat& gray, const cv::Mat& bgr)
{
    QrDetectionResult result = detectWithOpenCv(gray, bgr);
    if (!result.decodedBytes.empty())
        return result;

    const QrDetectionResult wechatResult = detectWithWeChat(gray, bgr);
    if (!wechatResult.decodedBytes.empty())
        return wechatResult;

    if (result.corners.empty() && !wechatResult.corners.empty())
        result.corners = wechatResult.corners;

    return result;
}

} // namespace

PluginMetadata QrCodeProcessor::metadata() const
{
    return PluginMetadata(
        "org.svw.qr_detection.qr",
        "QR Code Detection",
        "Detect and decode QR codes in the image",
        "QR Detection"
    );
}

QList<ParameterDescriptor> QrCodeProcessor::parameterDescriptors() const
{
    return {
        ParameterDescriptor::boolParam(
            "showDecodedText",
            "Show Decoded Text",
            true
        )
    };
}

void QrCodeProcessor::setParameter(const QString& id, const QVariant& value)
{
    if (id == "showDecodedText")
        m_showDecodedText = value.toBool();
}

QVariant QrCodeProcessor::parameter(const QString& id) const
{
    if (id == "showDecodedText")
        return m_showDecodedText;
    return {};
}

void QrCodeProcessor::resetParameters()
{
    m_showDecodedText = true;
}

ImageData QrCodeProcessor::process(const ImageData& input)
{
    if (input.isEmpty())
        return input;

    const cv::Mat gray = toGray(input.mat());
    const cv::Mat bgr = toBgr(input.mat());
    cv::Mat output = bgr.clone();

    const QrDetectionResult detection = detectQr(gray, bgr);
    const QString decodedText = decodeUtf8(detection.decodedBytes);

    if (detection.corners.size() >= 4)
        drawPolygon(output, detection.corners);

    ImageData result(output, input.sourceId());
    result.setTimestamp(input.timestamp());
    result.appendToHistory(metadata().id);

    if (!decodedText.isEmpty()) {
        result.setMetadata("decodedText", decodedText);

        if (m_showDecodedText && detection.corners.size() >= 4) {
            result.setMetadata("qr_label_x", std::max(0, minCornerX(detection.corners)));
            result.setMetadata("qr_label_y", std::max(0, minCornerY(detection.corners) - 8));
            result.setMetadata("qr_show_label", true);
        }
    } else if (detection.corners.size() >= 4) {
        result.setMetadata(
            "warning",
            QStringLiteral("QR code found but decoding failed. Colored or logo QR codes may need a clearer crop."));
    }

    return result;
}

QJsonObject QrCodeProcessor::parameterState() const
{
    QJsonObject state;
    state["showDecodedText"] = m_showDecodedText;
    return state;
}

void QrCodeProcessor::setParameterState(const QJsonObject& state)
{
    if (state.contains("showDecodedText"))
        m_showDecodedText = state["showDecodedText"].toBool();
}
