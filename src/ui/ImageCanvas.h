#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>
#include <QImage>
#include <QPointF>
#include <QString>
#include <QList>
#include "core/ImageData.h"

class QPainter;

/**
 * @brief Custom QWidget for high-performance image display.
 *
 * Supports pan (left-click drag), zoom (scroll wheel at cursor), fit-to-window.
 * Converts cv::Mat to QImage and paints via QPainter.
 * Right-click drag selects an ROI.
 */
class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget* parent = nullptr);

    /// Get the currently displayed image
    QImage currentImage() const { return m_image; }

    /// Get current zoom factor
    double zoomFactor() const { return m_zoom; }

    /// Check if an image is loaded
    bool hasImage() const { return !m_image.isNull(); }

public slots:
    /// Display an ImageData (converts cv::Mat to QImage internally)
    void displayImage(const ImageData& data);

    /// Display a QImage directly
    void displayQImage(const QImage& image);

    /// Fit image to window
    void fitToWindow();

    /// Reset zoom to 1:1
    void resetZoom();

    /// Zoom in by 25%
    void zoomIn();

    /// Zoom out by 25%
    void zoomOut();

    /// Set zoom factor explicitly
    void setZoom(double factor);

    /// Zoom while keeping a widget point anchored to the same image pixel
    void zoomAt(const QPoint& widgetPos, double factor);

    /// Clear the canvas
    void clear();

signals:
    void zoomChanged(double factor);
    void roiSelected(const QRect& roi);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    QPoint imageOrigin() const;
    void updateScrollBars();
    QPoint imageToWidget(const QPointF& imgPos) const;
    QPointF widgetToImage(const QPoint& widgetPos) const;
    void updateTextOverlays(const ImageData& data);
    void drawTextOverlays(QPainter& painter) const;

    struct TextOverlay {
        QString text;
        QPointF imagePos;
    };

    cv::Mat m_displayMat;
    QImage m_image;
    QList<TextOverlay> m_textOverlays;
    double m_zoom = 1.0;
    QPoint m_offset;          ///< Pan offset
    QPoint m_lastPanPos;      ///< Last pan position
    bool m_isPanning = false;

    // ROI selection
    QPoint m_roiStart;
    QPoint m_roiEnd;
    bool m_isSelectingRoi = false;
};

#endif // IMAGECANVAS_H
