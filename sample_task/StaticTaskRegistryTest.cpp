//
// Created by eugene on 01/10/2020.
//

#include <functional>
#include <string>
#include <at_task/StaticTaskRegistry.h>

class FooTask {
public:
  std::string GetName() { return "FooTask"; }
};
class BarTask {
public:
  std::string GetName() { return "BarTask"; }
};

template<typename T>
std::string GetName(const T& task) {
  return task.GetName();
}

int main() {
  StaticTaskManager<FooTask,BarTask> tml;
  auto names = tml.visit([] (auto&&t) {return t.GetName(); });

  return 0;
}

