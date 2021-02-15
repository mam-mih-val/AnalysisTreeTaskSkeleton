//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H

#include <boost/program_options.hpp>
#include <AnalysisTree/FillTask.hpp>

class TaskRegistry;

class UserTask  {

public:
  virtual ~UserTask() = default;
  virtual std::string GetName() const { return ""; }
  virtual size_t GetPriority() const { return 0l; }
  virtual AnalysisTree::FillTask* FillTaskPtr() = 0;

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

class UserFillTask : public UserTask, public AnalysisTree::FillTask {
public:
  virtual ~UserFillTask() = default;
  AnalysisTree::FillTask *FillTaskPtr() final {
    return this;
  }

 protected:
  auto VarId(const std::string &variable_name) const;

};



#endif //ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
