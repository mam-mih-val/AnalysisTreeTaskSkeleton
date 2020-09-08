//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_TASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_TASK_H

#include "UserTask.h"
#include "TaskRegistry.h"

class UserTask;
class TaskRegistry;

#define TASK_DEF(TASK_CLASS, PRIORITY) \
private:                          \
public:                      \
  static TaskRegistry::TaskInfo<TASK_CLASS> TASK_INFO; \
  std::size_t GetPriority() const override { return PRIORITY; } \
  std::string GetName() const override { return std::string(#TASK_CLASS); } \

#define TASK_IMPL(TASK_CLASS) \
TaskRegistry::TaskInfo<TASK_CLASS> TASK_CLASS::TASK_INFO = TaskRegistry::getInstance().RegisterTask<TASK_CLASS>();

template<typename T>
auto GetTaskInfo() {
  return T::TASK_INFO;
}

template<typename T>
auto GetTaskPtr() {
  return GetTaskInfo<T>().ptr;
}

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASK_H
