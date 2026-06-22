#include "ComparisonView.h"
#include "utils/CvMatToQImage.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <opencv2/imgproc.hpp>

ComparisonView::ComparisonView(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_beforeCanvas = new ImageCanvas(this);
    m_afterCanvas = new ImageCanvas(this);
    m_overlayCanvas = new ImageCanvas(this);

    m_beforeCanvas->setToolTip(tr("Original Image"));
    m_afterCanvas->setToolTip(tr("Processed Image"));

    m_splitter->addWidget(m_beforeCanvas);
    m_splitter->addWidget(m_afterCanvas);

    layout->addWidget(m_splitter);
    layout->addWidget(m_overlayCanvas);
    m_overlayCanvas->hide();

    setMouseTracking(true);
    m_dividerX = width() / 2;
}

void ComparisonView::setCompareMode(CompareMode mode)
{
    m_mode = mode;
    switch (mode) {
    case SideBySide:
        m_splitter->show();
        m_overlayCanvas->hide();
        setMouseTracking(false);
        break;
    case Overlay:
        m_splitter->hide();
        m_overlayCanvas->hide();
        setMouseTracking(true);
        m_dividerX = width() / 2;
        break;
    case Diff:
        m_splitter->hide();
        m_overlayCanvas->show();
        setMouseTracking(false);
        break;
    }
    updateDisplay();
}

void ComparisonView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_mode == Overlay && m_dividerX < 0)
        m_dividerX = width() / 2;
    update();
}

void ComparisonView::setBeforeImage(const ImageData& before)
{
    m_beforeData = before;
    updateDisplay();
}

void ComparisonView::setAfterImage(const ImageData& after)
{
    m_afterData = after;
    updateDisplay();
}

void ComparisonView::updateDisplay()
{
    if (m_beforeData.isEmpty() && m_afterData.isEmpty())
        return;

    switch (m_mode) {
    case SideBySide:
        m_beforeCanvas->displayImage(m_beforeData);
        m_afterCanvas->displayImage(m_afterData);
        break;
    case Overlay:
        update();
        break;
    case Diff: {
        if (!m_beforeData.isEmpty() && !m_afterData.isEmpty()) {
            QImage beforeQImg = CvMatToQImage::convert(m_beforeData.mat());
            QImage afterQImg = CvMatToQImage::convert(m_afterData.mat());
            QImage diffImg = computeDiffImage(beforeQImg, afterQImg);
            m_overlayCanvas->displayQImage(diffImg);
        }
        break;
    }
    }
}

QImage ComparisonView::computeDiffImage(const QImage& before, const QImage& after) const
{
    if (before.size() != after.size())
        return QImage();

    cv::Mat beforeMat = CvMatToQImage::fromQImage(before);
    cv::Mat afterMat = CvMatToQImage::fromQImage(after);

    // Ensure same size
    if (beforeMat.size() != afterMat.size())
        return QImage();

    cv::Mat diff;
    cv::absdiff(beforeMat, afterMat, diff);

    // Amplify the difference for visibility
    cv::Mat amplified;
    diff.convertTo(amplified, -1, 3.0, 0);

    return CvMatToQImage::convert(amplified);
}

void ComparisonView::paintEvent(QPaintEvent* event)
{
    if (m_mode == Overlay && !m_beforeData.isEmpty() && !m_afterData.isEmpty()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Draw after image on the right side of divider
        QImage afterImg = CvMatToQImage::convert(m_afterData.mat());
        QImage beforeImg = CvMatToQImage::convert(m_beforeData.mat());

        // Clip and draw before (left)
        painter.setClipRect(0, 0, m_dividerX, height());
        QSize scaledSize = afterImg.size().scaled(size(), Qt::KeepAspectRatio);
        QPoint topLeft((width() - scaledSize.width()) / 2,
                       (height() - scaledSize.height()) / 2);
        painter.drawImage(QRect(topLeft, scaledSize), beforeImg);

        // Clip and draw after (right)
        painter.setClipRect(m_dividerX, 0, width() - m_dividerX, height());
        painter.drawImage(QRect(topLeft, scaledSize), afterImg);

        // Draw divider line
        painter.setClipping(false);
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(m_dividerX, 0, m_dividerX, height());

        // Draw handle
        painter.setBrush(Qt::red);
        painter.drawEllipse(QPoint(m_dividerX, height() / 2), 6, 6);
    } else {
        QWidget::paintEvent(event);
    }
}

void ComparisonView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_draggingDivider) {
        m_dividerX = std::clamp(event->pos().x(), 0, width());
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void ComparisonView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_draggingDivider) {
        m_draggingDivider = false;
        unsetCursor();
    }
    QWidget::mouseReleaseEvent(event);
}

void ComparisonView::mousePressEvent(QMouseEvent* event)
{
    if (m_mode == Overlay && qAbs(event->pos().x() - m_dividerX) < 10) {
        m_draggingDivider = true;
        setCursor(Qt::SplitHCursor);
    }
    QWidget::mousePressEvent(event);
}

void ComparisonView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        if (m_beforeCanvas)
            m_beforeCanvas->setToolTip(tr("Original Image"));
        if (m_afterCanvas)
            m_afterCanvas->setToolTip(tr("Processed Image"));
    }
    QWidget::changeEvent(event);
}
