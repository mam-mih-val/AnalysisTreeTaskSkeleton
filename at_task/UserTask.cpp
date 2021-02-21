//
// Created by eugene on 31/08/2020.
//

#include "UserTask.h"

#include <regex>

#include <AnalysisTree/EventHeader.hpp>
#include <AnalysisTree/Detector.hpp>

using namespace ATI2;

UserFillTask::VariableIndex UserFillTask::VarId(const std::string &variable_name) const {
  auto &&[branch_name, field_name] = ParseVarName(variable_name);
  return VarId(branch_name, field_name);
}
UserFillTask::VariableIndex UserFillTask::VarId(const std::string &branch_name, const std::string &field_name) const {
  const auto& branch = config_->GetBranchConfig(branch_name);
  return branch.GetFieldId(field_name);
}

AnalysisTree::BranchConfig &UserFillTask::NewBranch(const std::string &branch_name,
                                                    AnalysisTree::DetType detector_type) {
  if (! (out_config_ && out_tree_)) {
    throw std::runtime_error("Output is not configured");
  }

  AnalysisTree::BranchConfig branch_config(branch_name, detector_type);

  if (branches_out_.find(branch_name) != branches_out_.end())
    throw std::runtime_error("Branch of that name already exists");

  auto branch_ptr = std::make_unique<Branch>(branch_config);
  branch_ptr->parent_config = out_config_;
  branch_ptr->ConnectOutputTree(out_tree_);
  branch_ptr->SetMutable(true);
  branches_out_.emplace(branch_name, std::move(branch_ptr));

  out_config_->AddBranchConfig(branch_config);
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

void UserFillTask::ReadMap(std::map<std::string, void *>& map) {
  for (auto &map_ele : map) {
    auto branch_name = map_ele.first;
    auto data_ptr = map_ele.second;

    AnalysisTree::BranchConfig config;
    try {
      config = config_->GetBranchConfig(branch_name);
    } catch (std::runtime_error &e) {
      // TODO warning
      continue;
    }

    auto branch = std::make_unique<Branch>(config, data_ptr);
    branch->parent_config = config_;
    branch->is_connected_to_input = true;
    branch->SetMutable(false);
    branch->Freeze();
    branches_in_.emplace(branch_name, std::move(branch));
  }
  std::cout << "Read map done" << std::endl;

}

ATI2::Variable UserFillTask::GetVar(const std::string &name) const {
  auto &&[br_name, f_name] = ParseVarName(name);
  return GetInBranch(br_name)->GetFieldVar(f_name);
}
