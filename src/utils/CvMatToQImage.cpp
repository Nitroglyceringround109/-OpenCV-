#include "CvMatToQImage.h"
#include <opencv2/imgproc.hpp>

namespace CvMatToQImage {

QImage convert(const cv::Mat& mat, bool copyData)
{
    if (mat.empty())
        return QImage();

    cv::Mat temp;
    QImage::Format format;

    switch (mat.type()) {
    case CV_8UC1:
        // Grayscale -> QImage::Format_Grayscale8
        format = QImage::Format_Grayscale8;
        temp = mat;
        break;

    case CV_8UC3:
        // BGR -> RGB conversion
        cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
        break;

    case CV_8UC4:
        // BGRA -> RGBA conversion
        cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
        format = QImage::Format_RGBA8888;
        break;

    default:
        // Unsupported format, try converting to BGR first
        if (mat.channels() == 1) {
            cv::cvtColor(mat, temp, cv::COLOR_GRAY2BGR);
            cv::cvtColor(temp, temp, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        } else if (mat.channels() == 3) {
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        } else if (mat.channels() == 4) {
            cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
            format = QImage::Format_RGBA8888;
        } else {
            qWarning("Unsupported cv::Mat format: type=%d channels=%d",
                     mat.type(), mat.channels());
            return QImage();
        }
        break;
    }

    // Calculate bytes per line (OpenCV may pad rows to 4-byte boundary)
    const int bytesPerLine = static_cast<int>(temp.step);

    QImage image;
    if (copyData) {
        // Deep copy - QImage owns the data
        image = QImage(temp.data, temp.cols, temp.rows, bytesPerLine, format).copy();
    } else {
        // Shared data - cv::Mat must outlive QImage
        image = QImage(temp.data, temp.cols, temp.rows, bytesPerLine, format);
    }

    return image;
}

cv::Mat fromQImage(const QImage& image)
{
    if (image.isNull())
        return cv::Mat();

    QImage temp;
    switch (image.format()) {
    case QImage::Format_RGB888:
        temp = image;
        break;
    case QImage::Format_RGBA8888:
        temp = image;
        break;
    case QImage::Format_Grayscale8:
        temp = image;
        break;
    default:
        // Convert to a format we can handle
        temp = image.convertToFormat(QImage::Format_RGB888);
        break;
    }

    cv::Mat mat;
    const int bytesPerLine = static_cast<int>(temp.bytesPerLine());

    switch (temp.format()) {
    case QImage::Format_RGB888:
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC3,
                      const_cast<uchar*>(temp.bits()), bytesPerLine);
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;
    case QImage::Format_RGBA8888:
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC4,
                      const_cast<uchar*>(temp.bits()), bytesPerLine);
        cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
        break;
    case QImage::Format_Grayscale8:
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC1,
                      const_cast<uchar*>(temp.bits()), bytesPerLine).clone();
        break;
    default:
        break;
    }

    return mat;
}

} // namespace CvMatToQImage
