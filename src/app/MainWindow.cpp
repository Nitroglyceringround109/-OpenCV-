#include "MainWindow.h"
#include "ui/PreferencesDialog.h"
#include "utils/AppPaths.h"
#include "utils/AppSettings.h"
#include "utils/ProcessorLocalization.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <opencv2/imgcodecs.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QDir>
#include <QActionGroup>
#include <QEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    wireConnections();
    loadPlugins();

    setWindowTitle(tr("Smart Vision Workbench"));
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // Core objects
    m_pipeline = new ProcessingPipeline(this);
    m_pluginManager = new PluginManager(this);
    m_projectModel = new ProjectModel(this);
    m_cameraDevice = new CameraDevice(this);
    m_frameGrabber = new FrameGrabber(this);
    m_fileSource = new ImageFileSource(this);

    // Central widget with splitter for canvas + comparison
    QSplitter* centralSplitter = new QSplitter(Qt::Vertical, this);
    m_canvas = new ImageCanvas(this);
    m_comparisonView = new ComparisonView(this);
    m_comparisonView->hide();
    m_comparisonView->setCompareMode(
        static_cast<ComparisonView::CompareMode>(AppSettings::defaultCompareMode()));

    centralSplitter->addWidget(m_canvas);
    centralSplitter->addWidget(m_comparisonView);
    centralSplitter->setStretchFactor(0, 3);
    centralSplitter->setStretchFactor(1, 1);
    setCentralWidget(centralSplitter);

    // Dock widgets
    m_sourcePanel = new SourcePanel(this);
    m_sourcePanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_sourcePanel);

    m_parameterPanel = new ParameterPanel(this);
    m_parameterPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_parameterPanel);

    m_pipelinePanel = new PipelinePanel(this);
    m_pipelinePanel->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_pipelinePanel);

    m_historyPanel = new HistoryPanel(this);
    m_historyPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tabifyDockWidget(m_parameterPanel, m_historyPanel);

    m_reprocessTimer = new QTimer(this);
    m_reprocessTimer->setSingleShot(true);
    m_reprocessTimer->setInterval(200);
    connect(m_reprocessTimer, &QTimer::timeout,
            m_pipeline, &ProcessingPipeline::reprocess);
}

