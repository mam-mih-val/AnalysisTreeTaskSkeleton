//
// Created by eugene on 31/08/2020.
//

#ifndef ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
#define ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H

#include <TChain.h>

#include <at_task/Task.h>
#include <at_task/StaticTaskRegistry.h>

#include <AnalysisTree/Detector.hpp>
#include <AnalysisTree/Module.hpp>
#include <AnalysisTree/EventHeader.hpp>

class FooTask : public UserFillTask {
public:
  boost::program_options::options_description GetBoostOptions() override {
    /* insert your options here if need */
    return boost::program_options::options_description(GetName() + " options");
  }
  void PreInit() override {
    /* insert your pre-init here */
    SetInputBranchNames({"PsdModules", "SimEventHeader"});
    SetOutputBranchName("Centality");
    is_init = true;
  }

  void Init(std::map<std::string, void *> &branches_map) override {
    psd = static_cast<ModuleDetector *>(branches_map.at("PsdModules"));
    in_chain_->SetBranchAddress("PsdModules", &psd);

    AnalysisTree::BranchConfig centrality_branch(out_branch_, AnalysisTree::DetType::kEventHeader);
    centrality_branch.AddField<float>("Centrality_Epsd");
    centrality_Epsd_field_id = centrality_branch.GetFieldId("Centrality_Epsd");

    out_tree_->Branch(out_branch_.c_str(), "AnalysisTree::Container", &ana_event_header_);
    config_->AddBranchConfig(centrality_branch);
    ana_event_header_->Init(centrality_branch);

    branches_map.emplace("Centrality", ana_event_header_);
  }

  void Exec() override {
    auto n_channel = psd->GetNumberOfChannels();

    float total_signal = 0.;
    for (int ich = 0; ich < n_channel; ++ich) {
      auto channel = psd->GetChannel(ich);
      auto signal = channel.GetSignal();
      total_signal += signal;
    }
    ana_event_header_->SetField(total_signal, centrality_Epsd_field_id);

  }
  void Finish() override {

  }

private:
  typedef AnalysisTree::Detector<AnalysisTree::Module> ModuleDetector;

  AnalysisTree::Container *ana_event_header_{nullptr};
  ModuleDetector *psd{nullptr};
  int centrality_Epsd_field_id{-999};
  bool is_init{false};

TASK_DEF(FooTask, 1)
};

class BarTask : public UserFillTask {

public:
  void PreInit() override {
    auto foo_task = GetTaskPtr<FooTask>();
    std::cout << foo_task->GetName() << std::endl;
  }

  void Init(std::map<std::string, void *> &Map) override {
    ReadMap(Map);

    vtx_x = GetVar("RecEventHeader/vtx_x");
    vtx_x.Print();

    vtx_tracks = GetBranch("VtxTracks");

    dca_x = GetVar("VtxTracks/dcax");
    dca_x.Print();

    NewBranch("test", AnalysisTree::DetType::kParticle);


  }
  void Exec() override {
    auto vtx_x_val = vtx_x.Get<float>();
    std::cout << vtx_x_val << std::endl;

//    std::cout << dca_x.Get<float>() << std::endl; /* not implemented */

    for (auto track : vtx_tracks->Loop()) {
      track.Print();
      std::cout << track.Get<float>(dca_x) << std::endl;
    }

  }
  void Finish() override {

  }

private:
  AnalysisTree::Container *centrality_{nullptr};

  Variable vtx_x;

  Branch *vtx_tracks;
  Variable dca_x;

 TASK_DEF(BarTask, 1);
};

#endif //ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
