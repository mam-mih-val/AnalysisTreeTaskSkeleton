//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
#define ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H

#include <boost/program_options.hpp>
#include <AnalysisTree/FillTask.hpp>
#include <AnalysisTree/EventHeader.hpp>
#include <AnalysisTree/Detector.hpp>
#include <TTree.h>

class TaskRegistry;

class UserTask {

 public:
  virtual ~UserTask() = default;
  virtual std::string GetName() const { return ""; }
  virtual size_t GetPriority() const { return 0l; }
  virtual AnalysisTree::FillTask *FillTaskPtr() = 0;

  virtual boost::program_options::options_description GetBoostOptions() { return {}; }
  virtual void ProcessBoostVM(const boost::program_options::variables_map &vm) { (void) vm; }

  virtual void PreInit() {}
  virtual void PostFinish() {}

  size_t GetOrderNo() const {
    return order_no_;
  }

  bool IsEnabled() const {
    return is_enabled_;
  }

 private:
  friend TaskRegistry;

  bool is_enabled_{true};
  std::size_t order_no_{0};
};

class UserFillTask : public UserTask, public AnalysisTree::FillTask {
 public:
  virtual ~UserFillTask() = default;
  AnalysisTree::FillTask *FillTaskPtr() final {
    return this;
  }

 protected:
  typedef short VariableIndex;
  /**
   * @brief This function simplifies routine of getting the variable Id from
   * <b>input</b> config during the initialization of the program (usually Init()).
   * Effectively replaces config_->GetBranchConfig("BranchName")->GetFieldId("FieldName")
   * Checks existence of the requested branch
   * @param variable_name - name of the variable in format "BranchName/FieldName"
   * @return variable id
   */
  VariableIndex VarId(const std::string &variable_name) const;
  VariableIndex VarId(const std::string &branch_name, const std::string &field_name) const;

  /**
   * @brief This function creates new branch in the out_config_
   * @param branch_name
   * @param detector_type
   * @return
   */
  AnalysisTree::BranchConfig &NewBranch(const std::string &branch_name, AnalysisTree::DetType detector_type);

  /**
   * @brief This function initialize new variable in the <b>output</b> config.
   * @tparam T - type of the variable
   * @param variable_name - name of the variable in format "BranchName/FieldName"
   * @return variable Id
   */
  template<typename T>
  short NewVar(const std::string &variable_name) {
    auto &&[branch_name, field_name] = ParseVarName(variable_name);

    auto &branch = out_config_->GetBranchConfig(branch_name);
    branch.template AddField<T>(field_name);
    return branch.GetFieldId(field_name);
  }

  void ReadMap(std::map<std::string, void *> &map);

 public:

  struct Variable;
  struct Branch;
  struct BranchLoop;

  Branch *GetBranch(const std::string &name) const { return branches_in_.at(name).get(); }
  Variable GetVar(const std::string &name) const {
    auto &&[br_name, f_name] = ParseVarName(name);

    Variable v;
    v.parent_branch = GetBranch(br_name);
    v.id = v.parent_branch->config.GetFieldId(f_name);
    v.name = name;

    if (v.id == AnalysisTree::UndefValueShort)
      throw std::runtime_error("Field of name '" + v.name + "' not found");

    return v;
  }

  struct BranchChannel {
    BranchChannel(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {}

    /* Getting value */
    template<typename T>
    T Get(const Variable &v) const;

    void Print(std::ostream &os = std::cout) const {
      os << "Branch " << branch->config.GetName() << " channel #" << i_channel << std::endl;
    }

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

    BranchLoopIter begin() const { return branch->ChannelsBegin(); }
    BranchLoopIter end() const { return branch->ChannelsEnd(); }
  };

  struct Branch {
    AnalysisTree::BranchConfig config;
    AnalysisTree::Configuration *parent_config;
    void *data{nullptr};

    void InitDataPtr() {
      ApplyT([this] (auto entity) {
        if (entity)
          throw std::runtime_error("Data ptr is already initialized");
        this->data = new typename std::remove_pointer<decltype(entity)>::type;
      });
    }

    void ConnectOutputTree(TTree *tree) {
      ApplyT([this,tree] (auto entity) {
        tree->Branch(config.GetName().c_str(),
                     std::add_pointer_t<decltype(entity)>(&this->data));
      });
    }


    /* Getting value */
    template<typename T>
    T Get(const Variable &v) const;

    size_t size() const;

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

  static std::pair<std::string, std::string> ParseVarName(const std::string &variable_name);
  std::map<std::string, std::unique_ptr<Branch>> branches_in_;
  std::map<std::string, std::unique_ptr<Branch>> branches_out_;

};

template<typename T>
struct BranchFieldGetImpl {
  BranchFieldGetImpl(T &result, const UserFillTask::Variable &var) : result(result), var(var) {}
  BranchFieldGetImpl(T &result, const UserFillTask::Variable &var, size_t i_channel)
      : result(result), var(var), i_channel(i_channel) {}

  template<typename Entity>
  void Eval(const Entity *entity) { result = entity->GetChannel(i_channel).template GetField<T>(var.id); }
  void Eval(const AnalysisTree::EventHeader *event_header) { result = event_header->template GetField<T>(var.id); };
  template<typename Entity>
  void operator()(const Entity *e) { Eval(e); }

  T &result;
  const UserFillTask::Variable &var;
  size_t i_channel{0};
};

template<typename T>
T UserFillTask::Branch::Get(const UserFillTask::Variable &v) const {
  T result;
  ApplyT(BranchFieldGetImpl<T>(result, v));
  return result;
}

template<typename T>
T UserFillTask::BranchChannel::Get(const UserFillTask::Variable &v) const {
  T result;
  branch->template ApplyT(BranchFieldGetImpl<T>(result, v, i_channel));
  return result;
}

struct BranchSizeImpl {
  explicit BranchSizeImpl(size_t &result) : result(result) {}
  size_t &result;

  template<typename Entity>
  void Eval(const Entity *e) { result = e->GetNumberOfChannels(); }
  void Eval(const AnalysisTree::EventHeader& e) { throw std::runtime_error("Not implemented"); }

  template<typename Entity>
  void operator() (const Entity* e) { Eval(e); }
};
inline
size_t UserFillTask::Branch::size() const {
  size_t result;
  ApplyT(BranchSizeImpl(result));
  return result;
}

#endif //ANALYSISTREESKELETON_TASK_MAIN_USERTASK_H
