#include "PipelinePanel.h"
#include "utils/ProcessorLocalization.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QToolButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>

PipelinePanel::PipelinePanel(QWidget* parent)
    : QDockWidget(tr("Processing Pipeline"), parent)
{
    setupUi();
}

void PipelinePanel::setupUi()
{
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);

    m_listWidget = new QListWidget(container);
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setAlternatingRowColors(true);

    connect(m_listWidget, &QListWidget::itemChanged,
            this, &PipelinePanel::onItemChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &PipelinePanel::onItemSelectionChanged);

    // Handle drag-drop reorder
    connect(m_listWidget->model(), &QAbstractItemModel::rowsMoved,
            this, [this](const QModelIndex&, int start, int, const QModelIndex&, int end) {
        emit processorMoved(start, end);
    });

    layout->addWidget(m_listWidget);

    // Add processor button
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addBtn = new QPushButton(tr("+ Add Processor"), container);
    m_addBtn->setToolTip(tr("Add a new processor to the pipeline"));
    connect(m_addBtn, &QPushButton::clicked,
            this, &PipelinePanel::addProcessorRequested);
    btnLayout->addWidget(m_addBtn);

    m_clearBtn = new QPushButton(tr("Clear All"), container);
    m_clearBtn->setToolTip(tr("Remove all processors from the pipeline"));
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        while (m_listWidget->count() > 0) {
            emit processorRemoved(0);
            delete m_listWidget->takeItem(0);
        }
    });
    btnLayout->addWidget(m_clearBtn);

    layout->addLayout(btnLayout);
    setWidget(container);
}

void PipelinePanel::rebuildList(const QList<PipelineNode>& nodes)
{
    m_listWidget->blockSignals(true);
    m_listWidget->clear();

    for (int i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (!node.processor) continue;

        QString text = QString("%1. %2").arg(i + 1)
            .arg(ProcessorLocalization::localize(node.processor->metadata()).name);
        QListWidgetItem* item = new QListWidgetItem(text, m_listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(node.enabled ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, i);  // Store index
    }

    m_listWidget->blockSignals(false);
}

void PipelinePanel::onItemChanged(QListWidgetItem* item)
{
    if (!item) return;
    int index = item->data(Qt::UserRole).toInt();
    bool enabled = (item->checkState() == Qt::Checked);
    emit processorToggled(index, enabled);
}

void PipelinePanel::onItemSelectionChanged()
{
    QList<QListWidgetItem*> selected = m_listWidget->selectedItems();
    if (!selected.isEmpty()) {
        int index = selected.first()->data(Qt::UserRole).toInt();
        emit processorSelected(index);
    }
}

void PipelinePanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDockWidget::changeEvent(event);
}

void PipelinePanel::retranslateUi()
{
    setWindowTitle(tr("Processing Pipeline"));
    if (m_addBtn) {
        m_addBtn->setText(tr("+ Add Processor"));
        m_addBtn->setToolTip(tr("Add a new processor to the pipeline"));
    }
    if (m_clearBtn) {
        m_clearBtn->setText(tr("Clear All"));
        m_clearBtn->setToolTip(tr("Remove all processors from the pipeline"));
    }
}
