#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class QComboBox;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QLabel;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);

private slots:
    void browseStylesheet();
    void browsePluginDirectory();
    void browseModelsDirectory();
    void openModelsFolder();
    void applyChanges();
    void restoreDefaults();

protected:
    void changeEvent(QEvent* event) override;

private:
    void loadFromSettings();
    void setupUi();
    void retranslateUi();
    void refreshThemeComboItems();
    void refreshCompareModeComboItems();
    void refreshLanguageComboItems();

    QGroupBox* m_appearanceGroup = nullptr;
    QGroupBox* m_pathsGroup = nullptr;
    QGroupBox* m_behaviorGroup = nullptr;
    QLabel* m_themeLabel = nullptr;
    QLabel* m_stylesheetLabel = nullptr;
    QLabel* m_pluginDirLabel = nullptr;
    QLabel* m_modelsDirLabel = nullptr;
    QLabel* m_modelsHint = nullptr;
    QLabel* m_historyLabel = nullptr;
    QLabel* m_cameraWidthLabel = nullptr;
    QLabel* m_cameraHeightLabel = nullptr;
    QLabel* m_compareModeLabel = nullptr;
    QLabel* m_languageLabel = nullptr;
    QPushButton* m_stylesheetBtn = nullptr;
    QPushButton* m_pluginBtn = nullptr;
    QPushButton* m_modelsBtn = nullptr;
    QPushButton* m_openModelsBtn = nullptr;

    QComboBox* m_themeCombo = nullptr;
    QComboBox* m_languageCombo = nullptr;
    QLineEdit* m_stylesheetEdit = nullptr;
    QLineEdit* m_pluginDirEdit = nullptr;
    QLineEdit* m_modelsDirEdit = nullptr;
    QSpinBox* m_historyMaxSpin = nullptr;
    QSpinBox* m_cameraWidthSpin = nullptr;
    QSpinBox* m_cameraHeightSpin = nullptr;
    QComboBox* m_compareModeCombo = nullptr;
};

#endif // PREFERENCESDIALOG_H
