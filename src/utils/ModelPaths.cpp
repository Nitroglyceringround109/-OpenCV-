#include "ModelPaths.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace ModelPaths {

namespace {
constexpr const char* kPrototxtName = "deploy.prototxt";
constexpr const char* kWeightsName = "res10_300x300_ssd_iter_140000.caffemodel";

bool filesExist(const QString& protoPath, const QString& weightsPath,
                QString* prototxt, QString* weights)
{
    if (!QFile::exists(protoPath))
        return false;
    if (!QFile::exists(weightsPath) || QFileInfo(weightsPath).size() < 1000000)
        return false;

    if (prototxt) *prototxt = protoPath;
    if (weights) *weights = weightsPath;
    return true;
}

QString extractResourceToCache(const QString& resourcePath, const QString& fileName,
                               const QString& cacheSubDir)
{
    QFile input(resourcePath);
    if (!input.open(QIODevice::ReadOnly))
        return {};

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QLatin1Char('/') + cacheSubDir;
    QDir().mkpath(cacheDir);

    const QString outputPath = QDir(cacheDir).filePath(fileName);
    if (QFile::exists(outputPath))
        return outputPath;

    QFile output(outputPath);
    if (!output.open(QIODevice::WriteOnly))
        return {};

    output.write(input.readAll());
    return outputPath;
}

QString extractResource(const QString& resourcePath, const QString& fileName)
{
    return extractResourceToCache(resourcePath, fileName, QStringLiteral("models"));
}
} // namespace

DnnFaceModelPaths resolveDnnFaceModel()
{
    DnnFaceModelPaths paths;

    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList modelDirs = {
        appDir + QStringLiteral("/models"),
        QDir(appDir).filePath(QStringLiteral("../resources/models")),
    };

#ifdef SVW_MODELS_DIR
    modelDirs << QStringLiteral(SVW_MODELS_DIR);
#endif

    for (const QString& dir : modelDirs) {
        if (filesExist(QDir(dir).filePath(kPrototxtName),
                       QDir(dir).filePath(kWeightsName),
                       &paths.prototxt, &paths.weights)) {
            return paths;
        }
    }

    const QString resourceProto = QStringLiteral(":/models/deploy.prototxt");
    const QString resourceWeights = QStringLiteral(":/models/res10_300x300_ssd_iter_140000.caffemodel");
    if (QFile::exists(resourceProto) && QFile::exists(resourceWeights)) {
        paths.prototxt = extractResource(resourceProto, kPrototxtName);
        paths.weights = extractResource(resourceWeights, kWeightsName);
        return paths;
    }

    if (QFile::exists(resourceProto)) {
        paths.prototxt = extractResource(resourceProto, kPrototxtName);
        const QString weightsOnDisk = QDir(appDir).filePath(
            QStringLiteral("models/") + kWeightsName);
        if (QFile::exists(weightsOnDisk))
            paths.weights = weightsOnDisk;
    }

    return paths;
}

WeChatQrModelPaths resolveWeChatQrModels()
{
    WeChatQrModelPaths paths;
    const QString appDir = QCoreApplication::applicationDirPath();

    QStringList modelDirs = {
        appDir + QStringLiteral("/models/wechat_qrcode"),
        QDir(appDir).filePath(QStringLiteral("../resources/models/wechat_qrcode")),
    };

#ifdef SVW_MODELS_DIR
    modelDirs << QDir(QStringLiteral(SVW_MODELS_DIR)).filePath(QStringLiteral("wechat_qrcode"));
#endif

    const QStringList requiredFiles = {
        QStringLiteral("detect.prototxt"),
        QStringLiteral("detect.caffemodel"),
        QStringLiteral("sr.prototxt"),
        QStringLiteral("sr.caffemodel"),
    };

    for (const QString& dir : modelDirs) {
        bool allExist = true;
        for (const QString& file : requiredFiles) {
            if (!QFile::exists(QDir(dir).filePath(file))) {
                allExist = false;
                break;
            }
        }
        if (!allExist)
            continue;

        paths.detectProto = QDir(dir).filePath(requiredFiles[0]);
        paths.detectModel = QDir(dir).filePath(requiredFiles[1]);
        paths.srProto = QDir(dir).filePath(requiredFiles[2]);
        paths.srModel = QDir(dir).filePath(requiredFiles[3]);
        return paths;
    }

    return paths;
}

QString resolveHaarCascade(const QString& cascadeFileName)
{
    const QString fileName = cascadeFileName.isEmpty()
        ? QStringLiteral("haarcascade_frontalface_alt2.xml")
        : cascadeFileName;

    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList cascadeDirs = {
        appDir + QStringLiteral("/models/cascades"),
        QDir(appDir).filePath(QStringLiteral("../resources/cascades")),
    };

#ifdef SVW_MODELS_DIR
    cascadeDirs << QDir(QStringLiteral(SVW_MODELS_DIR)).filePath(QStringLiteral("../cascades"));
#endif

    for (const QString& dir : cascadeDirs) {
        const QString path = QDir(dir).filePath(fileName);
        if (QFile::exists(path))
            return path;
    }

    const QString resourcePath = QStringLiteral(":/cascades/") + fileName;
    if (QFile::exists(resourcePath)) {
        const QString extracted = extractResourceToCache(
            resourcePath, fileName, QStringLiteral("cascades"));
        if (!extracted.isEmpty())
            return extracted;
    }

    return {};
}

QString resolveHaarFaceCascade()
{
    return resolveHaarCascade(QStringLiteral("haarcascade_frontalface_alt2.xml"));
}

QString resolveHaarProfileCascade()
{
    return resolveHaarCascade(QStringLiteral("haarcascade_profileface.xml"));
}

} // namespace ModelPaths
