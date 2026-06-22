#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

/**
 * @brief Convenience wrappers for JSON read/write operations.
 */
namespace JsonHelper {

/// Read a JSON file and return the root object
QJsonObject readJsonFile(const QString& filePath);

/// Write a JSON object to file with pretty formatting
bool writeJsonFile(const QString& filePath, const QJsonObject& obj);

/// Write a JSON array to file with pretty formatting
bool writeJsonFile(const QString& filePath, const QJsonArray& arr);

/// Get a string value from a JSON object with a default
QString getString(const QJsonObject& obj, const QString& key,
                  const QString& defaultValue = {});

/// Get an int value from a JSON object with a default
int getInt(const QJsonObject& obj, const QString& key, int defaultValue = 0);

/// Get a double value from a JSON object with a default
double getDouble(const QJsonObject& obj, const QString& key, double defaultValue = 0.0);

/// Get a bool value from a JSON object with a default
bool getBool(const QJsonObject& obj, const QString& key, bool defaultValue = false);

} // namespace JsonHelper

#endif // JSONHELPER_H
