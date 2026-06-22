#ifndef SOURCEPANEL_H
#define SOURCEPANEL_H

#include <QDockWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

/**
 * @brief Dock widget for source selection (camera, file, snapshot).
 */
class SourcePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit SourcePanel(QWidget* parent = nullptr);

public slots:
    void updateCameraList(const QList<int>& deviceIndices);
    void updateFps(int fps);
    void setSourceInfo(const QString& text);

protected:
    void changeEvent(QEvent* event) override;

signals:
    void cameraSelected(int deviceId);
    void fileSelected(const QString& path);
    void folderSelected(const QString& path);
    void snapshotRequested();
    void cameraStartRequested();
    void cameraStopRequested();

private:
    void setupUi();
    void retranslateUi();

    QComboBox* m_cameraCombo = nullptr;
    QPushButton* m_openFileBtn = nullptr;
    QPushButton* m_openFolderBtn = nullptr;
    QPushButton* m_cameraStartBtn = nullptr;
    QPushButton* m_cameraStopBtn = nullptr;
    QPushButton* m_snapshotBtn = nullptr;
    QLabel* m_fpsLabel = nullptr;
    QLabel* m_infoLabel = nullptr;
    QLabel* m_camLabel = nullptr;
    int m_currentFps = -1;
};

#endif // SOURCEPANEL_H
