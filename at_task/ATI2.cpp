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
  ApplyT([this, tree](auto entity) {
    tree->Branch(config.GetName().c_str(),
                 std::add_pointer_t<decltype(entity)>(&this->data));
  });
}

void Branch::InitDataPtr() {
  ApplyT([this](auto entity) {
    if (entity)
      throw std::runtime_error("Data ptr is already initialized");
    this->data = new typename std::remove_pointer<decltype(entity)>::type;
  });
}

struct BranchSizeImpl {
  explicit BranchSizeImpl(size_t &result) : result(result) {}
  size_t &result;

  template<typename Entity>
  void Eval(const Entity *e) { result = e->GetNumberOfChannels(); }
  void Eval(const AnalysisTree::EventHeader &e) { throw std::runtime_error("Not implemented"); }

  template<typename Entity>
  void operator()(const Entity *e) { Eval(e); }
};

size_t ATI2::Branch::size() const {
  size_t result;
  ApplyT(BranchSizeImpl(result));
  return result;
}
