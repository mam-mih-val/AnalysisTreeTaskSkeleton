//
// Created by eugene on 31/08/2020.
//

#include "UserTask.h"

#include <regex>

short UserFillTask::VarId(const std::string &variable_name) const {
  auto &&[branch_name, field_name] = ParseVarName(variable_name);

  for (const auto &branch_config : config_->GetBranchConfigs()) {
    if (branch_config.GetName() == branch_name) {
      return branch_config.GetFieldId(field_name);
    }
  }
  throw std::runtime_error("Branch with name " + branch_name + " is not found");
}
AnalysisTree::BranchConfig &UserFillTask::NewBranch(const std::string &branch_name,
                                                    AnalysisTree::DetType detector_type) {
  out_config_->AddBranchConfig(AnalysisTree::BranchConfig(branch_name, detector_type));
  return out_config_->GetBranchConfig(branch_name);
}

std::pair<std::string, std::string> UserFillTask::ParseVarName(const std::string &variable_name) {
  const std::regex re_vname("^(\\w+)/(\\w+)$");

  std::smatch match_results;
  if (std::regex_search(variable_name, match_results, re_vname)) {
    return {match_results.str(1), match_results.str(2)};
  }
  throw std::runtime_error("Invalid format for variable name");
}
