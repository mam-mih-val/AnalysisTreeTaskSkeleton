//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H

#include <boost/program_options.hpp>
#include <AnalysisTree/FillTask.hpp>

class TaskRegistry;

class UserTask : public AnalysisTree::FillTask {

public:
  virtual std::string GetName() const { return ""; }
  virtual size_t GetPriority() const { return 0l; }

  virtual boost::program_options::options_description GetBoostOptions() { return {}; };

  virtual void PreInit() {}
  virtual void PostFinish() {}

  size_t GetOrderNo() const {
    return order_no_;
  }

  bool IsEnabled() const {
    return is_enabled_;
  }

private:
  friend TaskRegistry;

  bool is_enabled_{true};
  std::size_t order_no_{0};
};



#endif //ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
