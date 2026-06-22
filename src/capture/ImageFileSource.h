#ifndef IMAGEFILESOURCE_H
#define IMAGEFILESOURCE_H

#include <QObject>
#include <QStringList>
#include "core/ImageData.h"

/**
 * @brief Loads single images or image sequences from disk.
 */
class ImageFileSource : public QObject
{
    Q_OBJECT

public:
    explicit ImageFileSource(QObject* parent = nullptr);

public slots:
    void loadFile(const QString& path);
    void loadFolder(const QString& dirPath);

signals:
    void imageLoaded(const ImageData& data);
    void loadError(const QString& message);

private:
    QStringList supportedFormats() const;
};

#endif // IMAGEFILESOURCE_H
