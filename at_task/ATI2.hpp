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
  template<typename T>
  T Get(const Variable &v) const;

  void Print(std::ostream &os = std::cout) const;

  void UpdatePointer();
  void ResetPointer() { data_ptr = nullptr; }

  const void *data_ptr;
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

  void InitDataPtr();

  void ConnectOutputTree(TTree *tree);

  /* Getting value */
  template<typename T>
  T Get(const Variable &v) const;

  size_t size() const;
  BranchChannel operator[](size_t i_channel);

  /* iterating */
  BranchLoop Loop() { return BranchLoop(this); };
  BranchLoopIter ChannelsBegin() { return BranchLoopIter(this, 0); };
  BranchLoopIter ChannelsEnd() { return BranchLoopIter(this, size()); };

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
  short id{0};

  template<typename T>
  T Get() { return parent_branch->template Get<T>(*this); }

  void Print(std::ostream &os = std::cout) const {
    os << name << "(id = " << id << ")" << std::endl;
  }
};

template<typename T>
T ATI2::Branch::Get(const ATI2::Variable &v) const {
  return ApplyT([&v](auto entity) -> T {
    if constexpr (std::is_same_v<AnalysisTree::EventHeader,
                                 std::remove_const_t<std::remove_pointer_t<decltype(entity)>>>) {
      return entity->template GetField<T>(v.id);
    }
    throw std::runtime_error("Get is not implemented for iterable detectors");
  });
}

template<typename T>
T ATI2::BranchChannel::Get(const ATI2::Variable &v) const {
  return branch->template ApplyT([this, &v](auto entity_ptr) -> T {
    return entity_ptr->GetChannel(this->i_channel).template GetField<T>(v.id);
  });
}


} // namespace ATI2

#endif //ATTASKSKELETON_AT_TASK_ATI2_HPP_
