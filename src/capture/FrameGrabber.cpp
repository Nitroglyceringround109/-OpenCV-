#include "FrameGrabber.h"

FrameGrabber::FrameGrabber(QObject* parent)
    : QObject(parent)
    , m_frameCount(0)
{
}

void FrameGrabber::onFrameCaptured(const cv::Mat& frame)
{
    ImageData data(frame, "camera");
    data.setMetadata("frameNumber", m_frameCount.fetchAndAddRelaxed(1));
    emit sourceFrameReady(data);
}
