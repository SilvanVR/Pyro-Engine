#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "thread.hpp"

class ThreadPool
{
    std::vector<std::unique_ptr<Thread>> threads;

public:
    ThreadPool() {}
    ThreadPool(const uint32_t& count) { setThreadCount(count); }

    // Some Getter's
    uint32_t    numThreads(){ return static_cast<uint32_t>(threads.size()); }
    Thread&     getThread(const uint32_t& index){ return *threads[index]; }
    Thread&     operator[](unsigned int index){ return *threads[index]; }

    // Sets the number of threads to be allocated in this pool. Destroy's all old threads.
    void setThreadCount(const uint32_t& count)
    {
        threads.clear();
        for (unsigned int i = 0; i < count; i++)
            threads.push_back(std::make_unique<Thread>());
    }

    // Wait until all threads have finished their work items
    void wait()
    {
        for (auto &thread : threads)
            thread->waitIdle();
    }

    // Find and return a thread with the least work (or the first one without a job)
    Thread& getThreadLeastWork()
    {
        Thread* leastWorkThread = &(*threads[0]);

        for (uint32_t i = 1; i < numThreads(); i++)
        {
            if(!leastWorkThread->hasJob()) // Return immediately if a thread without a job has be found
                return *leastWorkThread;

            if((*threads[i]).numJobs() < leastWorkThread->numJobs())
                leastWorkThread = &(*threads[i]);
        }

        return *leastWorkThread;
    }
};

#endif // !THREAD_POOL_H_
