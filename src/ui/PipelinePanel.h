#ifndef PIPELINEPANEL_H
#define PIPELINEPANEL_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include "core/PipelineNode.h"

/**
 * @brief Dock widget showing active pipeline nodes.
 *
 * Displays processor list with enable checkboxes and remove buttons.
 * Supports drag-and-drop reordering.
 * Emits signals for move/toggle/select/remove actions.
 */
class PipelinePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit PipelinePanel(QWidget* parent = nullptr);

public slots:
    /// Rebuild the list from current pipeline nodes
    void rebuildList(const QList<PipelineNode>& nodes);

signals:
    void processorMoved(int from, int to);
    void processorToggled(int index, bool enabled);
    void processorSelected(int index);
    void processorRemoved(int index);
    void addProcessorRequested();

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    void onItemChanged(QListWidgetItem* item);
    void onItemSelectionChanged();

    QListWidget* m_listWidget = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
};

#endif // PIPELINEPANEL_H
