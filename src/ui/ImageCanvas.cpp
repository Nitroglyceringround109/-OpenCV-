#include "ImageCanvas.h"
#include "utils/CvMatToQImage.h"
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>

ImageCanvas::ImageCanvas(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(200, 150);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void ImageCanvas::updateTextOverlays(const ImageData& data)
{
    m_textOverlays.clear();

    const QString decodedText = data.metadata("decodedText").toString();
    if (decodedText.isEmpty() || !data.metadata("qr_show_label").toBool())
        return;

    const int labelX = data.metadata("qr_label_x").toInt();
    const int labelY = data.metadata("qr_label_y").toInt();
    m_textOverlays.append({decodedText, QPointF(labelX, labelY)});
}

void ImageCanvas::drawTextOverlays(QPainter& painter) const
{
    if (m_textOverlays.isEmpty())
        return;

    QFont font = painter.font();
    font.setFamilies({
        QStringLiteral("Microsoft YaHei"),
        QStringLiteral("PingFang SC"),
        QStringLiteral("Noto Sans CJK SC"),
        QStringLiteral("Segoe UI")
    });
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    const int maxTextWidth = std::max(120, static_cast<int>(m_image.width() * m_zoom * 0.6));

    for (const TextOverlay& overlay : m_textOverlays) {
        const QPoint anchor = imageToWidget(overlay.imagePos);
        QFontMetrics fm(font);
        const QRect bounds = fm.boundingRect(
            QRect(0, 0, maxTextWidth, 10000),
            Qt::TextWordWrap,
            overlay.text);

        QRect bgRect = bounds;
        bgRect.moveTopLeft(anchor - QPoint(0, bounds.height()));
        bgRect.adjust(-6, -4, 6, 4);
        bgRect = bgRect.intersected(rect());

        painter.fillRect(bgRect, QColor(0, 0, 0, 210));
        painter.setPen(QColor(0, 255, 0));
        painter.drawText(
            bgRect.adjusted(6, 4, -6, -4),
            Qt::AlignLeft | Qt::TextWordWrap,
            overlay.text);
    }
}

void ImageCanvas::displayImage(const ImageData& data)
{
    if (data.isEmpty()) return;

    const cv::Size newSize = data.mat().size();
    const bool sameGeometry = !m_displayMat.empty()
        && m_displayMat.size() == newSize
        && m_displayMat.type() == data.mat().type();

    m_displayMat = data.mat().clone();
    m_image = CvMatToQImage::convert(m_displayMat, true);
    updateTextOverlays(data);
    if (!sameGeometry)
        fitToWindow();
    else
        repaint();
}

void ImageCanvas::displayQImage(const QImage& image)
{
    const bool sameGeometry = !m_image.isNull()
        && m_image.size() == image.size()
        && m_image.format() == image.format();

    m_image = image.copy();
    if (!sameGeometry)
        fitToWindow();
    else
        update();
}

void ImageCanvas::fitToWindow()
{
    if (m_image.isNull()) return;

    double scaleX = static_cast<double>(width()) / m_image.width();
    double scaleY = static_cast<double>(height()) / m_image.height();
    m_zoom = std::min(scaleX, scaleY);
    m_offset = QPoint(0, 0);
    emit zoomChanged(m_zoom);
    update();
}

void ImageCanvas::resetZoom()
{
    m_zoom = 1.0;
    m_offset = QPoint(0, 0);
    emit zoomChanged(m_zoom);
    update();
}

void ImageCanvas::zoomIn()
{
    zoomAt(rect().center(), 1.25);
}

void ImageCanvas::zoomOut()
{
    zoomAt(rect().center(), 1.0 / 1.25);
}

void ImageCanvas::setZoom(double factor)
{
    factor = std::clamp(factor, 0.05, 50.0);
    m_zoom = factor;
    emit zoomChanged(m_zoom);
    update();
}

void ImageCanvas::zoomAt(const QPoint& widgetPos, double factor)
{
    if (m_image.isNull()) return;

    const QPointF imagePos = widgetToImage(widgetPos);
    const double newZoom = std::clamp(m_zoom * factor, 0.05, 50.0);
    if (qFuzzyCompare(newZoom, m_zoom))
        return;

    const QPoint origin = imageOrigin();
    m_offset = widgetPos - origin - QPoint(
        static_cast<int>(imagePos.x() * newZoom),
        static_cast<int>(imagePos.y() * newZoom));
    m_zoom = newZoom;
    emit zoomChanged(m_zoom);
    update();
}

void ImageCanvas::clear()
{
    m_displayMat.release();
    m_image = QImage();
    m_textOverlays.clear();
    m_zoom = 1.0;
    m_offset = QPoint(0, 0);
    update();
}

QPoint ImageCanvas::imageOrigin() const
{
    const QSize scaledSize(
        static_cast<int>(m_image.width() * m_zoom),
        static_cast<int>(m_image.height() * m_zoom));
    return QPoint(
        (width() - scaledSize.width()) / 2 + m_offset.x(),
        (height() - scaledSize.height()) / 2 + m_offset.y());
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), QColor(30, 30, 30));

    if (m_image.isNull()) {
        painter.setPen(QColor(100, 100, 100));
        painter.setFont(QFont("Segoe UI", 12));
        painter.drawText(rect(), Qt::AlignCenter, tr("No Image Loaded"));
        return;
    }

    const QSize scaledSize(
        static_cast<int>(m_image.width() * m_zoom),
        static_cast<int>(m_image.height() * m_zoom));
    const QPoint origin = imageOrigin();

    painter.drawImage(QRect(origin, scaledSize), m_image);
    drawTextOverlays(painter);

    if (m_isSelectingRoi) {
        painter.setPen(QPen(Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        QRect roi(m_roiStart, m_roiEnd);
        painter.drawRect(roi);
    }
}

void ImageCanvas::wheelEvent(QWheelEvent* event)
{
    if (m_image.isNull()) return;

    double steps = 0.0;
    const QPoint pixelDelta = event->pixelDelta();
    const QPoint angleDelta = event->angleDelta();

    if (!pixelDelta.isNull()) {
        // High-resolution touchpads report pixel deltas; invert so forward scroll zooms in.
        steps = -pixelDelta.y() / 120.0;
        if (qFuzzyIsNull(steps))
            steps = -pixelDelta.x() / 120.0;
    }
    if (qFuzzyIsNull(steps)) {
        steps = angleDelta.y() / 120.0;
        if (qFuzzyIsNull(steps))
            steps = angleDelta.x() / 120.0;
    }
    if (qFuzzyIsNull(steps))
        return;

    const double factor = std::pow(1.15, steps);
    zoomAt(event->position().toPoint(), factor);
    event->accept();
}

void ImageCanvas::mousePressEvent(QMouseEvent* event)
{
    if (m_image.isNull()) return;

    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        m_isSelectingRoi = true;
        m_roiStart = event->pos();
        m_roiEnd = event->pos();
        event->accept();
    }
}

void ImageCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        m_offset += event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();
        update();
        event->accept();
    } else if (m_isSelectingRoi) {
        m_roiEnd = event->pos();
        update();
        event->accept();
    } else if (hasImage()) {
        setCursor(Qt::OpenHandCursor);
    }
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isPanning && event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(hasImage() ? Qt::OpenHandCursor : Qt::ArrowCursor);
        event->accept();
    } else if (m_isSelectingRoi && event->button() == Qt::RightButton) {
        m_isSelectingRoi = false;
        QRect roi(m_roiStart, m_roiEnd);
        if (roi.width() > 5 && roi.height() > 5)
            emit roiSelected(roi);
        update();
        event->accept();
    }
}

void ImageCanvas::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

QPoint ImageCanvas::imageToWidget(const QPointF& imgPos) const
{
    const QPoint origin = imageOrigin();
    return QPoint(
        origin.x() + static_cast<int>(imgPos.x() * m_zoom),
        origin.y() + static_cast<int>(imgPos.y() * m_zoom));
}

QPointF ImageCanvas::widgetToImage(const QPoint& widgetPos) const
{
    const QPoint origin = imageOrigin();
    return QPointF(
        (widgetPos.x() - origin.x()) / m_zoom,
        (widgetPos.y() - origin.y()) / m_zoom);
}

void ImageCanvas::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange && m_image.isNull())
        update();
    QWidget::changeEvent(event);
}
