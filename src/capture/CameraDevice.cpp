#include "CameraDevice.h"
#include "utils/AppSettings.h"
#include <QDebug>

CameraDevice::CameraDevice(QObject* parent)
    : QObject(parent)
    , m_running(0)
{
}

CameraDevice::~CameraDevice()
{
    stop();
}

QStringList CameraDevice::availableCameras() const
{
    QStringList list;
    for (int index : availableCameraIndices())
        list << tr("Camera %1").arg(index);
    if (list.isEmpty())
        list << tr("No camera found");
    return list;
}

QList<int> CameraDevice::availableCameraIndices() const
{
    QList<int> indices;
    for (int i = 0; i < 10; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            indices << i;
            cap.release();
        }
    }
    return indices;
}

bool CameraDevice::isRunning() const
{
    return m_running.loadRelaxed();
}

void CameraDevice::start(int deviceIndex)
{
    if (m_running.loadRelaxed()) stop();

    m_deviceIndex = deviceIndex;
    m_running.storeRelaxed(1);

    m_thread = QThread::create([this]() { captureLoop(); });
    m_thread->start();
    emit cameraStarted();
}

void CameraDevice::stop()
{
    m_running.storeRelaxed(0);
    if (m_thread) {
        m_thread->wait(3000);
        delete m_thread;
        m_thread = nullptr;
    }
    if (m_capture.isOpened()) {
        m_capture.release();
    }
    emit cameraStopped();
}

void CameraDevice::captureLoop()
{
    m_capture.open(m_deviceIndex, cv::CAP_DSHOW);
    if (!m_capture.isOpened()) {
        qWarning() << "Failed to open camera" << m_deviceIndex;
        emit cameraError(tr("Failed to open camera %1").arg(m_deviceIndex));
        m_running.storeRelaxed(0);
        return;
    }

    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, AppSettings::cameraWidth());
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, AppSettings::cameraHeight());

    cv::Mat frame;
    while (m_running.loadRelaxed()) {
        if (!m_capture.read(frame) || frame.empty()) {
            qWarning() << "Failed to read frame from camera";
            QThread::msleep(33);
            continue;
        }
        emit frameCaptured(frame);
        // Target ~30 fps
        QThread::msleep(10);
    }

    m_capture.release();
}
