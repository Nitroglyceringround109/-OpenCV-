#include "app/Application.h"
#include "app/MainWindow.h"
#include "core/ImageData.h"
#include "utils/AppSettings.h"
#include "utils/LanguageManager.h"

int main(int argc, char* argv[])
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<ImageData>("ImageData");

    Application app(argc, argv);
    AppSettings::initialize();
    AppSettings::applyTheme(&app);
    LanguageManager::install(&app, AppSettings::language());

    MainWindow window;
    window.show();

    return app.exec();
}
