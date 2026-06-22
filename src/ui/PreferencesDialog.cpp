#include "PreferencesDialog.h"
#include "utils/AppSettings.h"
#include "utils/AppPaths.h"
#include "app/Application.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QApplication>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QEvent>

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    loadFromSettings();
    retranslateUi();
}

void PreferencesDialog::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_appearanceGroup = new QGroupBox(this);
    QFormLayout* appearanceForm = new QFormLayout(m_appearanceGroup);
    m_themeCombo = new QComboBox(m_appearanceGroup);
    m_themeLabel = new QLabel(m_appearanceGroup);
    appearanceForm->addRow(m_themeLabel, m_themeCombo);

    m_languageCombo = new QComboBox(m_appearanceGroup);
    m_languageLabel = new QLabel(m_appearanceGroup);
    appearanceForm->addRow(m_languageLabel, m_languageCombo);

    QHBoxLayout* stylesheetLayout = new QHBoxLayout();
    m_stylesheetEdit = new QLineEdit(m_appearanceGroup);
    m_stylesheetBtn = new QPushButton(m_appearanceGroup);
    connect(m_stylesheetBtn, &QPushButton::clicked, this, &PreferencesDialog::browseStylesheet);
    stylesheetLayout->addWidget(m_stylesheetEdit);
    stylesheetLayout->addWidget(m_stylesheetBtn);
    m_stylesheetLabel = new QLabel(m_appearanceGroup);
    appearanceForm->addRow(m_stylesheetLabel, stylesheetLayout);
    layout->addWidget(m_appearanceGroup);

    m_pathsGroup = new QGroupBox(this);
    QFormLayout* pathsForm = new QFormLayout(m_pathsGroup);

    QHBoxLayout* pluginLayout = new QHBoxLayout();
    m_pluginDirEdit = new QLineEdit(m_pathsGroup);
    m_pluginBtn = new QPushButton(m_pathsGroup);
    connect(m_pluginBtn, &QPushButton::clicked, this, &PreferencesDialog::browsePluginDirectory);
    pluginLayout->addWidget(m_pluginDirEdit);
    pluginLayout->addWidget(m_pluginBtn);
    m_pluginDirLabel = new QLabel(m_pathsGroup);
    pathsForm->addRow(m_pluginDirLabel, pluginLayout);

    QHBoxLayout* modelsLayout = new QHBoxLayout();
    m_modelsDirEdit = new QLineEdit(m_pathsGroup);
    m_modelsBtn = new QPushButton(m_pathsGroup);
    m_openModelsBtn = new QPushButton(m_pathsGroup);
    connect(m_modelsBtn, &QPushButton::clicked, this, &PreferencesDialog::browseModelsDirectory);
    connect(m_openModelsBtn, &QPushButton::clicked, this, &PreferencesDialog::openModelsFolder);
    modelsLayout->addWidget(m_modelsDirEdit);
    modelsLayout->addWidget(m_modelsBtn);
    modelsLayout->addWidget(m_openModelsBtn);
    m_modelsDirLabel = new QLabel(m_pathsGroup);
    pathsForm->addRow(m_modelsDirLabel, modelsLayout);

    m_modelsHint = new QLabel(m_pathsGroup);
    m_modelsHint->setWordWrap(true);
    m_modelsHint->setStyleSheet("color: #888;");
    pathsForm->addRow(m_modelsHint);
    layout->addWidget(m_pathsGroup);

    m_behaviorGroup = new QGroupBox(this);
    QFormLayout* behaviorForm = new QFormLayout(m_behaviorGroup);
    m_historyMaxSpin = new QSpinBox(m_behaviorGroup);
    m_historyMaxSpin->setRange(5, 200);
    m_historyLabel = new QLabel(m_behaviorGroup);
    behaviorForm->addRow(m_historyLabel, m_historyMaxSpin);

    m_cameraWidthSpin = new QSpinBox(m_behaviorGroup);
    m_cameraWidthSpin->setRange(320, 3840);
    m_cameraWidthSpin->setSingleStep(160);
    m_cameraHeightSpin = new QSpinBox(m_behaviorGroup);
    m_cameraHeightSpin->setRange(240, 2160);
    m_cameraHeightSpin->setSingleStep(120);
    m_cameraWidthLabel = new QLabel(m_behaviorGroup);
    m_cameraHeightLabel = new QLabel(m_behaviorGroup);
    behaviorForm->addRow(m_cameraWidthLabel, m_cameraWidthSpin);
    behaviorForm->addRow(m_cameraHeightLabel, m_cameraHeightSpin);

    m_compareModeCombo = new QComboBox(m_behaviorGroup);
    m_compareModeLabel = new QLabel(m_behaviorGroup);
    behaviorForm->addRow(m_compareModeLabel, m_compareModeCombo);
    layout->addWidget(m_behaviorGroup);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults,
        this);
    connect(buttons, &QDialogButtonBox::accepted, this, &PreferencesDialog::applyChanges);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults),
            &QPushButton::clicked, this, &PreferencesDialog::restoreDefaults);
    layout->addWidget(buttons);
}

