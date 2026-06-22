#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <QDateTime>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <opencv2/core.hpp>
#include <memory>

/**
 * @brief Value object wrapping cv::Mat plus metadata.
 *
 * This is the domain-level representation of an image that flows
 * through the processing pipeline. It carries the actual pixel data
 * (cv::Mat) and rich metadata (source, timestamp, processing history).
 *
 * Uses explicit deep copy to make ownership semantics clear.
 */
class ImageData
{
public:
    ImageData();
    explicit ImageData(const cv::Mat& mat);
    ImageData(const cv::Mat& mat, const QString& sourceId);

    /// Deep copy constructor
    ImageData(const ImageData& other);
    ImageData& operator=(const ImageData& other);
    ImageData(ImageData&& other) noexcept;
    ImageData& operator=(ImageData&& other) noexcept;

    ~ImageData() = default;

    // --- Accessors ---
    const cv::Mat& mat() const { return m_mat; }
    cv::Mat& mat() { return m_mat; }

    bool isEmpty() const { return m_mat.empty(); }
    int width() const { return m_mat.cols; }
    int height() const { return m_mat.rows; }
    int type() const { return m_mat.type(); }
    int channels() const { return m_mat.channels(); }

    const QString& sourceId() const { return m_sourceId; }
    void setSourceId(const QString& id) { m_sourceId = id; }

    const QDateTime& timestamp() const { return m_timestamp; }
    void setTimestamp(const QDateTime& ts) { m_timestamp = ts; }

    // --- Metadata ---
    void setMetadata(const QString& key, const QVariant& value);
    QVariant metadata(const QString& key) const;
    const QMap<QString, QVariant>& allMetadata() const { return m_metadata; }

    // --- Processing history ---
    void appendToHistory(const QString& processorId);
    const QStringList& processingHistory() const { return m_history; }

    /// Create a deep copy of this ImageData
    ImageData deepCopy() const;

private:
    cv::Mat m_mat;
    QString m_sourceId;
    QDateTime m_timestamp;
    QMap<QString, QVariant> m_metadata;
    QStringList m_history;  ///< Ordered list of processor IDs that have been applied
};

Q_DECLARE_METATYPE(ImageData)
Q_DECLARE_METATYPE(cv::Mat)

#endif // IMAGEDATA_H