void MainWindow::setupMenuBar()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_actOpenImage = m_fileMenu->addAction(tr("Open Image..."), this, &MainWindow::onOpenImage,
                                             QKeySequence(QKeySequence::Open));
    m_actOpenReference = m_fileMenu->addAction(tr("Open Reference Image..."), this,
                                                 &MainWindow::onOpenReferenceImage);
    m_actOpenProject = m_fileMenu->addAction(tr("Open Project..."), this, &MainWindow::onOpenProject);
    m_actSaveProject = m_fileMenu->addAction(tr("Save Project"), this, &MainWindow::onSaveProject,
                                               QKeySequence(QKeySequence::Save));
    m_fileMenu->addSeparator();
    m_actExportResult = m_fileMenu->addAction(tr("Export Result..."), this, &MainWindow::onExportResult);
    m_fileMenu->addSeparator();
    m_actExit = m_fileMenu->addAction(tr("Exit"), this, &QWidget::close, QKeySequence(QKeySequence::Quit));

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_actPreferences = m_editMenu->addAction(tr("Preferences..."), this, &MainWindow::onPreferences);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_sourcePanel->toggleViewAction());
    m_viewMenu->addAction(m_parameterPanel->toggleViewAction());
    m_viewMenu->addAction(m_pipelinePanel->toggleViewAction());
    m_viewMenu->addAction(m_historyPanel->toggleViewAction());
    m_viewMenu->addSeparator();
    m_actToggleComparison = m_viewMenu->addAction(tr("Toggle Comparison"), this,
                                                  &MainWindow::onToggleComparison,
                                                  QKeySequence(tr("Ctrl+D")));
    m_compareModeMenu = m_viewMenu->addMenu(tr("Comparison Mode"));
    m_compareModeGroup = new QActionGroup(this);
    auto addCompareMode = [this](const QString& label, ComparisonView::CompareMode mode, int index) {
        QAction* action = m_compareModeMenu->addAction(label);
        action->setCheckable(true);
        action->setData(static_cast<int>(mode));
        m_compareModeGroup->addAction(action);
        m_compareModeActions[index] = action;
        if (static_cast<int>(mode) == AppSettings::defaultCompareMode())
            action->setChecked(true);
        connect(action, &QAction::triggered, this, [this, mode]() {
            onCompareModeChanged(static_cast<int>(mode));
        });
    };
    addCompareMode(tr("Side by Side"), ComparisonView::SideBySide, 0);
    addCompareMode(tr("Overlay"), ComparisonView::Overlay, 1);
    addCompareMode(tr("Difference"), ComparisonView::Diff, 2);

    m_languageMenu = m_viewMenu->addMenu(tr("Language"));
    m_languageGroup = new QActionGroup(this);
    m_languageEnglishAction = m_languageMenu->addAction(tr("English"));
    m_languageEnglishAction->setCheckable(true);
    m_languageEnglishAction->setData(static_cast<int>(AppSettings::Language::English));
    m_languageGroup->addAction(m_languageEnglishAction);

    m_languageChineseAction = m_languageMenu->addAction(QStringLiteral("中文"));
    m_languageChineseAction->setCheckable(true);
    m_languageChineseAction->setData(static_cast<int>(AppSettings::Language::Chinese));
    m_languageGroup->addAction(m_languageChineseAction);

    const AppSettings::Language currentLanguage = AppSettings::language();
    if (currentLanguage == AppSettings::Language::Chinese)
        m_languageChineseAction->setChecked(true);
    else
        m_languageEnglishAction->setChecked(true);

    connect(m_languageEnglishAction, &QAction::triggered, this, [this]() {
        onLanguageChanged(AppSettings::Language::English);
    });
    connect(m_languageChineseAction, &QAction::triggered, this, [this]() {
        onLanguageChanged(AppSettings::Language::Chinese);
    });

    m_viewMenu->addSeparator();
    m_actFitToWindow = m_viewMenu->addAction(tr("Fit to Window"), m_canvas, &ImageCanvas::fitToWindow,
                                             QKeySequence(tr("Ctrl+0")));
    m_actZoomIn = m_viewMenu->addAction(tr("Zoom In"), m_canvas, &ImageCanvas::zoomIn,
                                        QKeySequence(QKeySequence::ZoomIn));
    m_actZoomOut = m_viewMenu->addAction(tr("Zoom Out"), m_canvas, &ImageCanvas::zoomOut,
                                          QKeySequence(QKeySequence::ZoomOut));
    m_actResetZoom = m_viewMenu->addAction(tr("Reset Zoom"), m_canvas, &ImageCanvas::resetZoom,
                                           QKeySequence(tr("Ctrl+1")));

    m_processMenu = menuBar()->addMenu(tr("&Process"));
    m_addProcessorMenu = m_processMenu->addMenu(tr("Add Processor"));
    m_processMenu->addSeparator();
    m_actClearPipeline = m_processMenu->addAction(tr("Clear Pipeline"), m_pipeline,
                                                  &ProcessingPipeline::clearPipeline);
    m_actReprocess = m_processMenu->addAction(tr("Reprocess"), m_pipeline,
                                              &ProcessingPipeline::reprocess, QKeySequence(tr("F5")));

    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_actStartCamera = m_toolsMenu->addAction(tr("Start Camera"), this, &MainWindow::onStartCamera);
    m_actStopCamera = m_toolsMenu->addAction(tr("Stop Camera"), this, &MainWindow::onStopCamera);
    m_toolsMenu->addSeparator();
    m_actSnapshot = m_toolsMenu->addAction(tr("Snapshot"), this, &MainWindow::onSnapshot);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_actAbout = m_helpMenu->addAction(tr("About"), this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    m_mainToolbar = addToolBar(tr("Main Toolbar"));
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setIconSize(QSize(20, 20));

    m_tbOpen = m_mainToolbar->addAction(tr("📂 Open"), this, &MainWindow::onOpenImage);
    m_tbSave = m_mainToolbar->addAction(tr("💾 Save"), this, &MainWindow::onSaveProject);
    m_mainToolbar->addSeparator();
    m_tbCamera = m_mainToolbar->addAction(tr("📷 Camera"), this, &MainWindow::onStartCamera);
    m_tbSnapshot = m_mainToolbar->addAction(tr("📸 Snapshot"), this, &MainWindow::onSnapshot);
    m_mainToolbar->addSeparator();
    m_mainToolbar->addAction(tr("🔍+"), m_canvas, &ImageCanvas::zoomIn);
    m_mainToolbar->addAction(tr("🔍-"), m_canvas, &ImageCanvas::zoomOut);
    m_mainToolbar->addAction(tr("⊞ Fit"), m_canvas, &ImageCanvas::fitToWindow);
    m_mainToolbar->addSeparator();
    m_tbReprocess = m_mainToolbar->addAction(tr("↻ Reprocess"), m_pipeline, &ProcessingPipeline::reprocess);
    m_tbCompare = m_mainToolbar->addAction(tr("⇄ Compare"), this, &MainWindow::onToggleComparison);
}

