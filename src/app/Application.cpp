#include "Application.h"
#include <QFile>
#include <QDebug>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setOrganizationName("SmartVisionWorkbench");
    setOrganizationDomain("org.svw");
    setApplicationName("Smart Vision Workbench");
    setApplicationVersion("1.0.0");
}

void Application::loadStylesheet(const QString& path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        setStyleSheet(style);
        qDebug() << "Loaded stylesheet:" << path;
    } else {
        qWarning() << "Failed to load stylesheet:" << path;
    }
}

void Application::loadDefaultTheme()
{
    // Dark theme inline (fallback if .qss file not found)
    QString darkTheme = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QDockWidget {
            color: #cccccc;
            titlebar-close-icon: none;
        }
        QDockWidget::title {
            background-color: #2d2d2d;
            padding: 4px;
        }
        QListWidget {
            background-color: #252525;
            color: #cccccc;
            border: 1px solid #3d3d3d;
        }
        QListWidget::item:selected {
            background-color: #094771;
        }
        QListWidget::item:hover {
            background-color: #2a2d2e;
        }
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: 1px solid #0e639c;
            padding: 5px 15px;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #0d5689;
        }
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #666666;
        }
        QComboBox {
            background-color: #3c3c3c;
            color: #cccccc;
            border: 1px solid #555555;
            padding: 3px;
        }
        QSpinBox, QDoubleSpinBox {
            background-color: #3c3c3c;
            color: #cccccc;
            border: 1px solid #555555;
            padding-right: 18px;
            min-height: 22px;
            min-width: 72px;
        }
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 16px;
            border-left: 1px solid #555555;
            background-color: #4a4a4a;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 16px;
            border-left: 1px solid #555555;
            background-color: #4a4a4a;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #5a5a5a;
        }
        QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
            width: 8px;
            height: 8px;
        }
        QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
            width: 8px;
            height: 8px;
        }
        QGroupBox {
            color: #cccccc;
            border: 1px solid #3d3d3d;
            margin-top: 8px;
            padding-top: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 3px;
        }
        QCheckBox {
            color: #cccccc;
        }
        QLabel {
            color: #cccccc;
        }
        QMenuBar {
            background-color: #2d2d2d;
            color: #cccccc;
        }
        QMenuBar::item:selected {
            background-color: #094771;
        }
        QMenu {
            background-color: #252525;
            color: #cccccc;
            border: 1px solid #3d3d3d;
        }
        QMenu::item:selected {
            background-color: #094771;
        }
        QStatusBar {
            background-color: #007acc;
            color: white;
        }
        QToolBar {
            background-color: #2d2d2d;
            border: none;
            spacing: 3px;
            padding: 2px;
        }
        QSplitter::handle {
            background-color: #3d3d3d;
        }
        QScrollArea {
            border: none;
        }
    )";
    setStyleSheet(darkTheme);
}

void Application::loadLightTheme()
{
    QString lightTheme = R"(
        QMainWindow { background-color: #f3f3f3; }
        QDockWidget { color: #333333; }
        QDockWidget::title { background-color: #e0e0e0; padding: 4px; }
        QListWidget { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; }
        QListWidget::item:selected { background-color: #cce8ff; }
        QPushButton {
            background-color: #0078d4; color: white; border: 1px solid #0078d4;
            padding: 5px 15px; border-radius: 3px;
        }
        QPushButton:hover { background-color: #106ebe; }
        QPushButton:disabled { background-color: #cccccc; color: #666666; }
        QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit {
            background-color: #ffffff; color: #333333; border: 1px solid #cccccc; padding: 3px;
        }
        QSpinBox, QDoubleSpinBox {
            padding-right: 18px;
            min-height: 22px;
            min-width: 72px;
        }
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 16px;
            border-left: 1px solid #cccccc;
            background-color: #f0f0f0;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 16px;
            border-left: 1px solid #cccccc;
            background-color: #f0f0f0;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #e0e0e0;
        }
        QGroupBox { color: #333333; border: 1px solid #cccccc; margin-top: 8px; padding-top: 12px; }
        QLabel { color: #333333; }
        QMenuBar { background-color: #e0e0e0; color: #333333; }
        QMenu { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; }
        QMenu::item:selected { background-color: #cce8ff; }
        QStatusBar { background-color: #0078d4; color: white; }
        QToolBar { background-color: #e0e0e0; border: none; }
        QSplitter::handle { background-color: #cccccc; }
    )";
    setStyleSheet(lightTheme);
}
