#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "ImageData.h"

class ProcessingPipeline;
class PluginManager;

/**
 * @brief Serialization model for save/load workspace state.
 *
 * Saves/loads the complete workspace state (loaded image paths,
 * pipeline configuration with processor IDs and parameter states,
 * window layout) to a .svw JSON file.
 */
class ProjectModel : public QObject
{
    Q_OBJECT

public:
    explicit ProjectModel(QObject* parent = nullptr);

    /// Save current project to file
    bool save(const QString& filePath, ProcessingPipeline* pipeline,
              const QString& currentImagePath) const;

    /// Load project from file, returns loaded image path
    QString load(const QString& filePath, ProcessingPipeline* pipeline,
                 PluginManager* pluginManager);

    /// Check if there are unsaved changes
    bool isDirty() const { return m_dirty; }
    void setDirty(bool dirty) { m_dirty = dirty; }

    /// Current project file path
    QString currentFilePath() const { return m_filePath; }

signals:
    void projectSaved(const QString& filePath);
    void projectLoaded(const QString& filePath);
    void projectModified();

private:
    QJsonObject pipelineToJson(ProcessingPipeline* pipeline) const;
    void jsonToPipeline(const QJsonObject& obj, ProcessingPipeline* pipeline,
                        PluginManager* pluginManager) const;

    QString m_filePath;
    bool m_dirty = false;
};

#endif // PROJECTMODEL_H
