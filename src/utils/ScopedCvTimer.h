#ifndef SCOPEDCVTIMER_H
#define SCOPEDCVTIMER_H

#include <opencv2/core.hpp>
#include <functional>
#include <QString>

/**
 * @brief RAII utility for measuring elapsed time using cv::getTickCount().
 *
 * Usage:
 *   ScopedCvTimer timer([](int ms) { qDebug() << "Took" << ms << "ms"; });
 *   // ... code to time ...
 *   // Timer automatically reports on destruction
 */
class ScopedCvTimer
{
public:
    using Callback = std::function<void(int ms)>;

    explicit ScopedCvTimer(const Callback& cb = nullptr)
        : m_callback(cb)
        , m_start(cv::getTickCount())
    {
    }

    ~ScopedCvTimer()
    {
        if (m_callback) {
            int64_t end = cv::getTickCount();
            double freq = cv::getTickFrequency();
            int ms = static_cast<int>((end - m_start) / freq * 1000.0);
            m_callback(ms);
        }
    }

    /// Get elapsed milliseconds so far (without stopping the timer)
    int elapsed() const
    {
        int64_t now = cv::getTickCount();
        double freq = cv::getTickFrequency();
        return static_cast<int>((now - m_start) / freq * 1000.0);
    }

    // Non-copyable
    ScopedCvTimer(const ScopedCvTimer&) = delete;
    ScopedCvTimer& operator=(const ScopedCvTimer&) = delete;

private:
    Callback m_callback;
    int64_t m_start;
};

#endif // SCOPEDCVTIMER_H
