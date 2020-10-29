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

  static TaskRegistry &Instance();

  template<typename T>
  int RegisterTask(const char *name) {
    auto emplace_result = task_singletons_.emplace(
        std::string(name),
        TaskSingleton(std::string(name), DefaultTaskFactory < T > ));
    return emplace_result.second ? int(task_singletons_.size()) : -1;
  }

  std::vector<std::string> GetTaskNames();

  void UnloadAllTasks();

  void LoadAll();

  void Load(const std::string& name);

  void Unload(const std::string& name);

  [[nodiscard]] UserTask *TaskInstance(const std::string& name) const {
    return task_singletons_.at(name).Get();
  }


private:
  TaskRegistry() = default;
  TaskRegistry(const TaskRegistry &) = default;
  TaskRegistry(TaskRegistry&&) = default;
  TaskRegistry &operator=(TaskRegistry &) = default;

  template<typename T>
  static UserTask *DefaultTaskFactory() { return new T; }



  std::map<std::string, TaskSingleton> task_singletons_;

};

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASKREGISTRY_H
