#include "HistoryPanel.h"
#include "utils/AppSettings.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QEvent>
#include <opencv2/imgproc.hpp>

namespace {
ImageData createHistoryPreview(const ImageData& state, int maxDimension = 256)
{
    if (state.isEmpty())
        return state;

    const cv::Mat& mat = state.mat();
    const int maxSide = std::max(mat.cols, mat.rows);
    if (maxSide <= maxDimension)
        return state;

    const double scale = static_cast<double>(maxDimension) / maxSide;
    cv::Mat previewMat;
    cv::resize(mat, previewMat, cv::Size(), scale, scale, cv::INTER_AREA);

    ImageData preview(previewMat, state.sourceId());
    preview.setTimestamp(state.timestamp());
    return preview;
}
} // namespace

HistoryPanel::HistoryPanel(QWidget* parent)
    : QDockWidget(tr("History"), parent)
{
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);

    m_listWidget = new QListWidget(container);
    m_listWidget->setAlternatingRowColors(true);
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &HistoryPanel::onItemClicked);

    layout->addWidget(m_listWidget);
    setWidget(container);
}

void HistoryPanel::pushState(const ImageData& state, const QString& label)
{
    const int maxItems = AppSettings::historyMaxItems();

    QString text = label.isEmpty()
        ? QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
        : label;
    m_listWidget->addItem(text);
    m_states.append(createHistoryPreview(state));

    while (m_states.size() > maxItems) {
        delete m_listWidget->takeItem(0);
        m_states.removeFirst();
    }
}

void HistoryPanel::clear()
{
    m_listWidget->clear();
    m_states.clear();
}

void HistoryPanel::onItemClicked(QListWidgetItem* item)
{
    int row = m_listWidget->row(item);
    if (row >= 0 && row < m_states.size()) {
        emit stateRestored(m_states[row]);
    }
}

void HistoryPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        setWindowTitle(tr("History"));
    QDockWidget::changeEvent(event);
}
