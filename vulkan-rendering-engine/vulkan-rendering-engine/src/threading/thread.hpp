#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

class Thread
{
    std::thread                         worker;
    std::queue<std::function<void()>>   jobQueue;
    std::condition_variable             cv;
    std::mutex                          mutex;
    bool                                running;

    void queueLoop()
    {
        while (running)
        {
             {
                 // Condition Variable need a unique_lock, which will be unlocked when cv.wait() is called
                 std::unique_lock<std::mutex> lock(mutex);

                 // Wait until a job has arrived if queue is empty or when the thread should be terminated
                 cv.wait(lock, [&]() -> bool {

                     // Notify a possible waiting thread for this thread to be finished.
                     if (jobQueue.empty())
                         cv.notify_one(); // Calling notify here guarantees that this thread is waiting when notify the other waiting thread

                     return !jobQueue.empty() || !running; 
                 });
             } // Necessary?
            
             if (running)
             {
                 // Sync access to queue
                 std::lock_guard<std::mutex> lock(mutex);

                 // Execute first job in the queue
                 jobQueue.front()();
             
                 // Remove the executed job 
                 jobQueue.pop();

             } // destroys lock_guard and unlock the mutex
        }
    }


public:
    Thread()
        : running(true)
    {
        // Create a new thread
        worker = std::thread(&Thread::queueLoop, this);
    }

    ~Thread()
    {
        if (worker.joinable())
        {
            // Wait until all jobs has finished
            waitIdle();

            // Set boolean to false, which terminates the while-loop for the thread
            running = false;

            // Notify the waiting worker to terminate him
            cv.notify_one();

            // Wait until worker has been terminated
            worker.join();
        }
    }

    // Add a job to this threads queue
    void addJob(std::function<void()> job)
    {
        // Sync access to queue
        std::lock_guard<std::mutex> lock(mutex);

        // Add job to the queue
        jobQueue.push(std::move(job));

        // Notify possible waiting worker
        cv.notify_one();
    }

    // Wait until all jobs in the queue has finished
    void waitIdle()
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() -> bool { return jobQueue.empty(); });
    }

    // Return number of active jobs
    unsigned int numJobs()
    {
        return static_cast<unsigned int>(jobQueue.size());
    }

    // Return true if this thread has a job
    bool hasJob()
    {
        return jobQueue.size() != 0;
    }
};



#endif // !THREAD_H_
