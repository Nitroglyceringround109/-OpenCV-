#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

/**
 * @brief QApplication subclass for global initialization.
 *
 * Sets up organization name, loads QSS theme, installs translators,
 * and configures high-DPI attributes.
 */
class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);

    /// Load a Qt stylesheet from the given file path
    void loadStylesheet(const QString& path);

    /// Load the default dark theme
    void loadDefaultTheme();

    /// Load the built-in light theme
    void loadLightTheme();
};

#endif // APPLICATION_H
