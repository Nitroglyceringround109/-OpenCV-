cmake_minimum_required(VERSION 3.16)

project(SmartVisionWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ---- Find Qt6 ----
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent LinguistTools)

# ---- Find OpenCV ----
if(DEFINED ENV{OpenCV_DIR})
    set(OpenCV_DIR $ENV{OpenCV_DIR})
endif()
if(NOT OpenCV_DIR)
    if(EXISTS "C:/opencv-build/install")
        set(OpenCV_DIR "C:/opencv-build/install")
    endif()
endif()
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")

# ---- Shared resource paths ----
set(SVW_MODELS_DIR ${CMAKE_SOURCE_DIR}/resources/models)
set(SVW_DNN_PROTO ${SVW_MODELS_DIR}/deploy.prototxt)
set(SVW_DNN_WEIGHTS ${SVW_MODELS_DIR}/res10_300x300_ssd_iter_140000.caffemodel)
set(SVW_WECHAT_QR_DIR ${SVW_MODELS_DIR}/wechat_qrcode)

file(MAKE_DIRECTORY ${SVW_WECHAT_QR_DIR})

set(SVW_WECHAT_QR_FILES
    detect.prototxt
    detect.caffemodel
    sr.prototxt
    sr.caffemodel
)

foreach(_svw_wechat_file IN LISTS SVW_WECHAT_QR_FILES)
    set(_svw_wechat_path ${SVW_WECHAT_QR_DIR}/${_svw_wechat_file})
    if(NOT EXISTS ${_svw_wechat_path})
        message(STATUS "WeChat QR model missing, attempting download: ${_svw_wechat_file}")
        file(DOWNLOAD
            "https://raw.githubusercontent.com/WeChatCV/opencv_3rdparty/wechat_qrcode/${_svw_wechat_file}"
            ${_svw_wechat_path}
            STATUS _svw_wechat_download_status
        )
        list(GET _svw_wechat_download_status 0 _svw_wechat_download_code)
        if(NOT _svw_wechat_download_code EQUAL 0 OR NOT EXISTS ${_svw_wechat_path})
            file(REMOVE ${_svw_wechat_path})
            message(WARNING "Failed to download WeChat QR model ${_svw_wechat_file}. Run scripts/download_wechat_qr_models.bat")
        endif()
    endif()
endforeach()

if(NOT EXISTS ${SVW_DNN_PROTO})
    message(WARNING "DNN prototxt missing at ${SVW_DNN_PROTO}")
endif()

if(NOT EXISTS ${SVW_DNN_WEIGHTS})
    message(STATUS "DNN face weights not found, attempting download...")
    file(DOWNLOAD
        "https://github.com/opencv/opencv_3rdparty/raw/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel"
        ${SVW_DNN_WEIGHTS}
        STATUS _svw_download_status
        SHOW_PROGRESS
    )
    list(GET _svw_download_status 0 _svw_download_code)
    if(NOT _svw_download_code EQUAL 0 OR NOT EXISTS ${SVW_DNN_WEIGHTS})
        file(REMOVE ${SVW_DNN_WEIGHTS})
        message(WARNING "Failed to download DNN face weights. Run scripts/download_dnn_models.bat or place res10_300x300_ssd_iter_140000.caffemodel in resources/models/")
    else()
        file(SIZE ${SVW_DNN_WEIGHTS} _svw_weights_size)
        if(_svw_weights_size LESS 1000000)
            file(REMOVE ${SVW_DNN_WEIGHTS})
            message(WARNING "Downloaded DNN weights look invalid (${_svw_weights_size} bytes). Run scripts/download_dnn_models.bat manually.")
        endif()
    endif()
endif()

# ---- Plugin output directory ----
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
file(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})

# ---- Main application ----
add_subdirectory(src)

# ---- Plugins ----
add_subdirectory(plugins/color_conversion)
add_subdirectory(plugins/edge_detection)
add_subdirectory(plugins/threshold)
add_subdirectory(plugins/morphology)
add_subdirectory(plugins/filtering)
add_subdirectory(plugins/face_detection)
add_subdirectory(plugins/feature_matching)
add_subdirectory(plugins/qr_detection)

# ---- Deployment / install ----
include(GNUInstallDirs)

install(TARGETS SmartVisionWorkbench
    RUNTIME DESTINATION .
    BUNDLE DESTINATION .
)

if(WIN32)
    set(_svw_plugin_glob *.dll)
elseif(APPLE)
    set(_svw_plugin_glob *.dylib)
else()
    set(_svw_plugin_glob *.so)
endif()

install(DIRECTORY ${PLUGIN_OUTPUT_DIR}/
    DESTINATION plugins
    FILES_MATCHING PATTERN ${_svw_plugin_glob}
)

install(DIRECTORY ${SVW_MODELS_DIR}/
    DESTINATION models
    FILES_MATCHING
        PATTERN "deploy.prototxt"
        PATTERN "res10_300x300_ssd_iter_140000.caffemodel"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/cascades/
    DESTINATION models/cascades
    FILES_MATCHING PATTERN "*.xml"
)
