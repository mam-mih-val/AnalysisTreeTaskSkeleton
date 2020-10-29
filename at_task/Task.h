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
  static int REGISTRY_FLAG; \
  static const char* Name() { return #TASK_CLASS; }    \
  static std::size_t Priority() { return PRIORITY; }  \
  std::size_t GetPriority() const override { return TASK_CLASS::Priority(); } \
  std::string GetName() const override { return TASK_CLASS::Name(); }

#define TASK_IMPL(TASK_CLASS) \
TaskRegistry::TaskInfo<TASK_CLASS> TASK_CLASS::TASK_INFO = TaskRegistry::getInstance().RegisterTask<TASK_CLASS>();\
int TASK_CLASS::REGISTRY_FLAG = TaskRegistry::getInstance().RegisterTask<TASK_CLASS>(TASK_CLASS::Name());

template<typename T>
auto GetTaskInfo() {
  return T::TASK_INFO;
}

template<typename T>
auto GetTaskPtr() {
  return GetTaskInfo<T>().ptr;
}

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASK_H
