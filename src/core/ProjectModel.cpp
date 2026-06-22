#include "ProjectModel.h"
#include "ProcessingPipeline.h"
#include "PluginManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

ProjectModel::ProjectModel(QObject* parent)
    : QObject(parent)
{
}

bool ProjectModel::save(const QString& filePath, ProcessingPipeline* pipeline,
                        const QString& currentImagePath) const
{
    QJsonObject root;
    root["version"] = "1.0";
    root["imagePath"] = currentImagePath;
    root["pipeline"] = pipelineToJson(pipeline);

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    const_cast<ProjectModel*>(this)->m_filePath = filePath;
    const_cast<ProjectModel*>(this)->m_dirty = false;
    emit const_cast<ProjectModel*>(this)->projectSaved(filePath);
    return true;
}

QString ProjectModel::load(const QString& filePath, ProcessingPipeline* pipeline,
                           PluginManager* pluginManager)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return {};
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in project file:" << filePath;
        return {};
    }

    QJsonObject root = doc.object();
    QString imagePath = root["imagePath"].toString();

    pipeline->clearPipeline();
    jsonToPipeline(root["pipeline"].toObject(), pipeline, pluginManager);

    m_filePath = filePath;
    m_dirty = false;
    emit projectLoaded(filePath);
    return imagePath;
}

QJsonObject ProjectModel::pipelineToJson(ProcessingPipeline* pipeline) const
{
    QJsonArray nodesArray;
    for (int i = 0; i < pipeline->processorCount(); ++i) {
        const auto node = pipeline->nodeAt(i);
        QJsonObject nodeObj;
        nodeObj["processorId"] = node.processor->metadata().id;
        nodeObj["enabled"] = node.enabled;
        nodeObj["parameters"] = node.processor->parameterState();
        nodesArray.append(nodeObj);
    }

    QJsonObject pipelineObj;
    pipelineObj["nodes"] = nodesArray;
    return pipelineObj;
}

void ProjectModel::jsonToPipeline(const QJsonObject& obj, ProcessingPipeline* pipeline,
                                  PluginManager* pluginManager) const
{
    if (!pluginManager) {
        qWarning() << "ProjectModel::jsonToPipeline: PluginManager is required";
        return;
    }

    QJsonArray nodesArray = obj["nodes"].toArray();
    for (const QJsonValue& val : nodesArray) {
        QJsonObject nodeObj = val.toObject();
        QString processorId = nodeObj["processorId"].toString();
        bool enabled = nodeObj["enabled"].toBool(true);

        IImageProcessor* proc = pluginManager->createProcessor(processorId);
        if (!proc) {
            qWarning() << "Failed to restore processor:" << processorId;
            continue;
        }

        proc->setParameterState(nodeObj["parameters"].toObject());
        int index = pipeline->processorCount();
        pipeline->insertProcessor(index, proc);
        pipeline->setProcessorEnabled(index, enabled);
    }
}
