#include "threadwrap.h"

ThreadWrap::ThreadWrap()
:m_pThread(nullptr),
m_bPause(false),
m_bStop(false),
m_state(ThreadState::Stopped)
{

}

ThreadWrap::~ThreadWrap()
{
    Stop();
}

void ThreadWrap::Start()
{
    if (m_pThread == nullptr)
    {
        m_pThread = std::make_shared<std::thread>(&ThreadWrap::Run,this);

        if (m_pThread)
        {
            m_bPause = false;
            m_bStop = false;
            m_state = ThreadState::Running;
        }
    }
}

void ThreadWrap::Pause()
{
    if (m_pThread)
    {
        if (m_state == ThreadState::Running)
        {
            m_bPause = true;
            m_bStop = false;
            m_state = ThreadState::Paused;
        }
    }
}

void ThreadWrap::Stop()
{
    if (m_pThread)
    {
        if (m_state == ThreadState::Running)
        {
            m_bStop = true;
            m_bPause = false;

            m_condition.notify_all();

            if (m_pThread->joinable())
            {
                m_pThread->join();
            }
            
            m_pThread.reset();

            if (m_pThread)
            {
                m_state = ThreadState::Stopped;
            }
        }
    }
}

void ThreadWrap::Resume()
{
    if (m_pThread)
    {
        if (m_state == ThreadState::Paused)
        {
            m_bPause = false;
            m_condition.notify_all();
            m_state = ThreadState::Running;
        }
    }
}

int ThreadWrap::State() const
{
    return (int)m_state;
}

void ThreadWrap::Run()
{
    while (!m_bStop)
    {
        try
        {
            CustomProcess();
        }
        catch(const std::exception& e)
        {
            break;
        }

        if (m_bPause)
        {
            std::unique_lock<std::mutex> locker(m_mutex);
            if (m_bPause)
            {
                m_condition.wait(locker);
            }
            locker.unlock();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    m_bPause = false;
    m_bStop = false;
}
