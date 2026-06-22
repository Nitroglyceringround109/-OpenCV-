#ifndef CVMATTOQIMAGE_H
#define CVMATTOQIMAGE_H

#include <QImage>
#include <opencv2/core.hpp>

/**
 * @brief Convert between cv::Mat and QImage with correct format detection.
 *
 * Handles BGR, RGB, grayscale, and RGBA formats.
 * Uses cv::Mat::data pointer with QImage constructor for zero-copy
 * when possible (with proper lifetime management via deep copy fallback).
 */
namespace CvMatToQImage {

/// Convert cv::Mat (BGR, RGB, grayscale, RGBA) to QImage.
/// If copyData is false, the QImage shares data with the cv::Mat
/// (the cv::Mat must outlive the QImage).
QImage convert(const cv::Mat& mat, bool copyData = true);

/// Convert QImage to cv::Mat (BGR format for OpenCV processing).
cv::Mat fromQImage(const QImage& image);

} // namespace CvMatToQImage

#endif // CVMATTOQIMAGE_H
