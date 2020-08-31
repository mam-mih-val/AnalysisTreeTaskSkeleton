//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_TASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_TASK_H

#include "UserTask.h"
#include "TaskRegistry.h"

class UserTask;
class TaskRegistry;

#define TASK_DEF(TASK_CLASS) \
private:                          \
  static std::size_t TASK_ID;     \
public:                      \
  std::size_t GetTaskId() const override { return TASK_ID; } \
  std::string GetName() const override { return std::string(#TASK_CLASS); } \

#define TASK_IMPL(TASK_CLASS) \
std::size_t TASK_CLASS::TASK_ID = TaskRegistry::getInstance().RegisterTask<TASK_CLASS>();

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASK_H
