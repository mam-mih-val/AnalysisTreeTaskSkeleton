//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H

#include <boost/program_options.hpp>
#include <AnalysisTree/FillTask.hpp>
#include <AnalysisTree/EventHeader.hpp>
#include <AnalysisTree/Detector.hpp>
#include <TTree.h>

#include "ATI2.hpp"

class TaskRegistry;

class UserTask {

 public:
  virtual ~UserTask() = default;
  virtual std::string GetName() const { return ""; }
  virtual size_t GetPriority() const { return 0l; }
  virtual AnalysisTree::FillTask *FillTaskPtr() = 0;

  virtual boost::program_options::options_description GetBoostOptions() { return {}; }
  virtual void ProcessBoostVM(const boost::program_options::variables_map &vm) { (void) vm; }

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
  typedef short VariableIndex;
  /**
   * @brief This function simplifies routine of getting the variable Id from
   * <b>input</b> config during the initialization of the program (usually Init()).
   * Effectively replaces config_->GetBranchConfig("BranchName")->GetFieldId("FieldName")
   * Checks existence of the requested branch
   * @param variable_name - name of the variable in format "BranchName/FieldName"
   * @return variable id
   */
  VariableIndex VarId(const std::string &variable_name) const;
  VariableIndex VarId(const std::string &branch_name, const std::string &field_name) const;

  /**
   * @brief This function creates new branch in the out_config_
   * @param branch_name
   * @param detector_type
   * @return
   */
  AnalysisTree::BranchConfig &NewBranch(const std::string &branch_name, AnalysisTree::DetType detector_type);

  /**
   * @brief This function initialize new variable in the <b>output</b> config.
   * @tparam T - type of the variable
   * @param variable_name - name of the variable in format "BranchName/FieldName"
   * @return variable Id
   */
  template<typename T>
  short NewVar(const std::string &variable_name) {
    auto &&[branch_name, field_name] = ParseVarName(variable_name);

    auto &branch = out_config_->GetBranchConfig(branch_name);
    branch.template AddField<T>(field_name);
    return branch.GetFieldId(field_name);
  }

 public:

  void ReadMap(std::map<std::string, void *> &map);
  ATI2::Branch *GetInBranch(const std::string &name) const { return branches_in_.at(name).get(); }
  ATI2::Branch *GetOutBranch(const std::string &name) const { return branches_out_.at(name).get(); }
  ATI2::Variable GetVar(const std::string &name) const;

  std::map<std::string, std::unique_ptr<ATI2::Branch>> branches_in_;
  std::map<std::string, std::unique_ptr<ATI2::Branch>> branches_out_;

  static std::pair<std::string, std::string> ParseVarName(const std::string &variable_name);

};


#endif //ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
