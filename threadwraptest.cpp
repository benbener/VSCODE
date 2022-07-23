#include <iostream>
#include "threadwrap.h"

#include <list>
#include <string>

class MyThread : public ThreadWrap
{
public:
    MyThread()
    {
    }

    virtual ~MyThread()
    {
    }

    void CustomProcess() override
    {
        std::cout << "MyThread customProcess Run" << std::endl;
    }
};

std::mutex gMutex;
std::condition_variable gCondition;
std::list<std::string> gProducts;

class Productor : public ThreadWrap
{
public:
    Productor() {}
    Productor(std::string thread_name) : m_thread_name(thread_name) {}
    ~Productor() {}

    void CustomProcess() override
    {
        std::unique_lock<std::mutex> locker(gMutex);
        gProducts.push_back(m_thread_name + "_" + std::to_string(m_product_id++));
        gCondition.notify_one();
        std::cout << m_thread_name << " product " << m_product_id << std::endl;
        locker.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

private:
    std::string m_thread_name;
    int m_product_id;
};

class Customer : public ThreadWrap
{
public:
    Customer() {}
    Customer(std::string thread_name) : m_thread_name(thread_name) {}
    ~Customer() {}

    void CustomProcess() override
    {
        std::unique_lock<std::mutex> locker(gMutex);
        if (gProducts.empty())
        {
            std::cout << m_thread_name << " custome empty ..." << std::endl;
            gCondition.wait(locker);
        }
        else
        {
            std::string product_id = gProducts.front();
            gProducts.pop_front();
            std::cout << m_thread_name << " custome " << product_id << std::endl;
            locker.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }

private:
    std::string m_thread_name;
};

int main()
{
    // MyThread thd;
    // std::cout << "Thread state:" << thd.State() << std::endl;

    // thd.Start();
    // std::cout << "Thread state:" << thd.State() << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // thd.Pause();
    // std::cout << "Thread state:" << thd.State() << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // thd.Resume();
    // std::cout << "Thread state:" << thd.State() << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // thd.Stop();
    // std::cout << "Thread state:" << thd.State() << std::endl;

    Productor pdctor("Productor1");
    pdctor.Start();

    Productor pdctor2("Productor2");
    pdctor2.Start();

    Customer ctmer("Customer1");
    ctmer.Start();

    Customer ctmer2("Customer2");
    ctmer2.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    pdctor.Pause();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    pdctor2.Pause();

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    pdctor2.Resume();

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    pdctor.Resume();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    pdctor.Stop();
    pdctor2.Stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    ctmer.Stop();
    ctmer2.Stop();

    getchar();

    return 0;
}