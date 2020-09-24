//
// Created by eugene on 31/08/2020.
//

#include "TaskRegistry.h"

TaskRegistry &TaskRegistry::getInstance() {
  static TaskRegistry registry;
  return registry;
}
void TaskRegistry::EnableTasks(const std::vector<std::string> &enabled_task_names) {
  for (auto &t : tasks_) {
    /* if empty - enable all */
    if (enabled_task_names.empty()) t->is_enabled_ = true;
    else {
      std::string task_name = t->GetName();
      t->is_enabled_ = std::any_of(enabled_task_names.begin(), enabled_task_names.end(),
                                   [task_name] (auto &n) { return task_name == n; });
    }
  }
}

void TaskRegistry::DisableTasks(const std::vector<std::string> &disable_task_names) {
  for (auto &t : tasks_) {
    if (disable_task_names.empty()) t->is_enabled_ = true;
    else {
      std::string task_name = t->GetName();
      t->is_enabled_ = !std::any_of(disable_task_names.begin(), disable_task_names.end(),
                                    [task_name] (auto &n) { return task_name == n; });
    }
  }

}
void TaskRegistry::EnabledTasks(std::vector<UserTaskPtr> &enabled_tasks) const {
  enabled_tasks.clear();
  std::copy_if(cbegin(), cend(), std::back_inserter(enabled_tasks),
               [] (const UserTaskPtr &t) { return t->IsEnabled(); });

}