void MainWindow::setupStatusBar()
{
    m_statusImageInfo = new QLabel(tr("No image"), this);
    m_statusPipelineInfo = new QLabel(tr("Pipeline: 0 steps"), this);
    m_statusTimeInfo = new QLabel(tr("-- ms"), this);
    m_statusWarningInfo = new QLabel(this);
    m_statusWarningInfo->setStyleSheet(QStringLiteral("color: #e65100;"));
    m_statusWarningInfo->setWordWrap(false);

    statusBar()->addWidget(m_statusImageInfo, 1);
    statusBar()->addWidget(m_statusPipelineInfo, 1);
    statusBar()->addWidget(m_statusTimeInfo, 0);
    statusBar()->addWidget(m_statusWarningInfo, 2);
}

void MainWindow::wireConnections()
{
    // Source -> Pipeline
    connect(m_fileSource, &ImageFileSource::imageLoaded,
            this, [this](const ImageData& data) {
        m_originalImage = data;
        m_canvas->displayImage(data);
        m_pipeline->processFrame(data);
        m_statusImageInfo->setText(
            tr("%1x%2 | %3").arg(data.width()).arg(data.height()).arg(data.channels() == 1 ? "Gray" : "Color"));

        const QString filePath = data.metadata("filePath").toString();
        const QString fileName = data.metadata("fileName").toString();
        if (!filePath.isEmpty()) {
            m_sourcePanel->setSourceInfo(tr("Image: %1").arg(fileName));
        } else if (!fileName.isEmpty()) {
            m_sourcePanel->setSourceInfo(tr("Image: %1").arg(fileName));
        } else {
            m_sourcePanel->setSourceInfo(tr("Image loaded"));
        }
    });

    connect(m_cameraDevice, &CameraDevice::frameCaptured,
            m_frameGrabber, &FrameGrabber::onFrameCaptured,
            Qt::QueuedConnection);

    connect(m_frameGrabber, &FrameGrabber::sourceFrameReady,
            this, [this](const ImageData& data) {
        m_originalImage = data;
        // Only show raw frames when the pipeline is empty; otherwise the canvas
        // alternates raw/processed and flickers (especially with slow plugins).
        if (m_pipeline->processorCount() == 0)
            updateCanvasDisplay(data);
        updateFpsCounter();
        m_pipeline->processFrame(data);
    });

    connect(m_cameraDevice, &CameraDevice::cameraStarted,
            this, [this]() { m_cameraActive = true; });
    connect(m_cameraDevice, &CameraDevice::cameraStopped,
            this, [this]() {
        m_cameraActive = false;
        m_pendingDisplayUpdate = false;
    });

    // Pipeline -> Views
    connect(m_pipeline, &ProcessingPipeline::pipelineFinished,
            this, &MainWindow::onPipelineFinished);

    connect(m_pipeline, &ProcessingPipeline::processingTimeMs,
            this, &MainWindow::onProcessingTimeMs);

    connect(m_pipeline, &ProcessingPipeline::pipelineStructureChanged,
            this, [this]() {
        m_pipelinePanel->rebuildList(m_pipeline->allNodes());
        m_statusPipelineInfo->setText(
            tr("Pipeline: %1 steps").arg(m_pipeline->processorCount()));
    });

    // PipelinePanel -> Pipeline
    connect(m_pipelinePanel, &PipelinePanel::processorToggled,
            this, [this](int index, bool enabled) {
        m_pipeline->setProcessorEnabled(index, enabled);
        scheduleReprocess();
    });
    connect(m_pipelinePanel, &PipelinePanel::processorSelected,
            this, &MainWindow::onProcessorSelected);
    connect(m_pipelinePanel, &PipelinePanel::addProcessorRequested,
            this, &MainWindow::onAddProcessor);
    connect(m_pipelinePanel, &PipelinePanel::processorRemoved,
            this, [this](int index) {
        m_pipeline->removeProcessor(index);
    });
    connect(m_pipelinePanel, &PipelinePanel::processorMoved,
            m_pipeline, &ProcessingPipeline::moveProcessor);

    // ParameterPanel -> Pipeline
    connect(m_parameterPanel, &ParameterPanel::parameterChanged,
            this, &MainWindow::onParameterChanged);

    // SourcePanel
    connect(m_sourcePanel, &SourcePanel::fileSelected,
            m_fileSource, &ImageFileSource::loadFile);
    connect(m_sourcePanel, &SourcePanel::folderSelected,
            this, &MainWindow::onFolderSelected);
    connect(m_sourcePanel, &SourcePanel::cameraSelected,
            this, &MainWindow::onStartCamera);
    connect(m_sourcePanel, &SourcePanel::cameraStopRequested,
            this, &MainWindow::onStopCamera);
    connect(m_sourcePanel, &SourcePanel::snapshotRequested,
            this, &MainWindow::onSnapshot);

    connect(m_historyPanel, &HistoryPanel::stateRestored,
            this, [this](const ImageData& state) {
        m_processedImage = state;
        m_canvas->displayImage(state);
    });

    // Camera list
    m_sourcePanel->updateCameraList(m_cameraDevice->availableCameraIndices());
}

