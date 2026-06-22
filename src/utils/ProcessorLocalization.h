#ifndef PROCESSORLOCALIZATION_H
#define PROCESSORLOCALIZATION_H

#include "core/IImageProcessor.h"

namespace ProcessorLocalization {

PluginMetadata localize(const PluginMetadata& meta);
QList<ParameterDescriptor> localizeParameters(const QList<ParameterDescriptor>& descriptors);
QString localizedCategory(const QString& category);
QString localizedEnumValue(const QString& value);

} // namespace ProcessorLocalization

#endif // PROCESSORLOCALIZATION_H
