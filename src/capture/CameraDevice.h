#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QAtomicInt>
#include <opencv2/videoio.hpp>
#include "core/ImageData.h"

/**
 * @brief Camera capture using OpenCV's VideoCapture.
 *
 * Runs a capture loop in a background thread, emitting frames
 * as cv::Mat. Supports camera enumeration, start/stop, and
 * frame-dropping when the pipeline is busy.
 */
class CameraDevice : public QObject
{
    Q_OBJECT

public:
    explicit CameraDevice(QObject* parent = nullptr);
    ~CameraDevice();

    QStringList availableCameras() const;
    QList<int> availableCameraIndices() const;
    bool isRunning() const;

public slots:
    void start(int deviceIndex = 0);
    void stop();

signals:
    void frameCaptured(const cv::Mat& frame);
    void cameraError(const QString& message);
    void cameraStarted();
    void cameraStopped();

private slots:
    void captureLoop();

private:
    cv::VideoCapture m_capture;
    QThread* m_thread = nullptr;
    QAtomicInt m_running;
    int m_deviceIndex = 0;
};

#endif // CAMERADEVICE_H
