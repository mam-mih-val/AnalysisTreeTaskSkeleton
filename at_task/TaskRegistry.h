//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
#define ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H

#include <memory>
#include <list>
#include <algorithm>

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
    TaskPtr task(new T);

    auto task_insert_position = std::upper_bound(std::begin(tasks_), std::end(tasks_),
                                                 task,
                                                 TaskRegistry::TaskPtrPriorityComparator);
    tasks_.emplace(task_insert_position, std::move(task));
    /* reindex tasks */
    size_t task_order_no = 0;
    std::for_each(std::begin(tasks_), std::end(tasks_), [&task_order_no] (TaskPtr& t) {
      t->SetOrderNo(task_order_no++);
    });
    return tasks_.size();
  }

  auto tasks_begin() { return std::begin(tasks_); }
  auto begin() { return std::begin(tasks_); }
  auto tasks_end() { return std::end(tasks_); }
  auto end() { return std::end(tasks_); }
  [[nodiscard]] auto tasks_cbegin() const { return std::cbegin(tasks_); }
  [[nodiscard]] auto cbegin() const { return std::cbegin(tasks_); }
  [[nodiscard]] auto tasks_cend() const { return std::cend(tasks_); }
  [[nodiscard]] auto cend() const { return std::cend(tasks_); }


private:
  TaskRegistry() = default;
  TaskRegistry(const TaskRegistry&) = default;
  TaskRegistry& operator=(TaskRegistry&) = default;

  static bool TaskPtrPriorityComparator(const TaskPtr& t1, const TaskPtr& t2) {
    return t1->GetPriority() < t2->GetPriority();
  }

  std::list<TaskPtr> tasks_;

};

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
