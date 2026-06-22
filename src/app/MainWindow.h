#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QElapsedTimer>
#include <QActionGroup>
#include <QTimer>
#include "core/ProcessingPipeline.h"
#include "core/PluginManager.h"
#include "core/ProjectModel.h"
#include "capture/CameraDevice.h"
#include "capture/FrameGrabber.h"
#include "capture/ImageFileSource.h"
#include "ui/ImageCanvas.h"
#include "ui/ComparisonView.h"
#include "ui/PipelinePanel.h"
#include "ui/ParameterPanel.h"
#include "ui/SourcePanel.h"
#include "ui/HistoryPanel.h"
#include "ui/PreferencesDialog.h"
#include "utils/AppPaths.h"
#include "utils/AppSettings.h"
#include "utils/LanguageManager.h"

/**
 * @brief Main window: composition root that wires everything together.
 *
 * Creates all dock widgets, canvas, pipeline, plugin manager;
 * wires all signal/slot connections; implements menu/toolbar actions;
 * handles project save/load orchestration.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onOpenImage();
    void onOpenReferenceImage();
    void onSaveProject();
    void onOpenProject();
    void onExportResult();
    void onStartCamera(int deviceIndex = 0);
    void onStopCamera();
    void onSnapshot();
    void onAddProcessor();
    void onProcessorSelected(int index);
    void onParameterChanged(const QString& paramId, const QVariant& value);
    void onPipelineFinished(const ImageData& output);
    void onProcessingTimeMs(int ms);
    void onToggleComparison();
    void onCompareModeChanged(int mode);
    void onPreferences();
    void onFolderSelected(const QString& folderPath);
    void onAbout();
    void onLanguageChanged(AppSettings::Language language);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void wireConnections();
    void loadPlugins();
    void populateProcessorMenu();
    void retranslateUi();
    void updateFpsCounter();
    void scheduleReprocess();
    void updateCanvasDisplay(const ImageData& image);
    void updateComparisonView(const ImageData& output);
    void updatePipelineWarnings(const ImageData& output);

    // Core
    ProcessingPipeline* m_pipeline = nullptr;
    PluginManager* m_pluginManager = nullptr;
    ProjectModel* m_projectModel = nullptr;

    // Capture
    CameraDevice* m_cameraDevice = nullptr;
    FrameGrabber* m_frameGrabber = nullptr;
    ImageFileSource* m_fileSource = nullptr;

    // UI
    ImageCanvas* m_canvas = nullptr;
    ComparisonView* m_comparisonView = nullptr;
    PipelinePanel* m_pipelinePanel = nullptr;
    ParameterPanel* m_parameterPanel = nullptr;
    SourcePanel* m_sourcePanel = nullptr;
    HistoryPanel* m_historyPanel = nullptr;

    // State
    ImageData m_originalImage;  ///< Before processing
    ImageData m_processedImage; ///< After processing
    bool m_comparisonMode = false;
    QLabel* m_statusImageInfo = nullptr;
    QLabel* m_statusPipelineInfo = nullptr;
    QLabel* m_statusTimeInfo = nullptr;
    QLabel* m_statusWarningInfo = nullptr;

    int m_fpsFrameCount = 0;
    QElapsedTimer m_fpsTimer;
    QElapsedTimer m_displayThrottle;
    QTimer* m_reprocessTimer = nullptr;
    bool m_cameraActive = false;
    bool m_pendingDisplayUpdate = false;
    ImageData m_pendingDisplayImage;

    QMenu* m_addProcessorMenu = nullptr;
    QMenu* m_compareModeMenu = nullptr;
    QMenu* m_languageMenu = nullptr;
    QActionGroup* m_compareModeGroup = nullptr;
    QActionGroup* m_languageGroup = nullptr;
    QString m_loadedPluginDir;

    QMenu* m_fileMenu = nullptr;
    QMenu* m_editMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_processMenu = nullptr;
    QMenu* m_toolsMenu = nullptr;
    QMenu* m_helpMenu = nullptr;

    QAction* m_actOpenImage = nullptr;
    QAction* m_actOpenReference = nullptr;
    QAction* m_actOpenProject = nullptr;
    QAction* m_actSaveProject = nullptr;
    QAction* m_actExportResult = nullptr;
    QAction* m_actExit = nullptr;
    QAction* m_actPreferences = nullptr;
    QAction* m_actToggleComparison = nullptr;
    QAction* m_actFitToWindow = nullptr;
    QAction* m_actZoomIn = nullptr;
    QAction* m_actZoomOut = nullptr;
    QAction* m_actResetZoom = nullptr;
    QAction* m_actClearPipeline = nullptr;
    QAction* m_actReprocess = nullptr;
    QAction* m_actStartCamera = nullptr;
    QAction* m_actStopCamera = nullptr;
    QAction* m_actSnapshot = nullptr;
    QAction* m_actAbout = nullptr;
    QAction* m_compareModeActions[3] = {};
    QAction* m_languageEnglishAction = nullptr;
    QAction* m_languageChineseAction = nullptr;

    QToolBar* m_mainToolbar = nullptr;
    QAction* m_tbOpen = nullptr;
    QAction* m_tbSave = nullptr;
    QAction* m_tbCamera = nullptr;
    QAction* m_tbSnapshot = nullptr;
    QAction* m_tbReprocess = nullptr;
    QAction* m_tbCompare = nullptr;
};

#endif // MAINWINDOW_H
