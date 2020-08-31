//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H

#include <boost/program_options.hpp>
#include <infra/FillTask.hpp>

class UserTask : public AnalysisTree::FillTask {

public:
  virtual std::string GetName() const { return ""; };
  virtual std::size_t GetTaskId() const { return 0; };

  virtual boost::program_options::options_description GetBoostOptions() { return {}; };

  virtual void PreInit() {};

private:
};



#endif //ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
