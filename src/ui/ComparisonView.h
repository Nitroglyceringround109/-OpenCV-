#ifndef COMPARISONVIEW_H
#define COMPARISONVIEW_H

#include <QWidget>
#include <QSplitter>
#include "core/ImageData.h"
#include "ImageCanvas.h"

/**
 * @brief Container widget with three display modes for before/after comparison.
 *
 * Modes:
 * - SideBySide: Two ImageCanvas widgets in a QSplitter
 * - Overlay: Single canvas with a draggable vertical divider
 * - Diff: Single canvas showing cv::absdiff(before, after)
 */
class ComparisonView : public QWidget
{
    Q_OBJECT

public:
    enum CompareMode {
        SideBySide,
        Overlay,
        Diff
    };
    Q_ENUM(CompareMode)

    explicit ComparisonView(QWidget* parent = nullptr);

    void setCompareMode(CompareMode mode);
    CompareMode compareMode() const { return m_mode; }

public slots:
    void setBeforeImage(const ImageData& before);
    void setAfterImage(const ImageData& after);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateDisplay();
    QImage computeDiffImage(const QImage& before, const QImage& after) const;

    CompareMode m_mode = SideBySide;
    ImageCanvas* m_beforeCanvas = nullptr;
    ImageCanvas* m_afterCanvas = nullptr;
    ImageCanvas* m_overlayCanvas = nullptr;
    QSplitter* m_splitter = nullptr;

    ImageData m_beforeData;
    ImageData m_afterData;
    int m_dividerX = -1;  ///< Overlay divider position
    bool m_draggingDivider = false;
};

#endif // COMPARISONVIEW_H
