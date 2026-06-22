#include "ParameterPanel.h"
#include "utils/ProcessorLocalization.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QAbstractSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QEvent>

ParameterPanel::ParameterPanel(QWidget* parent)
    : QDockWidget(tr("Parameters"), parent)
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_contentWidget = new QWidget(m_scrollArea);
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setAlignment(Qt::AlignTop);
    m_contentLayout->setContentsMargins(8, 8, 8, 8);
    m_contentLayout->setSpacing(6);

    m_scrollArea->setWidget(m_contentWidget);
    setWidget(m_scrollArea);
}

void ParameterPanel::showParametersFor(IImageProcessor* processor)
{
    clear();
    m_currentProcessor = processor;

    if (!processor) return;

    m_suppressSignals = true;

    const PluginMetadata meta = ProcessorLocalization::localize(processor->metadata());
    const QList<ParameterDescriptor> descs =
        ProcessorLocalization::localizeParameters(processor->parameterDescriptors());

    // Title
    QLabel* titleLabel = new QLabel(meta.name, m_contentWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    m_contentLayout->addWidget(titleLabel);

    // Description
    if (!meta.description.isEmpty()) {
        QLabel* descLabel = new QLabel(meta.description, m_contentWidget);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("color: #888;");
        m_contentLayout->addWidget(descLabel);
    }

    // Separator
    QFrame* line = new QFrame(m_contentWidget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_contentLayout->addWidget(line);

    // Group descriptors by category
    QMap<QString, QList<ParameterDescriptor>> categories;
    for (const auto& desc : descs) {
        QString cat = desc.category.isEmpty() ? tr("General") : desc.category;
        categories[cat] << desc;
    }

    // Create widgets for each category
    for (auto it = categories.constBegin(); it != categories.constEnd(); ++it) {
        QGroupBox* group = new QGroupBox(it.key(), m_contentWidget);
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        groupLayout->setSpacing(4);

        for (const auto& desc : it.value()) {
            createWidgetForDescriptor(desc, groupLayout);
        }

        m_contentLayout->addWidget(group);
    }

    // Reset button
    QPushButton* resetBtn = new QPushButton(tr("Reset to Defaults"), m_contentWidget);
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentProcessor) {
            m_currentProcessor->resetParameters();
            showParametersFor(m_currentProcessor);  // Refresh UI
        }
    });
    m_contentLayout->addWidget(resetBtn);

    m_contentLayout->addStretch();
    m_suppressSignals = false;
}

void ParameterPanel::clear()
{
    m_currentProcessor = nullptr;
    m_widgets.clear();

    // Remove all widgets from layout
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
}

