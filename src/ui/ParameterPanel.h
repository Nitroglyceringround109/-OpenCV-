#ifndef PARAMETERPANEL_H
#define PARAMETERPANEL_H

#include <QDockWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMap>
#include "core/IImageProcessor.h"

/**
 * @brief Dock widget that dynamically generates parameter widgets from
 *        IImageProcessor::parameterDescriptors().
 *
 * This is the key "含金量" component: the host generates UI automatically
 * from metadata, so adding a new processor requires zero UI code.
 *
 * Creates QSpinBox, QDoubleSpinBox, QSlider, QComboBox, or QCheckBox
 * based on ParameterDescriptor fields.
 */
class ParameterPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget* parent = nullptr);

public slots:
    /// Show parameters for the given processor
    void showParametersFor(IImageProcessor* processor);

    /// Clear all parameter widgets
    void clear();

protected:
    void changeEvent(QEvent* event) override;

signals:
    /// Emitted when any parameter value changes
    void parameterChanged(const QString& paramId, const QVariant& value);

private:
    void createWidgetForDescriptor(const ParameterDescriptor& desc,
                                   QVBoxLayout* layout);
    void onIntChanged(const QString& id, int value);
    void onDoubleChanged(const QString& id, double value);
    void onBoolChanged(const QString& id, bool checked);
    void onEnumChanged(const QString& id, int index);

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    IImageProcessor* m_currentProcessor = nullptr;
    QMap<QString, QWidget*> m_widgets;  ///< paramId -> widget
    bool m_suppressSignals = false;
};

#endif // PARAMETERPANEL_H
