//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
#define ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H

#include <memory>
#include <list>

#include "UserTask.h"

class TaskRegistry {

public:
  typedef std::shared_ptr<UserTask> TaskPtr;

  static TaskRegistry& getInstance();

  /**
   * @brief Adds instance of task of type T into the registry
   * @tparam T
   * @return task id
   */
  template<typename T>
  std::size_t RegisterTask() noexcept {
    tasks_.emplace_back(new T);
    return tasks_.size() - 1;
  }

  auto tasks_begin() { return std::begin(tasks_); }
  auto tasks_end() { return std::end(tasks_); }
  [[nodiscard]] auto tasks_cbegin() const { return std::cbegin(tasks_); }
  [[nodiscard]] auto tasks_cend() const { return std::cend(tasks_); }

private:
  TaskRegistry() = default;
  TaskRegistry(const TaskRegistry&) = default;
  TaskRegistry& operator=(TaskRegistry&) = default;

  std::list<TaskPtr> tasks_;

};

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
