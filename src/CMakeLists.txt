# Smart Vision Workbench - src CMakeLists

set(APP_SOURCES
    app/Application.cpp
    app/MainWindow.cpp
)

set(CORE_SOURCES
    core/ImageData.cpp
    core/ProcessingPipeline.cpp
    core/PluginManager.cpp
    core/ProjectModel.cpp
)

set(CAPTURE_SOURCES
    capture/CameraDevice.cpp
    capture/FrameGrabber.cpp
    capture/ImageFileSource.cpp
)

set(UI_SOURCES
    ui/ImageCanvas.cpp
    ui/ComparisonView.cpp
    ui/PipelinePanel.cpp
    ui/ParameterPanel.cpp
    ui/SourcePanel.cpp
    ui/HistoryPanel.cpp
    ui/PreferencesDialog.cpp
)

set(UTIL_SOURCES
    utils/CvMatToQImage.cpp
    utils/JsonHelper.cpp
    utils/AppPaths.cpp
    utils/AppSettings.cpp
    utils/ModelPaths.cpp
    utils/LanguageManager.cpp
    utils/ProcessorLocalization.cpp
)

set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

qt_add_executable(SmartVisionWorkbench
    MANUAL_FINALIZATION
    main.cpp
    ${APP_SOURCES}
    ${CORE_SOURCES}
    ${CAPTURE_SOURCES}
    ${UI_SOURCES}
    ${UTIL_SOURCES}
    ${RESOURCE_FILES}
)

target_include_directories(SmartVisionWorkbench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(SmartVisionWorkbench PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    ${OpenCV_LIBS}
)

target_compile_definitions(SmartVisionWorkbench PRIVATE
    PLUGIN_DIR="${PLUGIN_OUTPUT_DIR}"
    SVW_MODELS_DIR="${SVW_MODELS_DIR}"
)

add_dependencies(SmartVisionWorkbench
    svw_color_conversion
    svw_edge_detection
    svw_threshold
    svw_morphology
    svw_filtering
    svw_face_detection
    svw_feature_matching
    svw_qr_detection
)

set_target_properties(SmartVisionWorkbench PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

qt_finalize_executable(SmartVisionWorkbench)

qt_add_translations(SmartVisionWorkbench
    TS_FILES ${CMAKE_SOURCE_DIR}/translations/svw_zh_CN.ts
    RESOURCE_PREFIX /i18n
)

add_custom_command(TARGET SmartVisionWorkbench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SmartVisionWorkbench>/models
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SVW_DNN_PROTO} $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/deploy.prototxt
    COMMENT "Copying DNN prototxt to output directory"
)

if(EXISTS ${SVW_DNN_WEIGHTS})
    file(SIZE ${SVW_DNN_WEIGHTS} _svw_installed_weights_size)
    if(_svw_installed_weights_size GREATER 1000000)
        add_custom_command(TARGET SmartVisionWorkbench POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SVW_DNN_WEIGHTS} $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/res10_300x300_ssd_iter_140000.caffemodel
            COMMENT "Copying DNN weights to output directory"
        )
    endif()
endif()

add_custom_command(TARGET SmartVisionWorkbench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/cascades
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/cascades
        $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/cascades
    COMMENT "Copying Haar cascades to output directory"
)

if(EXISTS ${SVW_WECHAT_QR_DIR}/detect.caffemodel)
    add_custom_command(TARGET SmartVisionWorkbench POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/wechat_qrcode
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SVW_WECHAT_QR_DIR}
            $<TARGET_FILE_DIR:SmartVisionWorkbench>/models/wechat_qrcode
        COMMENT "Copying WeChat QR models to output directory"
    )
endif()
