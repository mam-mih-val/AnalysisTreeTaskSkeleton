//
// Created by eugene on 17/02/2021.
//

#ifndef ATTASKSKELETON_AT_TASK_ATI2_HPP_
#define ATTASKSKELETON_AT_TASK_ATI2_HPP_

#include <AnalysisTree/BranchConfig.hpp>
#include <AnalysisTree/Configuration.hpp>
#include <AnalysisTree/Detector.hpp>
#include <AnalysisTree/EventHeader.hpp>
#include <utility>

namespace ATI2 {

class Variable;
struct Branch;
class ValueHolder;
class BranchChannel;
struct BranchChannelsIter;

class BranchChannel {
 public:
  /* Getting value */
  ValueHolder Value(const Variable &v) const;
  ValueHolder operator[](const Variable &v) const;
  void *Data() { return data_ptr; }
  const void *Data() const { return data_ptr; }

  /**
   * @brief Copy contents of other branch channel
   * @param other
   * @return
   */
  void CopyContents(const BranchChannel &other);

  void Print(std::ostream &os = std::cout) const;

 private:
  friend Branch;
  friend BranchChannelsIter;

  BranchChannel(Branch *branch, size_t i_channel);
  void UpdatePointer();
  void UpdateChannel(size_t new_channel);

  void *data_ptr{nullptr};
  Branch *branch;
  size_t i_channel;
};

struct BranchChannelsIter {
  BranchChannelsIter(Branch *branch, size_t i_channel) :
      branch(branch), i_channel(i_channel) {
    current_channel.reset(new BranchChannel(branch, i_channel));
  }

  bool operator==(const BranchChannelsIter &rhs) const {
    return i_channel == rhs.i_channel &&
        branch == rhs.branch;
  }
  bool operator!=(const BranchChannelsIter &rhs) const {
    return !(rhs == *this);
  }
  BranchChannel operator*() const {
    return current_channel.operator*();
  }
  BranchChannel &operator*() {
    return current_channel.operator*();
  }
  BranchChannelsIter &operator++();

  std::unique_ptr<BranchChannel> current_channel;
  Branch *branch;
  size_t i_channel;
};



struct Branch {
  struct BranchChannelsLoop {
    explicit BranchChannelsLoop(Branch *branch) : branch(branch) {}
    Branch *branch{nullptr};

    BranchChannelsIter begin() const;
    BranchChannelsIter end() const;
  };

  struct FieldsMapping {

    std::vector<std::pair<Variable /* src */, Variable /* dst */>> field_pairs;
  };

  ~Branch();


  AnalysisTree::BranchConfig config;
  AnalysisTree::Configuration *parent_config;
  void *data{nullptr}; /// owns object
  bool is_connected_to_input{false};
  bool is_connected_to_output{false};
  bool is_mutable{false};
  bool is_frozen{false};



  std::map<const Branch* /* other branch */, FieldsMapping> copy_fields_mapping;

  void InitDataPtr();
  void ConnectOutputTree(TTree *tree);

  Variable GetFieldVar(const std::string &field_name);
  bool HasField(const std::string &field_name) const;
  std::vector<std::string> GetFieldNames() const;

  /* Getting value */
  ValueHolder Value(const Variable &v) const;
  ValueHolder operator[](const Variable &v) const;

  /* iterating */
  size_t size() const;
  BranchChannel operator[](size_t i_channel);
  BranchChannelsLoop Loop() { return BranchChannelsLoop(this); };
  BranchChannelsIter ChannelsBegin() { return BranchChannelsIter(this, 0); };
  BranchChannelsIter ChannelsEnd() { return BranchChannelsIter(this, size()); };

  /* Modification */
  void Freeze(bool freeze = true) { is_frozen = freeze; };
  void CheckFrozen(bool expected = true) const;
  void CheckMutable(bool expected = true) const;
  BranchChannel NewChannel();
  void ClearChannels();
  Variable NewVariable(const std::string &field_name, AnalysisTree::Types type);
  void CopyContents(Branch *br);

  void CreateMapping(Branch *other);;

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

class Variable {

 public:
  ValueHolder operator*() const;
  void Print(std::ostream &os = std::cout) const;

  Branch *GetParentBranch() const {
    return parent_branch;
  }
  int GetId() const {
    return id;
  }
  AnalysisTree::Types GetFieldType() const {
    return field_type;
  }
  const std::string &GetName() const {
    return name;
  }
  const std::string &GetFieldName() const {
    return field_name;
  }
 private:
//  Variable() = default;
  friend Branch;

  Branch *parent_branch{nullptr};

  int id{0};
  AnalysisTree::Types field_type{AnalysisTree::Types::kNumberOfTypes};
  std::string name;
  std::string field_name;
};

class ValueHolder {
 public:
  float GetVal() const;
  int GetInt() const;
  bool GetBool() const;
  void SetVal(float val) const;
  void SetVal(int int_val) const;
  void SetVal(bool bool_val) const;

  operator float() const;

  template<typename T>
  ValueHolder& operator= (T new_val)  { SetVal(new_val); return *this; }
  ValueHolder& operator= (const ValueHolder& other);

 private:
  friend Branch;
  friend BranchChannel;

  ValueHolder(const Variable &v, void *data_ptr)
      : v(v), data_ptr(data_ptr) {
    entity_type = v.GetParentBranch()->config.GetType();
  }

  const Variable &v;
  AnalysisTree::DetType entity_type;
  void *data_ptr;
};




} // namespace ATI2

#endif //ATTASKSKELETON_AT_TASK_ATI2_HPP_
