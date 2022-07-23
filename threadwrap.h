#ifndef THREAD_WRAP_H
#define THREAD_WRAP_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadWrap
{
    enum class ThreadState
    {
        Running,
        Paused,
        Stopped
    };

public:
    ThreadWrap();
    virtual ~ThreadWrap();

    void Start();
    void Pause();
    void Stop();
    void Resume();

    int State() const;

private:
    void Run();

protected:
    virtual void CustomProcess() {}

private:
    ThreadState m_state;

    std::mutex m_mutex;
    std::shared_ptr<std::thread> m_pThread;
    std::condition_variable m_condition;
    std::atomic_bool m_bPause;
    std::atomic_bool m_bStop;
};

#endif