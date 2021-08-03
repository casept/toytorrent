#pragma once

//! This module implements a job system, where work units can be queued up and processed.
//! It's not thread-safe (yet).

#include <deque>
#include <memory>
#include <optional>
#include <vector>

namespace tt::job {
/// The key type of this module.
///
/// Jobs can be submitted to job queues for processing,
/// and they can enqueue new jobs.
///
/// This abstraction is used to schedule all tasks in the program.
class IJob {
   public:
    /// Drive this job to completion.
    /// Currently, this will tie up an entire thread.
    /// Resumption may be implemented later.
    virtual void process() = 0;
    virtual ~IJob() = default;
};

/// Where jobs to be processed go.
class JobQueue {
   public:
    JobQueue() noexcept;
    JobQueue(JobQueue&) = delete;
    JobQueue operator=(JobQueue&) = delete;
    JobQueue(JobQueue&&) = delete;
    JobQueue& operator=(JobQueue&&) = delete;

    /// Enqueue a new job
    void enqueue(std::unique_ptr<IJob> j);
    /// Enqueue a bunch of new jobs
    void enqueue_vec(std::vector<std::unique_ptr<IJob>> j);
    /// Process jobs until all done
    void process();

   private:
    std::deque<std::unique_ptr<IJob>> m_jobs;

    /// Take a job to work on it
    std::optional<std::unique_ptr<IJob>> deque();
};

}  // namespace tt::job