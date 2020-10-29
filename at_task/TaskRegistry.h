//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
#define ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H

#include <memory>
#include <list>
#include <algorithm>
#include <utility>

#include "UserTask.h"

class TaskRegistry {

public:
  template<typename T>
  using TaskPtrT = std::shared_ptr<T>;

  class TaskSingleton {

  public:
    explicit TaskSingleton(std::function<UserTask *()> factory) : factory_(std::move(factory)) {}
    TaskSingleton(std::string Name, std::function<UserTask *()> Factory)
        : name_(std::move(Name)), factory_(std::move(Factory)) {}

    void Load() {
      if (instance_) { /* already loaded */
        return;
      }
      instance_.reset(factory_());
    }
    void Reset() {
      instance_.reset();
    }
    [[nodiscard]] UserTask *Get() const {
      if (!instance_) {
        throw std::runtime_error("Task '" + GetName() + "' must be loaded before getting");
      }
      return instance_.get();
    }
    [[nodiscard]] bool IsLoaded() const { return bool(instance_); }
    [[nodiscard]] std::string GetName() const { return name_; }

  private:
    std::string name_;
    std::function<UserTask *()> factory_;

    std::unique_ptr<UserTask> instance_;
  };

  static TaskRegistry &getInstance();

  template<typename T>
  int RegisterTask(const char *name) {
    auto emplace_result = task_singletons_.emplace(
        std::string(name),
        TaskSingleton(std::string(name), DefaultTaskFactory < T > ));
    return emplace_result.second ? int(task_singletons_.size()) : -1;
  }

  std::vector<std::string> GetTaskNames();

  void EnableTasks(const std::vector<std::string> &enabled_task_names = {});

  void DisableTasks(const std::vector<std::string> &disable_task_names = {});

  void LoadEnabledTasks();

  void UnloadAllTasks();

  auto begin() {
    CheckLoaded();
    return loaded_tasks_.begin();
  }

  auto end() {
    CheckLoaded();
    return loaded_tasks_.end();
  }

  [[nodiscard]] UserTask *GetTaskInstance(const std::string& name) const {
    return task_singletons_.at(name).Get();
  }


private:
  TaskRegistry() = default;
  TaskRegistry(const TaskRegistry &) = default;
  TaskRegistry &operator=(TaskRegistry &) = default;

  template<typename T>
  static UserTask *DefaultTaskFactory() { return new T; }

  static bool TaskPriorityAscComparator(const UserTask *t1, const UserTask *t2) {
    return t1->GetPriority() < t2->GetPriority();
  }

  void CheckLoaded() const { if (!is_loaded) throw std::runtime_error("Tasks are not loaded yet"); }

  std::vector<std::string> enabled_task_names_;
  std::map<std::string, TaskSingleton> task_singletons_;

  bool is_loaded{false};
  std::vector<UserTask*> loaded_tasks_;

};

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
