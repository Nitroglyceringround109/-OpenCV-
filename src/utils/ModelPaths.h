#ifndef MODELPATHS_H
#define MODELPATHS_H

#include <QString>

namespace ModelPaths {

struct DnnFaceModelPaths {
    QString prototxt;
    QString weights;
    bool isValid() const { return !prototxt.isEmpty() && !weights.isEmpty(); }
};

struct WeChatQrModelPaths {
    QString detectProto;
    QString detectModel;
    QString srProto;
    QString srModel;
    bool isValid() const {
        return !detectProto.isEmpty() && !detectModel.isEmpty()
            && !srProto.isEmpty() && !srModel.isEmpty();
    }
};

DnnFaceModelPaths resolveDnnFaceModel();
WeChatQrModelPaths resolveWeChatQrModels();

/// Resolve Haar cascade XML to a real filesystem path (OpenCV cannot read Qt :/ resources).
QString resolveHaarCascade(const QString& cascadeFileName);
QString resolveHaarFaceCascade();
QString resolveHaarProfileCascade();

} // namespace ModelPaths

#endif // MODELPATHS_H
