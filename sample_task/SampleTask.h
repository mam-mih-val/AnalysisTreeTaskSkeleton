//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
#define ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H

#include <TChain.h>

#include "Task.h"

#include <core/Detector.hpp>
#include <core/Module.hpp>

class FooTask : public UserTask {
public:
  boost::program_options::options_description GetBoostOptions() override {
    /* insert your options here if need */
    return boost::program_options::options_description(GetName() + " options");
  }
  void PreInit() override {
    /* insert your pre-init here */
    SetInputBranchNames({"PsdModules"});
  }

  void Init(std::map<std::string, void *> &branches_map) override {
    psd = static_cast<ModuleDetector *>(branches_map.at("PsdModules"));
    in_chain_->SetBranchAddress("PsdModules", &psd);
  }

  void Exec() override {
    auto n_channel = psd->GetNumberOfChannels();

    for (int ich = 0; ich < n_channel; ++ich) {
      auto channel = psd->GetChannel(ich);
      auto signal = channel.GetSignal();
      std::cout << signal << std::endl;
    }


  }
  void Finish() override {

  }

private:
  typedef AnalysisTree::Detector<AnalysisTree::Module> ModuleDetector;

  ModuleDetector *psd {nullptr};

TASK_DEF(FooTask)
};


#endif //ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
