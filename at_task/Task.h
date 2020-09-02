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
  static std::size_t TASK_NO;     \
public:                      \
  std::size_t GetPriority() const override { return PRIORITY; } \
  std::string GetName() const override { return std::string(#TASK_CLASS); } \

#define TASK_IMPL(TASK_CLASS) \
std::size_t TASK_CLASS::TASK_NO = TaskRegistry::getInstance().RegisterTask<TASK_CLASS>();

#endif //ANALYSISTREESKELETON_TASK_MAIN_TASK_H