void PreferencesDialog::refreshThemeComboItems()
{
    const int current = m_themeCombo->currentData().toInt();
    m_themeCombo->clear();
    m_themeCombo->addItem(tr("Dark"), static_cast<int>(AppSettings::Theme::Dark));
    m_themeCombo->addItem(tr("Light"), static_cast<int>(AppSettings::Theme::Light));
    m_themeCombo->addItem(tr("Custom Stylesheet"), static_cast<int>(AppSettings::Theme::Custom));
    const int index = m_themeCombo->findData(current);
    if (index >= 0)
        m_themeCombo->setCurrentIndex(index);
}

void PreferencesDialog::refreshLanguageComboItems()
{
    const int current = m_languageCombo->currentData().toInt();
    m_languageCombo->clear();
    m_languageCombo->addItem(tr("English"), static_cast<int>(AppSettings::Language::English));
    m_languageCombo->addItem(tr("Chinese"), static_cast<int>(AppSettings::Language::Chinese));
    const int index = m_languageCombo->findData(current);
    if (index >= 0)
        m_languageCombo->setCurrentIndex(index);
}

void PreferencesDialog::refreshCompareModeComboItems()
{
    const int current = m_compareModeCombo->currentData().toInt();
    m_compareModeCombo->clear();
    m_compareModeCombo->addItem(tr("Side by Side"), 0);
    m_compareModeCombo->addItem(tr("Overlay"), 1);
    m_compareModeCombo->addItem(tr("Difference"), 2);
    const int index = m_compareModeCombo->findData(current);
    if (index >= 0)
        m_compareModeCombo->setCurrentIndex(index);
}

void PreferencesDialog::retranslateUi()
{
    setWindowTitle(tr("Preferences"));
    m_appearanceGroup->setTitle(tr("Appearance"));
    m_pathsGroup->setTitle(tr("Paths"));
    m_behaviorGroup->setTitle(tr("Behavior"));

    m_themeLabel->setText(tr("Theme:"));
    m_languageLabel->setText(tr("Language:"));
    m_stylesheetLabel->setText(tr("Stylesheet:"));
    m_pluginDirLabel->setText(tr("Plugin directory:"));
    m_modelsDirLabel->setText(tr("Models directory:"));
    m_historyLabel->setText(tr("History items:"));
    m_cameraWidthLabel->setText(tr("Camera width:"));
    m_cameraHeightLabel->setText(tr("Camera height:"));
    m_compareModeLabel->setText(tr("Default compare mode:"));

    m_stylesheetBtn->setText(tr("Browse..."));
    m_pluginBtn->setText(tr("Browse..."));
    m_modelsBtn->setText(tr("Browse..."));
    m_openModelsBtn->setText(tr("Open Folder"));

    m_modelsHint->setText(
        tr("DNN face detection needs deploy.prototxt and res10_300x300_ssd_iter_140000.caffemodel.\n"
           "Run scripts/download_dnn_models.bat if they are missing."));

    refreshThemeComboItems();
    refreshLanguageComboItems();
    refreshCompareModeComboItems();
}

void PreferencesDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(event);
}

void PreferencesDialog::loadFromSettings()
{
    refreshThemeComboItems();
    refreshLanguageComboItems();
    refreshCompareModeComboItems();

    m_themeCombo->setCurrentIndex(
        m_themeCombo->findData(static_cast<int>(AppSettings::theme())));
    m_languageCombo->setCurrentIndex(
        m_languageCombo->findData(static_cast<int>(AppSettings::language())));
    m_stylesheetEdit->setText(AppSettings::customStylesheetPath());
    m_pluginDirEdit->setText(AppSettings::customPluginDirectory());
    m_modelsDirEdit->setText(AppSettings::customModelsDirectory());
    m_historyMaxSpin->setValue(AppSettings::historyMaxItems());
    m_cameraWidthSpin->setValue(AppSettings::cameraWidth());
    m_cameraHeightSpin->setValue(AppSettings::cameraHeight());
    m_compareModeCombo->setCurrentIndex(
        m_compareModeCombo->findData(AppSettings::defaultCompareMode()));
}

void PreferencesDialog::browseStylesheet()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Select Stylesheet"), {},
        tr("Qt Stylesheets (*.qss);;All Files (*)"));
    if (!path.isEmpty())
        m_stylesheetEdit->setText(path);
}

void PreferencesDialog::browsePluginDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(
        this, tr("Select Plugin Directory"));
    if (!path.isEmpty())
        m_pluginDirEdit->setText(path);
}

void PreferencesDialog::browseModelsDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(
        this, tr("Select Models Directory"));
    if (!path.isEmpty())
        m_modelsDirEdit->setText(path);
}

void PreferencesDialog::openModelsFolder()
{
    const QString path = m_modelsDirEdit->text().trimmed();
    const QString target = path.isEmpty() ? AppPaths::resolveModelsDirectory() : path;
    QDir().mkpath(target);
    QDesktopServices::openUrl(QUrl::fromLocalFile(target));
}

void PreferencesDialog::applyChanges()
{
    AppSettings::setTheme(static_cast<AppSettings::Theme>(
        m_themeCombo->currentData().toInt()));
    AppSettings::setLanguage(static_cast<AppSettings::Language>(
        m_languageCombo->currentData().toInt()));
    AppSettings::setCustomStylesheetPath(m_stylesheetEdit->text().trimmed());
    AppSettings::setCustomPluginDirectory(m_pluginDirEdit->text().trimmed());
    AppSettings::setCustomModelsDirectory(m_modelsDirEdit->text().trimmed());
    AppSettings::setHistoryMaxItems(m_historyMaxSpin->value());
    AppSettings::setCameraWidth(m_cameraWidthSpin->value());
    AppSettings::setCameraHeight(m_cameraHeightSpin->value());
    AppSettings::setDefaultCompareMode(m_compareModeCombo->currentData().toInt());

    if (auto* app = qobject_cast<Application*>(QApplication::instance()))
        AppSettings::applyTheme(app);

    accept();
}

void PreferencesDialog::restoreDefaults()
{
    m_themeCombo->setCurrentIndex(0);
    m_languageCombo->setCurrentIndex(0);
    m_stylesheetEdit->clear();
    m_pluginDirEdit->clear();
    m_modelsDirEdit->clear();
    m_historyMaxSpin->setValue(50);
    m_cameraWidthSpin->setValue(1280);
    m_cameraHeightSpin->setValue(720);
    m_compareModeCombo->setCurrentIndex(0);
}
