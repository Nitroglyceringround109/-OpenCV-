#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <QThread>
#include <QMutex>
#include "core/ImageData.h"

class CameraDevice;

/**
 * @brief Thread that pulls frames from CameraDevice and emits ImageData.
 *
 * Implements frame-dropping when the pipeline is busy.
 */
class FrameGrabber : public QObject
{
    Q_OBJECT

public:
    explicit FrameGrabber(QObject* parent = nullptr);

public slots:
    void onFrameCaptured(const cv::Mat& frame);

signals:
    void sourceFrameReady(const ImageData& data);

private:
    QAtomicInt m_frameCount;
};

#endif // FRAMEGRABBER_H
