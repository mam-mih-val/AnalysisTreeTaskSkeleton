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
  BranchChannel(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {}

  /* Getting value */
  template<typename T>
  T Get(const Variable &v) const;

  void Print(std::ostream &os = std::cout) const;

  Branch *branch;
  size_t i_channel;
};

struct BranchLoopIter {
  BranchLoopIter(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {}

  bool operator==(const BranchLoopIter &rhs) const {
    return i_channel == rhs.i_channel &&
        branch == rhs.branch;
  }
  bool operator!=(const BranchLoopIter &rhs) const {
    return !(rhs == *this);
  }
  BranchChannel operator*() const {
    return BranchChannel(branch, i_channel);
  }
  BranchLoopIter &operator++() {
    i_channel++;
    return *this;
  }

  Branch *branch{nullptr};
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

  void InitDataPtr();

  void ConnectOutputTree(TTree *tree);

  /* Getting value */
  template<typename T>
  T Get(const Variable &v) const;

  size_t size() const;
  BranchChannel operator[] (size_t i_channel) { return BranchChannel(this, i_channel); }

  /* iterating */
  BranchLoop Loop() { return BranchLoop(this); };
  BranchLoopIter ChannelsBegin() { return BranchLoopIter(this, 0); };
  BranchLoopIter ChannelsEnd() { return BranchLoopIter(this, size()); };

  template<typename Functor>
  void ApplyT(Functor &&f) {
    using AnalysisTree::DetType;

    if (config.GetType() == DetType::kParticle) {
      f((AnalysisTree::Particles *) data);
    } else if (config.GetType() == DetType::kTrack) {
      f((AnalysisTree::TrackDetector *) data);
    } else if (config.GetType() == DetType::kModule) {
      f((AnalysisTree::ModuleDetector *) data);
    } else if (config.GetType() == DetType::kHit) {
      f((AnalysisTree::HitDetector *) data);
    } else if (config.GetType() == DetType::kEventHeader) {
      f((AnalysisTree::EventHeader *) data);
    }
    /* unreachable */
  }

  template<typename Functor>
  void ApplyT(Functor &&f) const {
    using AnalysisTree::DetType;

    if (config.GetType() == DetType::kParticle) {
      f((const AnalysisTree::Particles *) data);
    } else if (config.GetType() == DetType::kTrack) {
      f((const AnalysisTree::TrackDetector *) data);
    } else if (config.GetType() == DetType::kModule) {
      f((const AnalysisTree::ModuleDetector *) data);
    } else if (config.GetType() == DetType::kHit) {
      f((const AnalysisTree::HitDetector *) data);
    } else if (config.GetType() == DetType::kEventHeader) {
      f((const AnalysisTree::EventHeader *) data);
    }
    /* unreachable */
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
struct BranchFieldGetImpl {
  BranchFieldGetImpl(T &result, const ATI2::Variable &var) : result(result), var(var) {}
  BranchFieldGetImpl(T &result, const ATI2::Variable &var, size_t i_channel)
      : result(result), var(var), i_channel(i_channel) {}

  template<typename Entity>
  void Eval(const Entity *entity) { result = entity->GetChannel(i_channel).template GetField<T>(var.id); }
  void Eval(const AnalysisTree::EventHeader *event_header) { result = event_header->template GetField<T>(var.id); };
  template<typename Entity>
  void operator()(const Entity *e) { Eval(e); }

  T &result;
  const ATI2::Variable &var;
  size_t i_channel{0};
};

template<typename T>
T ATI2::Branch::Get(const ATI2::Variable &v) const {
  T result;
  ApplyT(BranchFieldGetImpl<T>(result, v));
  return result;
}

template<typename T>
T ATI2::BranchChannel::Get(const ATI2::Variable &v) const {
  T result;
  branch->template ApplyT(BranchFieldGetImpl<T>(result, v, i_channel));
  return result;
}

} // namespace ATI2

#endif //ATTASKSKELETON_AT_TASK_ATI2_HPP_
