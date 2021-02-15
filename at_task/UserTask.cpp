//
// Created by eugene on 31/08/2020.
//

#include "UserTask.h"

#include <regex>

auto UserFillTask::VarId(const std::string &variable_name) const {
  const std::regex re_vname("^(\\w+)/(\\w+)$");
  std::smatch match_results;
  if (std::regex_search(variable_name, match_results, re_vname)) {
    auto branch_name = match_results.str(1);
    auto field_name = match_results.str(2);

    for (const auto &branch_config : config_->GetBranchConfigs()) {
      if (branch_config.GetName() == branch_name) {
        return branch_config.GetFieldId(field_name);
      }
    }

    throw std::runtime_error("Branch with name " + branch_name + " is not found");
  }

  throw std::runtime_error("Invalid format for variable name");
}
