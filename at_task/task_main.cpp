#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>

#include <infra/TaskManager.hpp>

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
  vector<string> enable_tasks;
  vector<string> disable_tasks;
  string output_file_name;
  string output_tree_name;
  int n_events{-1};

  std::string tasks_list;
  {
    std::stringstream tasks_list_stream;
    for (auto &t : TaskRegistry::getInstance()) {
      tasks_list_stream << t->GetName() << " ";
    }
    tasks_list = tasks_list_stream.str();
  }

  try {
    using namespace boost::program_options;
    options_description desc("Common options");
    desc.add_options()
        ("help,h", "print usage message")
        ("tasks", "print registered tasks")
        ("input-filelists,i", value(&at_filelists)->required()->multitoken(), "lists of AT ROOT Files")
        ("tree-names,t", value(&tree_names)->required()->multitoken(), "Tree names")
        ("output-file-name,o", value(&output_file_name)->default_value("output.root"),
         "Output ROOT filename")
        ("output-tree-name", value(&output_tree_name)->default_value("aTree"),
         "Output tree name")
        ("n-events,n", value(&n_events)->default_value(-1),"Number of events to process (-1 = until the end)")
        ("enable-tasks", value(&enable_tasks)->multitoken(), ("Enable specific tasks\nTasks: " + tasks_list).c_str())
        ("disable-tasks", value(&disable_tasks)->multitoken(), ("Disable specific tasks\nTasks: " + tasks_list).c_str());

    for (auto &task : TaskRegistry::getInstance()) {
      desc.add(task->GetBoostOptions());
    }

    variables_map vm;

    store(parse_command_line(argc, argv, desc), vm);
    conflicting_options(vm, "enable-tasks", "disable-tasks");

    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    } else if (vm.count("tasks")) {
      print_tasks();
      return 0;
    }

    vm.notify();
  }
  catch (exception &e) {
    cerr << e.what() << endl;
    return 1;
  }

  TaskManager task_manager(at_filelists, tree_names);

  for (auto &task : TaskRegistry::getInstance()) {
    cout << "Adding task '" << task->GetName() << "' to the task manager" << std::endl;
    try {
      task->PreInit();
    } catch (exception &e) {
      std::cerr << "Task '" << task->GetName() << "': " << e.what() << std::endl;
      return 1;
    }
    task_manager.AddTask(task.operator->());
  }

  task_manager.SetOutFileName(output_file_name);
  task_manager.SetOutTreeName(output_tree_name);

  task_manager.Init();
  task_manager.Run(n_events);
  task_manager.Finish();

  for (auto &task : TaskRegistry::getInstance()) {
    task->PostFinish();
  }

  return 0;
}

