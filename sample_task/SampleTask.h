//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
#define ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H

#include "Task.h"


class FooTask : public UserTask {
public:

  void Init(std::map<std::string, void *> &Map) override {

  }

  void Exec() override {

  }
  void Finish() override {

  }

  TASK_DEF(FooTask)
};


#endif //ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
