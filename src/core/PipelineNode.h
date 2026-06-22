#ifndef PIPELINENODE_H
#define PIPELINENODE_H

#include "IImageProcessor.h"
#include <memory>

/**
 * @brief Lightweight wrapper: processor + enabled state.
 *
 * PipelineNode is a value struct owned by ProcessingPipeline.
 * It does NOT inherit QObject.
 */
struct PipelineNode
{
    IImageProcessor* processor = nullptr;
    bool enabled = true;

    PipelineNode() = default;
    explicit PipelineNode(IImageProcessor* proc, bool en = true)
        : processor(proc), enabled(en) {}

    bool isValid() const { return processor != nullptr; }
};

#endif // PIPELINENODE_H
