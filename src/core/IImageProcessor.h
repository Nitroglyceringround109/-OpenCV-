#ifndef IIMAGEPROCESSOR_H
#define IIMAGEPROCESSOR_H

#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVersionNumber>
#include <opencv2/core.hpp>
#include "ImageData.h"

/**
 * @brief Describes one adjustable parameter exposed by a processor.
 *
 * The host application uses this metadata to dynamically generate
 * appropriate UI widgets (spinbox, slider, checkbox, combobox)
 * without any plugin-specific UI code.
 */
struct ParameterDescriptor {
    QString id;              ///< Machine-readable key, e.g. "threshold"
    QString displayName;     ///< Human-readable, e.g. "Threshold Value"
    QString category;        ///< Grouping hint, e.g. "Advanced"
    QVariant defaultValue;   ///< Default value
    QVariant minValue;       ///< Minimum (for int/double types)
    QVariant maxValue;       ///< Maximum (for int/double types)
    QVariant stepValue;      ///< Step size for spinbox/slider
    QVariantList enumValues; ///< For combo-box parameters: list of valid values
    QString toolTip;         ///< Hover help text

    ParameterDescriptor() = default;

    /// Convenience constructor for int parameters
    static ParameterDescriptor intParam(
        const QString& id, const QString& displayName,
        int defaultValue, int min, int max, int step = 1,
        const QString& category = {}, const QString& toolTip = {})
    {
        ParameterDescriptor d;
        d.id = id;
        d.displayName = displayName;
        d.category = category;
        d.defaultValue = defaultValue;
        d.minValue = min;
        d.maxValue = max;
        d.stepValue = step;
        d.toolTip = toolTip;
        return d;
    }

    /// Convenience constructor for double parameters
    static ParameterDescriptor doubleParam(
        const QString& id, const QString& displayName,
        double defaultValue, double min, double max, double step = 0.1,
        const QString& category = {}, const QString& toolTip = {})
    {
        ParameterDescriptor d;
        d.id = id;
        d.displayName = displayName;
        d.category = category;
        d.defaultValue = defaultValue;
        d.minValue = min;
        d.maxValue = max;
        d.stepValue = step;
        d.toolTip = toolTip;
        return d;
    }

    /// Convenience constructor for bool parameters
    static ParameterDescriptor boolParam(
        const QString& id, const QString& displayName,
        bool defaultValue,
        const QString& category = {}, const QString& toolTip = {})
    {
        ParameterDescriptor d;
        d.id = id;
        d.displayName = displayName;
        d.category = category;
        d.defaultValue = defaultValue;
        d.toolTip = toolTip;
        return d;
    }

    /// Convenience constructor for enum (combobox) parameters
    static ParameterDescriptor enumParam(
        const QString& id, const QString& displayName,
        const QStringList& values, int defaultIndex = 0,
        const QString& category = {}, const QString& toolTip = {})
    {
        ParameterDescriptor d;
        d.id = id;
        d.displayName = displayName;
        d.category = category;
        d.defaultValue = defaultIndex;
        for (const auto& v : values)
            d.enumValues << v;
        d.toolTip = toolTip;
        return d;
    }
};
Q_DECLARE_METATYPE(ParameterDescriptor)

/**
 * @brief Metadata about a plugin/processor, used for menus and registry.
 */
struct PluginMetadata {
    QString id;              ///< Unique identifier: "org.svw.edge_detection.canny"
    QString name;            ///< Display name: "Canny Edge Detection"
    QString description;     ///< One-line description
    QString category;        ///< Grouping: "Edge Detection", "Filtering", etc.
    QString author;
    QVersionNumber version;
    QString iconPath;        ///< Resource path to icon

    PluginMetadata() = default;
    PluginMetadata(const QString& id, const QString& name,
                   const QString& desc, const QString& category,
                   const QString& author = {},
                   const QVersionNumber& version = QVersionNumber(1, 0, 0))
        : id(id), name(name), description(desc),
          category(category), author(author), version(version) {}
};
Q_DECLARE_METATYPE(PluginMetadata)

/**
 * @brief The central plugin interface for all image processors.
 *
 * Every image processing algorithm implements this interface.
 * The host application discovers processors via PluginManager,
 * chains them in ProcessingPipeline, and generates UI from
 * parameterDescriptors().
 *
 * Implementations must be thread-safe if used in async pipeline mode.
 * The process() method must NOT modify the input ImageData; create a new one.
 */
class IImageProcessor
{
public:
    virtual ~IImageProcessor() = default;

    // --- Identity ---
    virtual PluginMetadata metadata() const = 0;

    // --- Parameters ---
    virtual QList<ParameterDescriptor> parameterDescriptors() const = 0;
    virtual void setParameter(const QString& id, const QVariant& value) = 0;
    virtual QVariant parameter(const QString& id) const = 0;
    virtual void resetParameters() = 0;

    // --- Processing ---
    /// Process the input ImageData and return a new ImageData with the result.
    /// Must be thread-safe. Must NOT modify the input.
    virtual ImageData process(const ImageData& input) = 0;

    // --- Serialization ---
    /// Serialize current parameter state to JSON for project save
    virtual QJsonObject parameterState() const = 0;
    /// Restore parameter state from JSON for project load
    virtual void setParameterState(const QJsonObject& state) = 0;

    // --- Capabilities ---
    /// Whether this processor requires a second image (e.g., feature matching)
    virtual bool requiresSecondImage() const { return false; }
    /// Whether this processor produces overlay/annotation output
    /// (draws on top rather than transforming pixels)
    virtual bool isAnnotationProcessor() const { return false; }
};

#define IImageProcessor_IID "org.svw.IImageProcessor"

Q_DECLARE_INTERFACE(IImageProcessor, IImageProcessor_IID)

#endif // IIMAGEPROCESSOR_H
