//
// Created by eugene on 17/02/2021.
//

#ifndef ATTASKSKELETON_AT_TASK_ATI2_HPP_
#define ATTASKSKELETON_AT_TASK_ATI2_HPP_

#include <AnalysisTree/BranchConfig.hpp>
#include <AnalysisTree/Configuration.hpp>
#include <AnalysisTree/Detector.hpp>
#include <AnalysisTree/EventHeader.hpp>

namespace ATI2 {

struct Variable;
struct Branch;
struct BranchLoop;

struct BranchChannel {
  BranchChannel(Branch *branch, size_t i_channel);

  /* Getting value */
  double Value(const Variable &v) const;
  void *Data() { return data_ptr; }
  const void *Data() const { return data_ptr; }

  void Print(std::ostream &os = std::cout) const;

  template<typename Functor>
  auto ApplyT(Functor &&functor);
  template<typename Functor>
  auto ApplyT(Functor &&functor) const;
  void UpdatePointer();
  void UpdateChannel(size_t new_channel);

  void *data_ptr{nullptr};
  Branch *branch;
  size_t i_channel;
};

struct BranchLoopIter {
  BranchLoopIter(Branch *branch, size_t i_channel) : branch(branch),
                                                     i_channel(i_channel),
                                                     current_channel(std::make_unique<BranchChannel>(branch,
                                                                                                     i_channel)) {}

  bool operator==(const BranchLoopIter &rhs) const {
    return i_channel == rhs.i_channel &&
        branch == rhs.branch;
  }
  bool operator!=(const BranchLoopIter &rhs) const {
    return !(rhs == *this);
  }
  BranchChannel operator*() const {
    return current_channel.operator*();
  }
  BranchChannel &operator*() {
    return current_channel.operator*();
  }
  BranchLoopIter &operator++();

  std::unique_ptr<BranchChannel> current_channel;
  Branch *branch;
  size_t i_channel;
};

struct BranchLoop {
  explicit BranchLoop(Branch *branch) : branch(branch) {}
  Branch *branch{nullptr};

  BranchLoopIter begin() const;
  BranchLoopIter end() const;
};

struct Branch {
  AnalysisTree::BranchConfig config;
  AnalysisTree::Configuration *parent_config;
  void *data{nullptr};
  bool is_connected_to_input{false};
  bool is_connected_to_output{false};
  bool is_mutable{false};
  bool is_frozen{false};

  void InitDataPtr();
  void ConnectOutputTree(TTree *tree);

  Variable GetVar(const std::string &field_name);

  /* Getting value */
  double Value(const Variable &v) const;

  size_t size() const;
  BranchChannel operator[](size_t i_channel);

  /* iterating */
  BranchLoop Loop() { return BranchLoop(this); };
  BranchLoopIter ChannelsBegin() { return BranchLoopIter(this, 0); };
  BranchLoopIter ChannelsEnd() { return BranchLoopIter(this, size()); };

  /* Modification */
  void Freeze(bool freeze = true) { is_frozen = freeze; };
  void CheckFrozen() const;
  void CheckMutable() const;
  BranchChannel NewChannel();
  template<typename T>
  Variable NewVariable(const std::string &field_name);

  template<typename EntityPtr>
  constexpr static const bool is_event_header_v =
      std::is_same_v<AnalysisTree::EventHeader, std::remove_const_t<std::remove_pointer_t<EntityPtr>>>;

  template<typename Functor>
  auto ApplyT(Functor &&f) {
    using AnalysisTree::DetType;

    if (config.GetType() == DetType::kParticle) {
      return f((AnalysisTree::Particles *) data);
    } else if (config.GetType() == DetType::kTrack) {
      return f((AnalysisTree::TrackDetector *) data);
    } else if (config.GetType() == DetType::kModule) {
      return f((AnalysisTree::ModuleDetector *) data);
    } else if (config.GetType() == DetType::kHit) {
      return f((AnalysisTree::HitDetector *) data);
    } else if (config.GetType() == DetType::kEventHeader) {
      return f((AnalysisTree::EventHeader *) data);
    }
    /* unreachable */
    assert(false);
  }

  template<typename Functor>
  auto ApplyT(Functor &&f) const {
    using AnalysisTree::DetType;

    if (config.GetType() == DetType::kParticle) {
      return f((const AnalysisTree::Particles *) data);
    } else if (config.GetType() == DetType::kTrack) {
      return f((const AnalysisTree::TrackDetector *) data);
    } else if (config.GetType() == DetType::kModule) {
      return f((const AnalysisTree::ModuleDetector *) data);
    } else if (config.GetType() == DetType::kHit) {
      return f((const AnalysisTree::HitDetector *) data);
    } else if (config.GetType() == DetType::kEventHeader) {
      return f((const AnalysisTree::EventHeader *) data);
    }
    /* unreachable */
    assert(false);
  }

};

struct Variable {
  Branch *parent_branch{nullptr};
  std::string name;
  std::string field_name;

  AnalysisTree::Types field_type{AnalysisTree::Types::kNumberOfTypes};
  short id{0};

  double operator*() const { return parent_branch->Value(*this); }

  void Print(std::ostream &os = std::cout) const;
};



template<typename T>
Variable Branch::NewVariable(const std::string &field_name) {
  CheckFrozen();
  config.template AddField<T>(field_name);

  ATI2::Variable v;
  v.name = config.GetName() + "/" + field_name;
  v.field_name = field_name;
  v.parent_branch = this;
  v.id = config.GetFieldId(field_name);
  v.field_type = config.GetFieldType(field_name);
  return v;
}



template<typename Functor>
auto BranchChannel::ApplyT(Functor &&functor) {
  using AnalysisTree::DetType;
  if (DetType::kParticle == branch->config.GetType()) {
    return functor((AnalysisTree::Particle *) Data());
  } else if (DetType::kHit == branch->config.GetType()) {
    return functor((AnalysisTree::Hit *) Data());
  } else if (DetType::kModule == branch->config.GetType()) {
    return functor((AnalysisTree::Module *) Data());
  } else if (DetType::kTrack == branch->config.GetType()) {
    return functor((AnalysisTree::Track *) Data());
  }
  assert(false);
}

template<typename Functor>
auto BranchChannel::ApplyT(Functor &&functor) const {
  using AnalysisTree::DetType;
  if (DetType::kParticle == branch->config.GetType()) {
    return functor((const AnalysisTree::Particle *) Data());
  } else if (DetType::kHit == branch->config.GetType()) {
    return functor((const AnalysisTree::Hit *) Data());
  } else if (DetType::kModule == branch->config.GetType()) {
    return functor((const AnalysisTree::Module *) Data());
  } else if (DetType::kTrack == branch->config.GetType()) {
    return functor((const AnalysisTree::Track *) Data());
  }
  assert(false);
}

} // namespace ATI2

#endif //ATTASKSKELETON_AT_TASK_ATI2_HPP_