void MainWindow::loadPlugins()
{
    QString pluginDir = AppPaths::resolvePluginDirectory();
    if (pluginDir == m_loadedPluginDir && m_pluginManager->hasPlugins())
        return;

    m_pipeline->clearPipeline();
    m_pluginManager->unloadAll();
    m_loadedPluginDir = pluginDir;

    qDebug() << "Scanning plugins in:" << pluginDir;
    m_pluginManager->scanPlugins(pluginDir);
    populateProcessorMenu();
}

void MainWindow::populateProcessorMenu()
{
    m_addProcessorMenu->clear();
    QStringList categories = m_pluginManager->categories();

    for (const QString& cat : categories) {
        QMenu* catMenu = m_addProcessorMenu->addMenu(ProcessorLocalization::localizedCategory(cat));
        QList<PluginMetadata> procs = m_pluginManager->processorsByCategory(cat);
        for (const PluginMetadata& meta : procs) {
            const PluginMetadata localized = ProcessorLocalization::localize(meta);
            catMenu->addAction(localized.name, this, [this, meta]() {
                IImageProcessor* proc = m_pluginManager->createProcessor(meta.id);
                if (proc) {
                    m_pipeline->insertProcessor(m_pipeline->processorCount(), proc);
                    m_pipeline->reprocess();
                }
            });
        }
    }

    // If no plugins loaded, show a message
    if (m_addProcessorMenu->isEmpty()) {
        m_addProcessorMenu->addAction(tr("No plugins found"))->setEnabled(false);
    }
}

void MainWindow::onOpenImage()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open Image"), {},
        tr("Images (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;All Files (*)"));
    if (!path.isEmpty())
        m_fileSource->loadFile(path);
}

