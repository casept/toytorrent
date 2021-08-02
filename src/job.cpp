#include "job.hpp"

#include <memory>
#include <utility>

namespace tt::job {
JobQueue::JobQueue() noexcept : m_jobs(std::deque<std::unique_ptr<IJob>>()) {}

void JobQueue::enqueue(std::unique_ptr<IJob> j) { m_jobs.push_back(std::move(j)); }

std::optional<std::unique_ptr<IJob>> JobQueue::deque() { return {}; }

void JobQueue::process() {}
}  // namespace tt::job