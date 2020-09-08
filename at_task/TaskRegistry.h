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
  template<typename T>
  using TaskPtrT = std::shared_ptr<T>;

  typedef TaskPtrT<UserTask> UserTaskPtr;

  template <typename T>
  struct TaskInfo {
    TaskPtrT<T> ptr;
    size_t task_no{0};
  };

  static TaskRegistry& getInstance();

  /**
   * @brief Adds instance of task of type T into the registry
   * @tparam T
   * @return task id
   */
  template<typename T>
  TaskInfo<T> RegisterTask() noexcept {
    TaskPtrT<T> task(new T);

    auto task_insert_position = std::upper_bound(std::begin(tasks_), std::end(tasks_),
                                                 task,
                                                 TaskRegistry::TaskPtrPriorityComparator);
    tasks_.emplace(task_insert_position, task);
    /* reindex tasks */
    size_t task_order_no = 0;
    std::for_each(std::begin(tasks_), std::end(tasks_), [&task_order_no] (UserTaskPtr& t) {
      t->order_no_ = task_order_no;
      t->is_enabled_ = true;
      ++task_order_no;
    });

    TaskInfo<T> info;
    info.ptr = task;
    info.task_no = tasks_.size();
    return info;
  }

  void EnableTasks(const std::vector<std::string> & enabled_task_names = {});

  void DisableTasks(const std::vector<std::string> &disable_task_names = {});

  auto begin() { return std::begin(tasks_); }

  auto end() { return std::end(tasks_); }
  [[nodiscard]] auto cbegin() const { return std::cbegin(tasks_); }
  [[nodiscard]] auto cend() const { return std::cend(tasks_); }
  auto tasks_begin() { return std::begin(tasks_); }

  auto tasks_end() { return std::end(tasks_); }
  [[nodiscard]] auto tasks_cbegin() const { return std::cbegin(tasks_); }
  [[nodiscard]] auto tasks_cend() const { return std::cend(tasks_); }

  void EnabledTasks(std::vector<UserTaskPtr> &enabled_tasks) const {
    enabled_tasks.clear();
    std::copy_if(cbegin(), cend(), std::back_inserter(enabled_tasks),
                 [] (const UserTaskPtr &t) { return t->IsEnabled(); });

  }

private:
  TaskRegistry() = default;
  TaskRegistry(const TaskRegistry&) = default;
  TaskRegistry& operator=(TaskRegistry&) = default;

  static bool TaskPtrPriorityComparator(const UserTaskPtr& t1, const UserTaskPtr& t2) {
    return t1->GetPriority() < t2->GetPriority();
  }

  std::list<UserTaskPtr> tasks_;

};

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