void MainWindow::onOpenReferenceImage()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open Reference Image"), {},
        tr("Images (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;All Files (*)"));
    if (path.isEmpty())
        return;

    cv::Mat mat = cv::imread(path.toStdString());
    if (mat.empty()) {
        QMessageBox::warning(this, tr("Open Reference Image"),
                             tr("Failed to load image: %1").arg(path));
        return;
    }

    ImageData ref(mat, "reference");
    ref.setMetadata("filePath", path);
    m_pipeline->setSecondImage(ref);
    m_pipeline->reprocess();
    m_statusImageInfo->setText(
        tr("Reference: %1x%2").arg(ref.width()).arg(ref.height()));
}

void MainWindow::onSaveProject()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save Project"), {},
        tr("SVW Project (*.svw);;All Files (*)"));
    if (!path.isEmpty())
        m_projectModel->save(path, m_pipeline, m_originalImage.metadata("filePath").toString());
}

void MainWindow::onOpenProject()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open Project"), {},
        tr("SVW Project (*.svw);;All Files (*)"));
    if (!path.isEmpty()) {
        QString imagePath = m_projectModel->load(path, m_pipeline, m_pluginManager);
        if (!imagePath.isEmpty())
            m_fileSource->loadFile(imagePath);
        m_pipeline->reprocess();
    }
}

void MainWindow::onExportResult()
{
    if (m_processedImage.isEmpty()) {
        QMessageBox::information(this, tr("Export"), tr("No processed image to export."));
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, tr("Export Result"), {},
        tr("PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp);;All Files (*)"));
    if (!path.isEmpty()) {
        cv::imwrite(path.toStdString(), m_processedImage.mat());
    }
}

void MainWindow::onStartCamera(int deviceIndex)
{
    m_cameraDevice->start(deviceIndex);
    m_fpsFrameCount = 0;
    m_fpsTimer.restart();
    m_sourcePanel->setSourceInfo(tr("Camera %1").arg(deviceIndex));
}

void MainWindow::onStopCamera()
{
    m_cameraDevice->stop();
    m_sourcePanel->setSourceInfo(tr("No source selected"));
}

void MainWindow::onSnapshot()
{
    if (m_processedImage.isEmpty()) return;

    QString path = QFileDialog::getSaveFileName(
        this, tr("Save Snapshot"), {},
        tr("PNG (*.png);;JPEG (*.jpg);;All Files (*)"));
    if (!path.isEmpty())
        cv::imwrite(path.toStdString(), m_processedImage.mat());
}

void MainWindow::onAddProcessor()
{
    // Show the add processor menu at cursor position
    m_addProcessorMenu->exec(QCursor::pos());
}

void MainWindow::onProcessorSelected(int index)
{
    if (index < 0 || index >= m_pipeline->processorCount()) return;
    const PipelineNode node = m_pipeline->nodeAt(index);
    if (node.processor)
        m_parameterPanel->showParametersFor(node.processor);
}

void MainWindow::onParameterChanged(const QString& paramId, const QVariant& value)
{
    Q_UNUSED(paramId)
    Q_UNUSED(value)
    scheduleReprocess();
}

void MainWindow::scheduleReprocess()
{
    m_reprocessTimer->start();
}

void MainWindow::updateCanvasDisplay(const ImageData& image)
{
    if (image.isEmpty())
        return;

    if (m_cameraActive) {
        if (!m_displayThrottle.isValid())
            m_displayThrottle.start();
        if (m_displayThrottle.elapsed() < 33) {
            m_pendingDisplayUpdate = true;
            m_pendingDisplayImage = image;
            return;
        }
        m_displayThrottle.restart();
    }

    m_pendingDisplayUpdate = false;
    m_canvas->displayImage(image);
}

void MainWindow::updateComparisonView(const ImageData& output)
{
    if (!m_comparisonMode)
        return;

    m_comparisonView->setBeforeImage(m_originalImage);
    m_comparisonView->setAfterImage(output);
}

