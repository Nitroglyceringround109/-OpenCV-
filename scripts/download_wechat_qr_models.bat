@echo off
setlocal

set "DEST=%~dp0..\resources\models\wechat_qrcode"
if not exist "%DEST%" mkdir "%DEST%"

set "BASE=https://raw.githubusercontent.com/WeChatCV/opencv_3rdparty/wechat_qrcode"

for %%F in (detect.prototxt detect.caffemodel sr.prototxt sr.caffemodel) do (
    if not exist "%DEST%\%%F" (
        echo Downloading %%F ...
        curl -L --ssl-no-revoke -o "%DEST%\%%F" "%BASE%/%%F"
    ) else (
        echo Already present: %%F
    )
)

echo.
echo WeChat QR models directory: %DEST%
endlocal
