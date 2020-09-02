//
// Created by eugene on 31/08/2020.
//

#include "TaskRegistry.h"

TaskRegistry &TaskRegistry::getInstance() {
  static TaskRegistry registry;
  return registry;
}