void MainWindow::onPipelineFinished(const ImageData& output)
{
    m_processedImage = output;
    updateCanvasDisplay(output);
    updateComparisonView(output);
    updatePipelineWarnings(output);

    const QString decodedText = output.metadata("decodedText").toString();
    if (!decodedText.isEmpty()) {
        const QString preview = decodedText.length() > 48
            ? decodedText.left(48) + QStringLiteral("…")
            : decodedText;
        m_statusImageInfo->setToolTip(decodedText);
        if (!m_originalImage.isEmpty()) {
            m_statusImageInfo->setText(
                tr("%1x%2 | %3 | QR: %4")
                    .arg(output.width())
                    .arg(output.height())
                    .arg(output.channels() == 1 ? tr("Gray") : tr("Color"))
                    .arg(preview));
        }
    } else {
        m_statusImageInfo->setToolTip(QString());
        if (!m_originalImage.isEmpty() && !m_cameraActive) {
            m_statusImageInfo->setText(
                tr("%1x%2 | %3")
                    .arg(output.width())
                    .arg(output.height())
                    .arg(output.channels() == 1 ? tr("Gray") : tr("Color")));
        }
    }

    if (!m_cameraActive)
        m_historyPanel->pushState(output);

    if (m_pendingDisplayUpdate && m_cameraActive) {
        m_pendingDisplayUpdate = false;
        QTimer::singleShot(0, this, [this]() {
            if (!m_pendingDisplayImage.isEmpty())
                updateCanvasDisplay(m_pendingDisplayImage);
        });
    }
}

void MainWindow::updatePipelineWarnings(const ImageData& output)
{
    const QStringList warnings = output.metadata("pipeline_warnings").toStringList();
    if (warnings.isEmpty()) {
        m_statusWarningInfo->clear();
        m_statusWarningInfo->setToolTip(QString());
        return;
    }

    const QString summary = warnings.size() == 1
        ? warnings.first()
        : tr("%1 warnings — %2").arg(warnings.size()).arg(warnings.first());
    m_statusWarningInfo->setText(summary);
    m_statusWarningInfo->setToolTip(warnings.join(QStringLiteral("\n")));
}

void MainWindow::onProcessingTimeMs(int ms)
{
    m_statusTimeInfo->setText(tr("%1 ms").arg(ms));
}

void MainWindow::onToggleComparison()
{
    m_comparisonMode = !m_comparisonMode;
    if (m_comparisonMode) {
        m_comparisonView->show();
        m_comparisonView->setBeforeImage(m_originalImage);
        m_comparisonView->setAfterImage(m_processedImage);
    } else {
        m_comparisonView->hide();
    }
}

void MainWindow::onCompareModeChanged(int mode)
{
    m_comparisonView->setCompareMode(static_cast<ComparisonView::CompareMode>(mode));
    AppSettings::setDefaultCompareMode(mode);

    if (m_comparisonMode) {
        m_comparisonView->setBeforeImage(m_originalImage);
        m_comparisonView->setAfterImage(m_processedImage);
    }

    for (QAction* action : m_compareModeGroup->actions()) {
        if (action->data().toInt() == mode)
            action->setChecked(true);
    }
}

void MainWindow::onPreferences()
{
    const AppSettings::Language previousLanguage = AppSettings::language();

    PreferencesDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (AppSettings::language() != previousLanguage) {
        LanguageManager::install(qApp, AppSettings::language());
        if (AppSettings::language() == AppSettings::Language::Chinese)
            m_languageChineseAction->setChecked(true);
        else
            m_languageEnglishAction->setChecked(true);
    }

    const QString newPluginDir = AppPaths::resolvePluginDirectory();
    if (newPluginDir != m_loadedPluginDir) {
        loadPlugins();
        m_pipelinePanel->rebuildList(m_pipeline->allNodes());
    }

    m_comparisonView->setCompareMode(
        static_cast<ComparisonView::CompareMode>(AppSettings::defaultCompareMode()));
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About Smart Vision Workbench"),
        tr("<h2>Smart Vision Workbench</h2>"
           "<p>Version 1.0.0</p>"
           "<p>A professional image processing application built with Qt 6 and OpenCV.</p>"
           "<p>Features plugin architecture, real-time camera processing, "
           "and an extensible processing pipeline.</p>"));
}

void MainWindow::onFolderSelected(const QString& folderPath)
{
    QDir dir(folderPath);
    const QStringList filters = {
        "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tiff", "*.tif", "*.webp"
    };
    const QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        QMessageBox::information(this, tr("Open Folder"),
                                 tr("No images found in: %1").arg(folderPath));
        return;
    }
    m_fileSource->loadFile(dir.filePath(files.first()));
}

