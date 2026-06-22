#include "JsonHelper.h"
#include <QFile>
#include <QDebug>

namespace JsonHelper {

QJsonObject readJsonFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open JSON file:" << filePath;
        return QJsonObject();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in file:" << filePath;
        return QJsonObject();
    }

    return doc.object();
}

bool writeJsonFile(const QString& filePath, const QJsonObject& obj)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool writeJsonFile(const QString& filePath, const QJsonArray& arr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QJsonDocument doc(arr);
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

QString getString(const QJsonObject& obj, const QString& key, const QString& defaultValue)
{
    return obj.contains(key) ? obj[key].toString(defaultValue) : defaultValue;
}

int getInt(const QJsonObject& obj, const QString& key, int defaultValue)
{
    return obj.contains(key) ? obj[key].toInt(defaultValue) : defaultValue;
}

double getDouble(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.contains(key) ? obj[key].toDouble(defaultValue) : defaultValue;
}

bool getBool(const QJsonObject& obj, const QString& key, bool defaultValue)
{
    return obj.contains(key) ? obj[key].toBool(defaultValue) : defaultValue;
}

} // namespace JsonHelper