void ParameterPanel::createWidgetForDescriptor(const ParameterDescriptor& desc,
                                                QVBoxLayout* layout)
{
    // Has enum values -> ComboBox
    if (!desc.enumValues.isEmpty()) {
        QHBoxLayout* hLayout = new QHBoxLayout();
        QLabel* label = new QLabel(desc.displayName, m_contentWidget);
        label->setToolTip(desc.toolTip);
        hLayout->addWidget(label);

        QComboBox* combo = new QComboBox(m_contentWidget);
        for (const auto& val : desc.enumValues)
            combo->addItem(val.toString());
        QVariant currentValue = m_currentProcessor->parameter(desc.id);
        if (!currentValue.isValid())
            currentValue = desc.defaultValue;
        combo->setToolTip(desc.toolTip);

        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this, desc](int index) {
            onEnumChanged(desc.id, index);
        });

        combo->blockSignals(true);
        combo->setCurrentIndex(currentValue.toInt());
        combo->blockSignals(false);

        hLayout->addWidget(combo);
        m_widgets[desc.id] = combo;
        layout->addLayout(hLayout);
        return;
    }

    // Bool type -> CheckBox
    if (desc.defaultValue.typeId() == QMetaType::Bool) {
        QCheckBox* check = new QCheckBox(desc.displayName, m_contentWidget);
        QVariant currentValue = m_currentProcessor->parameter(desc.id);
        if (!currentValue.isValid())
            currentValue = desc.defaultValue;
        check->setToolTip(desc.toolTip);

        connect(check, &QCheckBox::toggled,
                this, [this, desc](bool checked) {
            onBoolChanged(desc.id, checked);
        });

        check->blockSignals(true);
        check->setChecked(currentValue.toBool());
        check->blockSignals(false);

        m_widgets[desc.id] = check;
        layout->addWidget(check);
        return;
    }

    // Int type -> SpinBox + Slider
    if (desc.defaultValue.typeId() == QMetaType::Int) {
        QHBoxLayout* hLayout = new QHBoxLayout();
        QLabel* label = new QLabel(desc.displayName, m_contentWidget);
        label->setToolTip(desc.toolTip);
        hLayout->addWidget(label);

        QSpinBox* spin = new QSpinBox(m_contentWidget);
        spin->setRange(desc.minValue.toInt(), desc.maxValue.toInt());
        spin->setSingleStep(desc.stepValue.toInt());
        spin->setMinimumWidth(88);
        spin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        spin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        spin->setKeyboardTracking(false);
        QVariant currentValue = m_currentProcessor->parameter(desc.id);
        if (!currentValue.isValid())
            currentValue = desc.defaultValue;
        spin->setToolTip(desc.toolTip);

        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this, desc](int value) {
            onIntChanged(desc.id, value);
        });

        spin->blockSignals(true);
        spin->setValue(currentValue.toInt());
        spin->blockSignals(false);

        hLayout->addWidget(spin, 1);
        m_widgets[desc.id] = spin;
        layout->addLayout(hLayout);
        return;
    }

    // Double type -> DoubleSpinBox
    if (desc.defaultValue.typeId() == QMetaType::Double) {
        QHBoxLayout* hLayout = new QHBoxLayout();
        QLabel* label = new QLabel(desc.displayName, m_contentWidget);
        label->setToolTip(desc.toolTip);
        hLayout->addWidget(label);

        QDoubleSpinBox* spin = new QDoubleSpinBox(m_contentWidget);
        const double step = desc.stepValue.toDouble();
        spin->setRange(desc.minValue.toDouble(), desc.maxValue.toDouble());
        spin->setSingleStep(step);
        spin->setDecimals(step >= 0.1 ? 2 : (step >= 0.01 ? 3 : 4));
        spin->setMinimumWidth(88);
        spin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        spin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        spin->setKeyboardTracking(false);
        QVariant currentValue = m_currentProcessor->parameter(desc.id);
        if (!currentValue.isValid())
            currentValue = desc.defaultValue;
        spin->setToolTip(desc.toolTip);

        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this, desc](double value) {
            onDoubleChanged(desc.id, value);
        });

        spin->blockSignals(true);
        spin->setValue(currentValue.toDouble());
        spin->blockSignals(false);

        hLayout->addWidget(spin, 1);
        m_widgets[desc.id] = spin;
        layout->addLayout(hLayout);
        return;
    }
}

void ParameterPanel::onIntChanged(const QString& id, int value)
{
    if (m_suppressSignals) return;
    if (m_currentProcessor)
        m_currentProcessor->setParameter(id, value);
    emit parameterChanged(id, value);
}

void ParameterPanel::onDoubleChanged(const QString& id, double value)
{
    if (m_suppressSignals) return;
    if (m_currentProcessor)
        m_currentProcessor->setParameter(id, value);
    emit parameterChanged(id, value);
}

void ParameterPanel::onBoolChanged(const QString& id, bool checked)
{
    if (m_suppressSignals) return;
    if (m_currentProcessor)
        m_currentProcessor->setParameter(id, checked);
    emit parameterChanged(id, checked);
}

void ParameterPanel::onEnumChanged(const QString& id, int index)
{
    if (m_suppressSignals) return;
    if (m_currentProcessor)
        m_currentProcessor->setParameter(id, index);
    emit parameterChanged(id, index);
}

void ParameterPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("Parameters"));
        if (m_currentProcessor)
            showParametersFor(m_currentProcessor);
    }
    QDockWidget::changeEvent(event);
}
