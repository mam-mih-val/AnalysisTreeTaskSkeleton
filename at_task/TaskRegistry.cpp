//
// Created by eugene on 31/08/2020.
//

#include "TaskRegistry.h"

TaskRegistry &TaskRegistry::Instance() {
  static TaskRegistry registry;
  return registry;
}

void TaskRegistry::EnableTasks(const std::vector<std::string> &enabled_task_names) {
  /* if empty - enable all tasks */
  if (enabled_task_names.empty())
    enabled_task_names_ = GetTaskNames();
  else {
    enabled_task_names_.clear();
    for (auto &task_name : enabled_task_names) {
      auto &task = task_singletons_.at(task_name);
      enabled_task_names_.emplace_back(task_name);
    }
  }
}

void TaskRegistry::DisableTasks(const std::vector<std::string> &disable_task_names) {
  /* if empty - enable all tasks */
  auto tasks_to_enable = GetTaskNames();
  for (auto &task_to_disable : disable_task_names) {
    auto task_name_pos = std::find(tasks_to_enable.begin(), tasks_to_enable.end(), task_to_disable);
    if (task_name_pos != tasks_to_enable.end()) {
      tasks_to_enable.erase(task_name_pos);
    } else {
      // TODO warn user
    }
  }
  enabled_task_names_ = tasks_to_enable;

}
void TaskRegistry::LoadEnabledTasks() {
  UnloadAllTasks();
  loaded_tasks_.reserve(enabled_task_names_.size());
  for (auto &task_name : enabled_task_names_) {
    task_singletons_.at(task_name).Load();
    auto ptr = task_singletons_.at(task_name).Get();
    loaded_tasks_.emplace_back(ptr);
  }
  std::sort(loaded_tasks_.begin(), loaded_tasks_.end(), TaskPriorityAscComparator);
  is_loaded = true;
}
void TaskRegistry::UnloadAllTasks() {
  loaded_tasks_.clear();
  for (auto &t : task_singletons_) {
    t.second.Reset();
  }
  is_loaded = false;
}
std::vector<std::string> TaskRegistry::GetTaskNames() {
  std::vector<std::string> result;
  std::transform(task_singletons_.begin(), task_singletons_.end(),
                 std::back_inserter(result), [] (auto &ele) { return ele.first; });
  return result;
}
