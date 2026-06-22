#include "ImageFileSource.h"
#include <opencv2/imgcodecs.hpp>
#include <QDir>
#include <QFileInfo>

ImageFileSource::ImageFileSource(QObject* parent)
    : QObject(parent)
{
}

void ImageFileSource::loadFile(const QString& path)
{
    cv::Mat mat = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    if (mat.empty()) {
        emit loadError(tr("Failed to load image: %1").arg(path));
        return;
    }

    ImageData data(mat, QFileInfo(path).fileName());
    data.setMetadata("filePath", path);
    data.setMetadata("fileName", QFileInfo(path).fileName());
    emit imageLoaded(data);
}

void ImageFileSource::loadFolder(const QString& dirPath)
{
    QDir dir(dirPath);
    QStringList nameFilters = supportedFormats();
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        emit loadError(tr("No images found in: %1").arg(dirPath));
        return;
    }

    // Load the first image
    loadFile(files.first().absoluteFilePath());
}

QStringList ImageFileSource::supportedFormats() const
{
    return {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tiff", "*.tif", "*.webp", "*.pbm", "*.pgm", "*.ppm"};
}
