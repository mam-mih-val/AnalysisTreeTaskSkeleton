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


    rec_event_header_vtx_x = GetVar("RecEventHeader/vtx_x");
    rec_event_header_vtx_x.Print();

    vtx_tracks_branch = GetInBranch("VtxTracks");

    vtxtracks_dca_x = GetVar("VtxTracks/dcax");
    vtxtracks_dca_x.Print();

    NewBranch("ProcessedTracks", AnalysisTree::DetType::kTrack);

    processed_tracks_branch = GetOutBranch("ProcessedTracks");
    /* We will take dcax/y from VtxTracks */
    processed_tracks_dcax = processed_tracks_branch->NewVariable("dcax", AnalysisTree::Types::kFloat);
//    processed_tracks_dcay = processed_tracks_branch->NewVariable("dcay", AnalysisTree::Types::kFloat);

    NewBranch("test_event_header", AnalysisTree::DetType::kEventHeader);
    test_event_header = GetOutBranch("test_event_header");
    test_event_header_vtx_x = test_event_header->GetFieldVar("vtx_x");
    rec_event_header = GetInBranch("RecEventHeader");

    test_event_header->Freeze(); /* No more structural changes */

  }
  void Exec() override {

    /* Abilities of ATI2::Branch */
    test_event_header->CopyContents(rec_event_header);
//    std::cout << (*rec_event_header)[rec_event_header_vtx_x] << "\t"
//              << (*test_event_header)[test_event_header_vtx_x] << std::endl;




    auto vtx_x_val = *rec_event_header_vtx_x;
//    std::cout << float(vtx_x_val) << std::endl;


//    std::cout << dca_x.Get<float>() << std::endl; /* not implemented */
//    GetBranch("RecEventHeader")->size(); /* not implemented */

    processed_tracks_branch->ClearChannels();
    /* abilities of BranchChannel */
    for (auto &vtx_track : vtx_tracks_branch->Loop()) {
      auto processed_track = processed_tracks_branch->NewChannel();
//      std::cout << "Current size " << processed_tracks_branch->size() << std::endl;
      processed_track.CopyContents(vtx_track);
//      std::cout << vtx_track[vtxtracks_dca_x].GetVal() << "\t"
//                << processed_track[processed_tracks_dcax].GetVal() << std::endl;
    }

  }
  void Finish() override {

  }

private:
  AnalysisTree::Container *centrality_{nullptr};

  ATI2::Branch *rec_event_header{nullptr};
  ATI2::Variable rec_event_header_vtx_x;

  ATI2::Branch *test_event_header{nullptr};

  ATI2::Variable test_event_header_vtx_x;
  ATI2::Variable vtxtracks_dca_x;
  ATI2::Branch *processed_tracks_branch;
  ATI2::Variable processed_tracks_dcax;

  ATI2::Branch *vtx_tracks_branch;
 TASK_DEF(BarTask, 1);
};

#endif //ANALYSISTREESKELETON_SAMPLE_TASK_SAMPLETASK_H