void MainWindow::updateFpsCounter()
{
    ++m_fpsFrameCount;
    if (!m_fpsTimer.isValid())
        m_fpsTimer.start();
    if (m_fpsTimer.elapsed() >= 1000) {
        m_sourcePanel->updateFps(m_fpsFrameCount);
        m_fpsFrameCount = 0;
        m_fpsTimer.restart();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_cameraDevice->stop();
    QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("Smart Vision Workbench"));

    if (m_fileMenu) {
        m_fileMenu->setTitle(tr("&File"));
        m_actOpenImage->setText(tr("Open Image..."));
        m_actOpenReference->setText(tr("Open Reference Image..."));
        m_actOpenProject->setText(tr("Open Project..."));
        m_actSaveProject->setText(tr("Save Project"));
        m_actExportResult->setText(tr("Export Result..."));
        m_actExit->setText(tr("Exit"));
    }

    if (m_editMenu)
        m_editMenu->setTitle(tr("&Edit"));
    if (m_actPreferences)
        m_actPreferences->setText(tr("Preferences..."));

    if (m_viewMenu) {
        m_viewMenu->setTitle(tr("&View"));
        m_actToggleComparison->setText(tr("Toggle Comparison"));
        m_compareModeMenu->setTitle(tr("Comparison Mode"));
        m_languageMenu->setTitle(tr("Language"));
        m_actFitToWindow->setText(tr("Fit to Window"));
        m_actZoomIn->setText(tr("Zoom In"));
        m_actZoomOut->setText(tr("Zoom Out"));
        m_actResetZoom->setText(tr("Reset Zoom"));
    }

    if (m_compareModeActions[0])
        m_compareModeActions[0]->setText(tr("Side by Side"));
    if (m_compareModeActions[1])
        m_compareModeActions[1]->setText(tr("Overlay"));
    if (m_compareModeActions[2])
        m_compareModeActions[2]->setText(tr("Difference"));
    if (m_languageEnglishAction)
        m_languageEnglishAction->setText(tr("English"));

    if (m_processMenu) {
        m_processMenu->setTitle(tr("&Process"));
        m_addProcessorMenu->setTitle(tr("Add Processor"));
        m_actClearPipeline->setText(tr("Clear Pipeline"));
        m_actReprocess->setText(tr("Reprocess"));
    }

    if (m_toolsMenu) {
        m_toolsMenu->setTitle(tr("&Tools"));
        m_actStartCamera->setText(tr("Start Camera"));
        m_actStopCamera->setText(tr("Stop Camera"));
        m_actSnapshot->setText(tr("Snapshot"));
    }

    if (m_helpMenu) {
        m_helpMenu->setTitle(tr("&Help"));
        m_actAbout->setText(tr("About"));
    }

    if (m_mainToolbar) {
        m_mainToolbar->setWindowTitle(tr("Main Toolbar"));
        m_tbOpen->setText(tr("📂 Open"));
        m_tbSave->setText(tr("💾 Save"));
        m_tbCamera->setText(tr("📷 Camera"));
        m_tbSnapshot->setText(tr("📸 Snapshot"));
        m_tbReprocess->setText(tr("↻ Reprocess"));
        m_tbCompare->setText(tr("⇄ Compare"));
    }

    if (m_statusImageInfo && m_originalImage.isEmpty())
        m_statusImageInfo->setText(tr("No image"));
    if (m_statusPipelineInfo)
        m_statusPipelineInfo->setText(tr("Pipeline: %1 steps").arg(m_pipeline->processorCount()));

    populateProcessorMenu();
    m_pipelinePanel->rebuildList(m_pipeline->allNodes());
}

void MainWindow::onLanguageChanged(AppSettings::Language language)
{
    if (AppSettings::language() == language)
        return;

    AppSettings::setLanguage(language);
    LanguageManager::install(qApp, language);

    if (language == AppSettings::Language::Chinese)
        m_languageChineseAction->setChecked(true);
    else
        m_languageEnglishAction->setChecked(true);
}
