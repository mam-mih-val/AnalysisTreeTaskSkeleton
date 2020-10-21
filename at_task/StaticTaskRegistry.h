//
// Created by eugene on 01/10/2020.
//

#ifndef ATTASKSKELETON_AT_TASK_STATICTASKREGISTRY_H
#define ATTASKSKELETON_AT_TASK_STATICTASKREGISTRY_H

#include <tuple>
#include <vector>

template<typename ... Tasks>
class StaticTaskManager {
public:
  static constexpr std::size_t n_tasks = sizeof...(Tasks);
  template<std::size_t I>
  using task_type = typename std::tuple_element<I, std::tuple<Tasks...>>::type;

  template<typename Task>
  Task &Get() { return std::get<Task>(tasks_); }
  template<std::size_t I>
  typename std::add_lvalue_reference_t<task_type<I>> Get() { return std::get<I>(tasks_); }

  template<typename Visitor>
  auto visit(Visitor&&v) {
    return visit_impl(std::forward<Visitor>(v), std::make_index_sequence<n_tasks>());
  }
private:
  template<typename Visitor, std::size_t...I>
  auto visit_impl(Visitor&&v, std::index_sequence<I...>) {
    using ret_type = std::invoke_result_t<Visitor,task_type<1>>;
    return make_vector<ret_type, I...>(v(Get<I>())...);
  }

  template<typename T, size_t I>
  using IndexedArg = T;

  template<typename T, std::size_t ... I>
  std::vector<T> make_vector(IndexedArg<T,I>...args) {
    std::vector<T> v({args...});
    return v;
  }

  std::tuple<Tasks...> tasks_;
};

#endif //ATTASKSKELETON_AT_TASK_STATICTASKREGISTRY_H
