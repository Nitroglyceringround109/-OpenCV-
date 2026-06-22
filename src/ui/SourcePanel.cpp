#include "SourcePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QEvent>

SourcePanel::SourcePanel(QWidget* parent)
    : QDockWidget(tr("Source"), parent)
{
    setupUi();
}

void SourcePanel::setupUi()
{
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Camera section
    m_camLabel = new QLabel(tr("Camera:"), container);
    layout->addWidget(m_camLabel);

    m_cameraCombo = new QComboBox(container);
    layout->addWidget(m_cameraCombo);

    QHBoxLayout* camBtnLayout = new QHBoxLayout();
    m_cameraStartBtn = new QPushButton(tr("Start"), container);
    m_cameraStopBtn = new QPushButton(tr("Stop"), container);
    m_cameraStopBtn->setEnabled(false);
    camBtnLayout->addWidget(m_cameraStartBtn);
    camBtnLayout->addWidget(m_cameraStopBtn);
    layout->addLayout(camBtnLayout);

    connect(m_cameraStartBtn, &QPushButton::clicked, this, [this]() {
        m_cameraStartBtn->setEnabled(false);
        m_cameraStopBtn->setEnabled(true);
        emit cameraSelected(m_cameraCombo->currentData().toInt());
    });
    connect(m_cameraStopBtn, &QPushButton::clicked, this, [this]() {
        m_cameraStartBtn->setEnabled(true);
        m_cameraStopBtn->setEnabled(false);
        emit cameraStopRequested();
    });

    // Separator
    QFrame* line1 = new QFrame(container);
    line1->setFrameShape(QFrame::HLine);
    layout->addWidget(line1);

    // File section
    m_openFileBtn = new QPushButton(tr("Open Image..."), container);
    layout->addWidget(m_openFileBtn);
    connect(m_openFileBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Open Image"), {},
            tr("Images (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;All Files (*)"));
        if (!path.isEmpty())
            emit fileSelected(path);
    });

    m_openFolderBtn = new QPushButton(tr("Open Folder..."), container);
    layout->addWidget(m_openFolderBtn);
    connect(m_openFolderBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(
            this, tr("Open Image Folder"));
        if (!path.isEmpty())
            emit folderSelected(path);
    });

    // Separator
    QFrame* line2 = new QFrame(container);
    line2->setFrameShape(QFrame::HLine);
    layout->addWidget(line2);

    // Snapshot
    m_snapshotBtn = new QPushButton(tr("📷 Snapshot"), container);
    layout->addWidget(m_snapshotBtn);
    connect(m_snapshotBtn, &QPushButton::clicked,
            this, &SourcePanel::snapshotRequested);

    // FPS display
    m_fpsLabel = new QLabel(tr("FPS: --"), container);
    m_fpsLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    layout->addWidget(m_fpsLabel);

    // Info label
    m_infoLabel = new QLabel(tr("No source selected"), container);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: #888;");
    layout->addWidget(m_infoLabel);

    layout->addStretch();
    setWidget(container);
}

void SourcePanel::updateCameraList(const QList<int>& deviceIndices)
{
    m_cameraCombo->clear();
    for (int index : deviceIndices)
        m_cameraCombo->addItem(tr("Camera %1").arg(index), index);
}

void SourcePanel::updateFps(int fps)
{
    m_currentFps = fps;
    m_fpsLabel->setText(tr("FPS: %1").arg(fps));
}

void SourcePanel::setSourceInfo(const QString& text)
{
    m_infoLabel->setText(text);
}

void SourcePanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDockWidget::changeEvent(event);
}

void SourcePanel::retranslateUi()
{
    setWindowTitle(tr("Source"));
    if (m_camLabel)
        m_camLabel->setText(tr("Camera:"));
    if (m_cameraStartBtn)
        m_cameraStartBtn->setText(tr("Start"));
    if (m_cameraStopBtn)
        m_cameraStopBtn->setText(tr("Stop"));
    if (m_openFileBtn)
        m_openFileBtn->setText(tr("Open Image..."));
    if (m_openFolderBtn)
        m_openFolderBtn->setText(tr("Open Folder..."));
    if (m_snapshotBtn)
        m_snapshotBtn->setText(tr("📷 Snapshot"));

    if (m_cameraCombo) {
        const int currentIndex = m_cameraCombo->currentIndex();
        const int currentDevice = m_cameraCombo->currentData().toInt();
        for (int i = 0; i < m_cameraCombo->count(); ++i)
            m_cameraCombo->setItemText(i, tr("Camera %1").arg(m_cameraCombo->itemData(i).toInt()));
        if (currentIndex >= 0)
            m_cameraCombo->setCurrentIndex(currentIndex);
        else
            Q_UNUSED(currentDevice);
    }

    if (m_fpsLabel) {
        if (m_currentFps >= 0)
            m_fpsLabel->setText(tr("FPS: %1").arg(m_currentFps));
        else
            m_fpsLabel->setText(tr("FPS: --"));
    }
}
