@echo off
setlocal
set "MODEL_DIR=%~dp0resources\models"
set "WEIGHTS=%MODEL_DIR%\res10_300x300_ssd_iter_140000.caffemodel"
set "PROTO=%MODEL_DIR%\deploy.prototxt"

if not exist "%MODEL_DIR%" mkdir "%MODEL_DIR%"

if not exist "%PROTO%" (
    echo Downloading deploy.prototxt...
    curl -L --retry 3 -o "%PROTO%" "https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt"
)

if not exist "%WEIGHTS%" (
    echo Downloading DNN face weights (~10 MB)...
    curl --ssl-no-revoke -L --retry 5 -o "%WEIGHTS%" "https://github.com/opencv/opencv_3rdparty/raw/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel"
)

if exist "%WEIGHTS%" (
    echo DNN face model files are ready in %MODEL_DIR%
) else (
    echo Failed to download weights. Copy res10_300x300_ssd_iter_140000.caffemodel manually.
    exit /1
)

endlocal
