#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

class QApplication;

namespace AppSettings {
enum class Language;
}

namespace LanguageManager {

void install(QApplication* app, AppSettings::Language language);
void notifyLanguageChanged(QApplication* app);

} // namespace LanguageManager

#endif // LANGUAGEMANAGER_H
