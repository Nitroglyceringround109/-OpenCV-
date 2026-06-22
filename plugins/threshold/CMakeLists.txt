# Threshold Plugin CMakeLists

set(PLUGIN_NAME svw_threshold)

add_library(${PLUGIN_NAME} SHARED
    ${CMAKE_SOURCE_DIR}/src/core/ImageData.cpp
    ThresholdPlugin.cpp
    BinaryThresholdProcessor.cpp
    AdaptiveThresholdProcessor.cpp
    OtsuThresholdProcessor.cpp
)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
)

# Output to plugin directory
set_target_properties(${PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)
