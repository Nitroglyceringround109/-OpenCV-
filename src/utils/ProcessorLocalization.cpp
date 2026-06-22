#include "ProcessorLocalization.h"
#include <QCoreApplication>

namespace ProcessorLocalization {

namespace {

QString translate(const QString& source)
{
    if (source.isEmpty())
        return source;
    return QCoreApplication::translate("Processors", source.toUtf8().constData());
}

} // namespace

QString localizedCategory(const QString& category)
{
    return translate(category);
}

QString localizedEnumValue(const QString& value)
{
    return translate(value);
}

PluginMetadata localize(const PluginMetadata& meta)
{
    PluginMetadata result = meta;
    result.name = translate(meta.name);
    result.description = translate(meta.description);
    result.category = localizedCategory(meta.category);
    return result;
}

QList<ParameterDescriptor> localizeParameters(const QList<ParameterDescriptor>& descriptors)
{
    QList<ParameterDescriptor> result;
    result.reserve(descriptors.size());

    for (const ParameterDescriptor& desc : descriptors) {
        ParameterDescriptor localized = desc;
        localized.displayName = translate(desc.displayName);
        localized.category = desc.category.isEmpty() ? desc.category : translate(desc.category);
        localized.toolTip = translate(desc.toolTip);

        localized.enumValues.clear();
        for (const QVariant& value : desc.enumValues)
            localized.enumValues << localizedEnumValue(value.toString());

        result << localized;
    }

    return result;
}

} // namespace ProcessorLocalization
