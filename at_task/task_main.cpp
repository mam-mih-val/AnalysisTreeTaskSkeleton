#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>

#include <AnalysisTree/TaskManager.hpp>
#include <AnalysisTreeCutsRegistry/CutsRegistry.hpp>

#include "TaskRegistry.h"

using namespace std;
using namespace AnalysisTree;

void conflicting_options(const boost::program_options::variables_map & vm,
                         const std::string & opt1, const std::string & opt2)
{
  if (vm.count(opt1) && !vm[opt1].defaulted() &&
      vm.count(opt2) && !vm[opt2].defaulted())
  {
    throw std::logic_error(std::string("Conflicting options '") +
        opt1 + "' and '" + opt2 + "'.");
  }
}

void print_tasks() {
  for (auto& task : TaskRegistry::getInstance()) {
    cout << "#" << task->GetOrderNo() << " " << task->GetName() << "(prio=" << task->GetPriority() << ")" << std::endl;
  }
}

int main(int argc, char ** argv) {

  vector<string> at_filelists;
  vector<string> tree_names;
  bool enable_tasks_count{false};
  vector<string> enabled_task_names;
  bool disable_tasks_count{false};
  vector<string> disabled_task_names;
  string output_file_name;
  string output_tree_name;
  int n_events{-1};

  bool cuts_macro_count{false};
  string cuts_macro;
  bool event_cuts_count{false};
  string event_cuts;
  vector<string> branch_cuts;


  std::string tasks_list;
  {
    std::stringstream tasks_list_stream;
    for (auto &task_name : TaskRegistry::getInstance().GetTaskNames()) {
      tasks_list_stream << task_name << " ";
    }
    tasks_list = tasks_list_stream.str();
  }

  using namespace boost::program_options;
  options_description desc("Common options");
  try {
    desc.add_options()
        ("help,h", "print usage message")
        ("tasks", "print registered tasks")
        ("input-filelists,i", value(&at_filelists)->required()->multitoken(), "lists of AT ROOT Files")
        ("tree-names,t", value(&tree_names)->required()->multitoken(), "Tree names")
        ("output-file-name,o", value(&output_file_name)->default_value(""),
         "Output ROOT filename")
        ("output-tree-name", value(&output_tree_name)->default_value("aTree"),
         "Output tree name")
        ("n-events,n", value(&n_events)->default_value(-1),"Number of events to process (-1 = until the end)")
        ("enable-tasks", value(&enabled_task_names)->multitoken(), ("Enable specific tasks\nTasks: " + tasks_list).c_str())
        ("disable-tasks", value(&disabled_task_names)->multitoken(), ("Disable specific tasks\nTasks: " + tasks_list).c_str())
        /* cuts management */
        ("cuts-macro", value(&cuts_macro), ("Macro with cuts definitions"))
        ("event-cuts", value(&event_cuts), ("Name of event cuts"))
        ("branch-cuts", value(&branch_cuts), ("Name(s) of branch cuts"))
        ;

//    for (auto &task : TaskRegistry::getInstance()) {
//      desc.add(task->GetBoostOptions());
//    }

    variables_map vm;

    store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    } else if (vm.count("tasks")) {
      print_tasks();
      return 0;
    }

    conflicting_options(vm, "enable-tasks", "disable-tasks");
    enable_tasks_count = vm.count("enable-tasks");
    disable_tasks_count = vm.count("disable-tasks");

    cuts_macro_count = vm.count("cuts-macro");
    event_cuts_count = vm.count("event-cuts");

    vm.notify();
  }
  catch (exception &e) {
    cerr << e.what() << endl;
    cerr << desc << endl;
    return 1;
  }

  if (enable_tasks_count) {
    TaskRegistry::getInstance().EnableTasks(enabled_task_names);
  } else if (disable_tasks_count) {
    TaskRegistry::getInstance().DisableTasks(disabled_task_names);
  }
  TaskRegistry::getInstance().LoadEnabledTasks();

  TaskManager task_manager(at_filelists, tree_names);

  if (cuts_macro_count) {
    LoadCutsFromFile(cuts_macro.c_str());
  }

  if (event_cuts_count) {
    task_manager.SetEventCuts(new Cuts(gCutsRegistry.at(event_cuts)));
  }



  for (auto &task : TaskRegistry::getInstance()) {
    cout << "Adding task '" << task->GetName() << "' to the task manager" << std::endl;
    try {
      task->PreInit();
    } catch (exception &e) {
      std::cerr << "Task '" << task->GetName() << "': " << e.what() << std::endl;
      return 1;
    }
    task_manager.AddTask(task->FillTaskPtr());
  }

  task_manager.SetOutFileName(output_file_name);
  task_manager.SetOutTreeName(output_tree_name);

  task_manager.Init();
  task_manager.Run(n_events);
  task_manager.Finish();

  for (auto &task : TaskRegistry::getInstance()) {
    task->PostFinish();
  }

  TaskRegistry::getInstance().UnloadAllTasks();
  return 0;
}

