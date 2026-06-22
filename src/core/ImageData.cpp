#include "ImageData.h"

ImageData::ImageData()
    : m_timestamp(QDateTime::currentDateTime())
{
}

ImageData::ImageData(const cv::Mat& mat)
    : m_mat(mat.clone())
    , m_timestamp(QDateTime::currentDateTime())
{
}

ImageData::ImageData(const cv::Mat& mat, const QString& sourceId)
    : m_mat(mat.clone())
    , m_sourceId(sourceId)
    , m_timestamp(QDateTime::currentDateTime())
{
}

ImageData::ImageData(const ImageData& other)
    : m_mat(other.m_mat.clone())
    , m_sourceId(other.m_sourceId)
    , m_timestamp(other.m_timestamp)
    , m_metadata(other.m_metadata)
    , m_history(other.m_history)
{
}

ImageData& ImageData::operator=(const ImageData& other)
{
    if (this != &other) {
        m_mat = other.m_mat.clone();
        m_sourceId = other.m_sourceId;
        m_timestamp = other.m_timestamp;
        m_metadata = other.m_metadata;
        m_history = other.m_history;
    }
    return *this;
}

ImageData::ImageData(ImageData&& other) noexcept
    : m_mat(std::move(other.m_mat))
    , m_sourceId(std::move(other.m_sourceId))
    , m_timestamp(std::move(other.m_timestamp))
    , m_metadata(std::move(other.m_metadata))
    , m_history(std::move(other.m_history))
{
}

ImageData& ImageData::operator=(ImageData&& other) noexcept
{
    if (this != &other) {
        m_mat = std::move(other.m_mat);
        m_sourceId = std::move(other.m_sourceId);
        m_timestamp = std::move(other.m_timestamp);
        m_metadata = std::move(other.m_metadata);
        m_history = std::move(other.m_history);
    }
    return *this;
}

void ImageData::setMetadata(const QString& key, const QVariant& value)
{
    m_metadata[key] = value;
}

QVariant ImageData::metadata(const QString& key) const
{
    return m_metadata.value(key);
}

void ImageData::appendToHistory(const QString& processorId)
{
    m_history.append(processorId);
}

ImageData ImageData::deepCopy() const
{
    return ImageData(*this);
}
