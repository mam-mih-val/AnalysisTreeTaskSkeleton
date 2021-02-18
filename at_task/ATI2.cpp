//
// Created by eugene on 17/02/2021.
//

#include <iostream>

#include <TTree.h>

#include "ATI2.hpp"

using namespace ATI2;

void BranchChannel::Print(std::ostream &os) const {
  os << "Branch " << branch->config.GetName() << " channel #" << i_channel << std::endl;
}

BranchLoopIter BranchLoop::begin() const { return branch->ChannelsBegin(); }
BranchLoopIter BranchLoop::end() const { return branch->ChannelsEnd(); }

void Branch::ConnectOutputTree(TTree *tree) {
  is_connected_to_output = ApplyT([this, tree](auto entity) -> bool {
    if (!tree)
      return false;
    auto new_tree_branch_ptr = tree->Branch(config.GetName().c_str(),
                                            std::add_pointer_t<decltype(entity)>(&this->data));
    return (bool) new_tree_branch_ptr;
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
    if constexpr (std::is_same_v<AnalysisTree::EventHeader,
                                 std::remove_const_t<std::remove_pointer_t<decltype(entity_ptr)>>>) {
      throw std::runtime_error("Size is not implemented for EventHeader variable");
    } else  {
      return entity_ptr->GetNumberOfChannels();
    }
  });
}

BranchChannel::BranchChannel(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {
  UpdatePointer();
}

BranchChannel Branch::operator[](size_t i_channel) { return BranchChannel(this, i_channel); }

void BranchChannel::UpdatePointer() {
  if (i_channel < branch->size()) {
    data_ptr = branch->ApplyT([this](auto entity_ptr) -> void * {
      if constexpr (std::is_same_v<AnalysisTree::EventHeader,
                                   std::remove_const_t<std::remove_pointer_t<decltype(entity_ptr)>>>) {
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
