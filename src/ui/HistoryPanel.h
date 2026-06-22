#ifndef HISTORYPANEL_H
#define HISTORYPANEL_H

#include <QDockWidget>
#include <QListWidget>
#include <QUndoStack>
#include "core/ImageData.h"

/**
 * @brief Dock widget showing processing history for undo/redo.
 */
class HistoryPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void pushState(const ImageData& state, const QString& label = {});
    void clear();

protected:
    void changeEvent(QEvent* event) override;

signals:
    void stateRestored(const ImageData& state);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* m_listWidget = nullptr;
    QList<ImageData> m_states;
};

#endif // HISTORYPANEL_H
