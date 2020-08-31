#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>

#include <infra/TaskManager.hpp>

#include "TaskRegistry.h"

using namespace std;
using namespace AnalysisTree;
int main(int argc, char ** argv) {

  vector<string> at_filelists;
  vector<string> tree_names;
  string output_file_name;
  string output_tree_name;

  try {
    using namespace boost::program_options;
    options_description desc("Common options");
    desc.add_options()
        ("help,h", "print usage message")
        ("input-filelists,i", value(&at_filelists)->required()->multitoken(), "lists of AT ROOT Files")
        ("tree-names,t", value(&tree_names)->required()->multitoken(), "Tree names")
        ("output-file-name,o", value(&output_file_name)->default_value("output.root"),
         "Output ROOT filename")
        ("output-tree-name", value(&output_tree_name)->default_value("aTree"),
         "Output tree name");

    for (
        auto it = TaskRegistry::getInstance().tasks_begin();
        it != TaskRegistry::getInstance().tasks_end();
        ++it
        ) {
      desc.add((*it)->GetBoostOptions());
      (*it)->PreInit();
    }

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    }

    vm.notify();
  }
  catch (exception &e) {
    cerr << e.what() << endl;
    return 1;
  }

  TaskManager task_manager(at_filelists, tree_names);

  for (
      auto it = TaskRegistry::getInstance().tasks_begin();
      it != TaskRegistry::getInstance().tasks_end();
      ++it
      ) {
    cout << "Adding task '" << (*it)->GetName() << "' to the task manager" << std::endl;
  }

  task_manager.SetOutFileName(output_file_name);
  task_manager.SetOutTreeName(output_tree_name);

  return 0;
}

