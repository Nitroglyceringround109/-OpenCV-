#ifndef PROCESSINGPIPELINE_H
#define PROCESSINGPIPELINE_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QAtomicInt>
#include <cstdint>
#include "PipelineNode.h"
#include "ImageData.h"

/**
 * @brief Ordered chain of IImageProcessor steps.
 *
 * The pipeline owns an ordered list of PipelineNodes and applies
 * them sequentially. It runs processing on a worker thread to
 * keep the UI responsive, and implements frame-dropping for
 * real-time camera mode.
 *
 * Signal flow:
 *   processFrame(input) → [step1 → step2 → ...] → pipelineFinished(output)
 */
class ProcessingPipeline : public QObject
{
    Q_OBJECT

public:
    explicit ProcessingPipeline(QObject* parent = nullptr);
    ~ProcessingPipeline();

    // --- Pipeline management ---
    void insertProcessor(int index, IImageProcessor* proc);
    void removeProcessor(int index);
    void moveProcessor(int from, int to);
    void setProcessorEnabled(int index, bool enabled);
    void clearPipeline();

    int processorCount() const;
    PipelineNode nodeAt(int index) const;
    QList<PipelineNode> allNodes() const;

    // --- Second image (for feature matching) ---
    void setSecondImage(const ImageData& img);
    void clearSecondImage();
    bool hasSecondImage() const;

    // --- Processing ---
    /// Get the last input image (for reprocessing when params change)
    ImageData lastInput() const;

public slots:
    /// Process the input through all enabled processors.
    /// Thread-safe: can be called from any thread.
    void processFrame(const ImageData& input);

    /// Reprocess the last input (e.g., when a parameter changes)
    void reprocess();

signals:
    /// Emitted when pipeline processing completes
    void pipelineFinished(const ImageData& output);

    /// Emitted when a node's enabled state or params change
    void pipelineNodeChanged(int index);

    /// Emitted when nodes are added/removed/reordered
    void pipelineStructureChanged();

    /// Processing time in milliseconds (for status bar)
    void processingTimeMs(int ms);

    /// Per-processor timing info
    void processorTimeMs(const QString& processorId, int ms);

private:
    ImageData runPipeline(const ImageData& input);
    void finishProcessing(const ImageData& result, std::uint64_t requestId);

    QList<PipelineNode> m_nodes;
    mutable QMutex m_mutex;
    QAtomicInt m_processing;  ///< 1 if currently processing, 0 otherwise
    QAtomicInt m_frameDropped; ///< Flag for frame-dropping
    std::uint64_t m_requestId = 0;

    ImageData m_lastInput;
    ImageData m_secondImage;
    bool m_hasSecondImage = false;
};

#endif // PROCESSINGPIPELINE_H
