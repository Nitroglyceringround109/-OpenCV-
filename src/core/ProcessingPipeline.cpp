#include "ProcessingPipeline.h"
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QMetaObject>
#include <utility>

ProcessingPipeline::ProcessingPipeline(QObject* parent)
    : QObject(parent)
    , m_processing(0)
    , m_frameDropped(0)
{
}

ProcessingPipeline::~ProcessingPipeline()
{
    clearPipeline();
}

void ProcessingPipeline::insertProcessor(int index, IImageProcessor* proc)
{
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index > m_nodes.size())
        index = m_nodes.size();
    m_nodes.insert(index, PipelineNode(proc));
    locker.unlock();
    emit pipelineStructureChanged();
}

void ProcessingPipeline::removeProcessor(int index)
{
    IImageProcessor* proc = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        if (index < 0 || index >= m_nodes.size())
            return;
        proc = m_nodes[index].processor;
        m_nodes.removeAt(index);
    }
    delete proc;
    emit pipelineStructureChanged();
}

void ProcessingPipeline::moveProcessor(int from, int to)
{
    QMutexLocker locker(&m_mutex);
    if (from < 0 || from >= m_nodes.size()) return;
    if (to < 0 || to >= m_nodes.size()) return;
    m_nodes.move(from, to);
    locker.unlock();
    emit pipelineStructureChanged();
}

void ProcessingPipeline::setProcessorEnabled(int index, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index >= m_nodes.size()) return;
    m_nodes[index].enabled = enabled;
    locker.unlock();
    emit pipelineNodeChanged(index);
}

void ProcessingPipeline::clearPipeline()
{
    QList<IImageProcessor*> toDelete;
    {
        QMutexLocker locker(&m_mutex);
        for (const auto& node : m_nodes) {
            if (node.processor)
                toDelete << node.processor;
        }
        m_nodes.clear();
    }
    qDeleteAll(toDelete);
    emit pipelineStructureChanged();
}

int ProcessingPipeline::processorCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_nodes.size();
}

PipelineNode ProcessingPipeline::nodeAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    return m_nodes.at(index);
}

QList<PipelineNode> ProcessingPipeline::allNodes() const
{
    QMutexLocker locker(&m_mutex);
    return m_nodes;
}

void ProcessingPipeline::setSecondImage(const ImageData& img)
{
    QMutexLocker locker(&m_mutex);
    m_secondImage = img;
    m_hasSecondImage = true;
}

void ProcessingPipeline::clearSecondImage()
{
    QMutexLocker locker(&m_mutex);
    m_secondImage = ImageData();
    m_hasSecondImage = false;
}

bool ProcessingPipeline::hasSecondImage() const
{
    QMutexLocker locker(&m_mutex);
    return m_hasSecondImage;
}

ImageData ProcessingPipeline::lastInput() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastInput;
}

void ProcessingPipeline::processFrame(const ImageData& input)
{
    std::uint64_t requestId = 0;
    {
        QMutexLocker locker(&m_mutex);
        m_lastInput = input;
        m_requestId++;
        requestId = m_requestId;
    }

    if (m_processing.testAndSetRelaxed(0, 1)) {
        (void)QtConcurrent::run([this, requestId]() {
            ImageData inputCopy;
            {
                QMutexLocker locker(&m_mutex);
                inputCopy = m_lastInput;
            }

            ImageData result = runPipeline(inputCopy);
            finishProcessing(result, requestId);
        });
    } else {
        m_frameDropped.storeRelaxed(1);
    }
}

void ProcessingPipeline::finishProcessing(const ImageData& result, std::uint64_t requestId)
{
    m_processing.storeRelaxed(0);

    bool stale = false;
    {
        QMutexLocker locker(&m_mutex);
        stale = requestId != m_requestId;
    }

    if (!stale)
        emit pipelineFinished(result);

    if (stale || m_frameDropped.testAndSetRelaxed(1, 0)) {
        QMetaObject::invokeMethod(this, &ProcessingPipeline::reprocess,
                                  Qt::QueuedConnection);
    }
}

void ProcessingPipeline::reprocess()
{
    ImageData input;
    {
        QMutexLocker locker(&m_mutex);
        input = m_lastInput;
    }
    if (!input.isEmpty())
        processFrame(input);
}

ImageData ProcessingPipeline::runPipeline(const ImageData& input)
{
    if (input.isEmpty())
        return input;

    QElapsedTimer totalTimer;
    totalTimer.start();

    QList<PipelineNode> nodes;
    ImageData secondImage;
    bool hasSecondImage = false;
    {
        QMutexLocker locker(&m_mutex);
        nodes = m_nodes;
        secondImage = m_secondImage;
        hasSecondImage = m_hasSecondImage;
    }

    ImageData current = input;

    for (const auto& node : nodes) {
        if (!node.enabled || !node.processor) continue;

        ImageData processorInput = current;
        if (node.processor->requiresSecondImage() && hasSecondImage) {
            processorInput.setMetadata("_secondImage",
                QVariant::fromValue(secondImage.mat()));
        }

        QElapsedTimer stepTimer;
        stepTimer.start();

        ImageData next = node.processor->process(processorInput);

        QStringList warnings = current.metadata("pipeline_warnings").toStringList();
        const QVariant stepWarning = next.metadata("warning");
        if (stepWarning.isValid())
            warnings << stepWarning.toString();
        if (!warnings.isEmpty())
            next.setMetadata("pipeline_warnings", warnings);

        current = std::move(next);

        int elapsed = static_cast<int>(stepTimer.elapsed());
        emit processorTimeMs(node.processor->metadata().id, elapsed);

        if (current.isEmpty())
            break;
    }

    int totalMs = static_cast<int>(totalTimer.elapsed());
    emit processingTimeMs(totalMs);

    return current;
}
