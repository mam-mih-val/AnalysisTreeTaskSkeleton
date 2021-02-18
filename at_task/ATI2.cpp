//
// Created by eugene on 17/02/2021.
//

#include <iostream>

#include <TTree.h>

#include "ATI2.hpp"

using namespace ATI2;

namespace Impl {

template<typename Entity>
inline
double ReadValue(const Variable &v, const Entity& e) {
  using AnalysisTree::Types;

  if (v.field_type == Types::kFloat) {
    return e.template GetField<float>(v.id);
  } else if (v.field_type == Types::kInteger) {
    return e.template GetField<int>(v.id);
  } else if (v.field_type == Types::kBool) {
    return e.template GetField<bool>(v.id);
  }
  /* unreachable */
  assert(false);
}

template<typename Entity>
inline
void SetValue(const Variable& v, Entity&e, double value) {
  using AnalysisTree::Types;

  if (v.field_type == Types::kFloat) {
    e.template SetField<float>(value, v.id);
    return;
  } else if (v.field_type == Types::kInteger) {
    e.template SetField<int>(value, v.id);
    return;
  } else if (v.field_type == Types::kBool) {
    e.template SetField<bool>(value, v.id);
    return;
  }
  /* unreachable */
  assert(false);
}

} // namespace Impl


void BranchChannel::Print(std::ostream &os) const {
  os << "Branch " << branch->config.GetName() << " channel #" << i_channel << std::endl;
}

BranchLoopIter BranchLoop::begin() const { return branch->ChannelsBegin(); }
BranchLoopIter BranchLoop::end() const { return branch->ChannelsEnd(); }

Variable Branch::GetVar(const std::string &field_name) {
  ATI2::Variable v;
  v.parent_branch = this;
  v.id = v.parent_branch->config.GetFieldId(field_name);
  v.name = this->config.GetName() + "/" + field_name;
  v.field_name = field_name;
  v.field_type = config.GetFieldType(field_name);

  if (v.id == AnalysisTree::UndefValueShort)
    throw std::runtime_error("Field of name '" + v.name + "' not found");
  return v;
}
void Branch::ConnectOutputTree(TTree *tree) {
  is_connected_to_output = ApplyT([this, tree](auto entity) -> bool {
    if (!tree)
      return false;
    auto new_tree_branch_ptr = tree->Branch(config.GetName().c_str(),
                                            std::add_pointer_t<decltype(entity)>(&this->data));
    return bool(new_tree_branch_ptr);
  });
}

void Branch::InitDataPtr() {
  ApplyT([this](auto entity) {
    if (entity)
      throw std::runtime_error("Data ptr is already initialized");
    this->data = new typename std::remove_pointer<decltype(entity)>::type;
  });
}

size_t ATI2::Branch::size() const {
  return ApplyT([](auto entity_ptr) -> size_t {
    if constexpr (is_event_header_v<decltype(entity_ptr)>) {
      throw std::runtime_error("Size is not implemented for EventHeader variable");
    } else {
      return entity_ptr->GetNumberOfChannels();
    }
  });
}

BranchChannel::BranchChannel(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {
  UpdatePointer();
}

BranchChannel Branch::operator[](size_t i_channel) { return BranchChannel(this, i_channel); }
BranchChannel Branch::NewChannel() {
  CheckMutable();
  ApplyT([this](auto entity_ptr) {
    if constexpr (is_event_header_v<decltype(entity_ptr)>) {
      throw std::runtime_error("Not applicable for EventHeader");
    } else {
      auto channel = entity_ptr->AddChannel();
      channel->Init(this->config);
      Freeze();
    }
  });
  return operator[](size() - 1);
}
void Branch::CheckFrozen() const {
  if (is_frozen)
    throw std::runtime_error("Branch is frozen");
}
void Branch::CheckMutable() const {
  if (!is_mutable)
    throw std::runtime_error("Branch is not mutable");
}

double Branch::Value(const Variable &v) const {
  return ApplyT([v] (auto entity_ptr) -> double {
    if constexpr (is_event_header_v<decltype(entity_ptr)>) {
      return Impl::ReadValue(v, *entity_ptr);
    } else {
      throw std::runtime_error("Not implemented for Detector<...>");
    }
  });
}

void Branch::Set(const Variable &v, double value) {
  CheckMutable();
  ApplyT([&v, value] (auto entity_ptr) -> void {
    if constexpr (is_event_header_v< decltype(entity_ptr)>) {
      Impl::SetValue(v, *entity_ptr, value);
    } else {
      throw std::runtime_error("Not implemented for Detector<...>");
    }
  });
}

double ATI2::BranchChannel::Value(const ATI2::Variable &v) const {
  return ApplyT([this, &v](auto entity_ptr) -> double {
    return Impl::ReadValue(v, *entity_ptr);
  });
}
void BranchChannel::Set(const Variable &v, double value) {
  if (branch != v.parent_branch) {
    throw std::runtime_error("Inconsistent branches");
  }
  branch->CheckMutable();
  ApplyT([&v, value] (auto entity_ptr) -> void {
    Impl::SetValue(v, *entity_ptr, value);
  });
}

void BranchChannel::UpdateChannel(size_t new_channel) {
  i_channel = new_channel;
  UpdatePointer();
}
void BranchChannel::UpdatePointer() {
  if (i_channel < branch->size()) {
    data_ptr = branch->ApplyT([this](auto entity_ptr) -> void * {
      if constexpr (Branch::is_event_header_v<decltype(entity_ptr)>) {
        throw std::runtime_error("Getting channel of the EventHeader is not implemented");
      } else {
        return &entity_ptr->GetChannel(this->i_channel);
      }
    });
  } else {
    data_ptr = nullptr;
  }
}
BranchLoopIter &BranchLoopIter::operator++() {
  i_channel++;
  current_channel->UpdateChannel(i_channel);
  return *this;
}

void Variable::Print(std::ostream &os) const {
  os << name << "(id = " << id << ")" << std::endl;
}
