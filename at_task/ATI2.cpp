//
// Created by eugene on 17/02/2021.
//

#include <iostream>

#include <TTree.h>

#include "ATI2.hpp"

using namespace ATI2;

void BranchChannel::Print(std::ostream &os) const {
  os << "Branch " << branch->config.GetName() << " channel #" << i_channel << std::endl;
}

BranchChannelsIter Branch::BranchChannelsLoop::begin() const { return branch->ChannelsBegin(); }

BranchChannelsIter Branch::BranchChannelsLoop::end() const { return branch->ChannelsEnd(); }

Variable Branch::GetFieldVar(const std::string &field_name) {
  ATI2::Variable v;
  v.parent_branch = this;
  v.id = v.parent_branch->config.GetFieldId(field_name);
  v.name = this->config.GetName() + "/" + field_name;
  v.field_name = field_name;
  v.field_type = config.GetFieldType(field_name);

  if (v.id == AnalysisTree::UndefValueShort)
    throw std::runtime_error("Field of name '" + v.name + "' not found");
  return v;
}

void Branch::ConnectOutputTree(TTree *tree) {
  is_connected_to_output = ApplyT([this, tree](auto entity) -> bool {
    if (!tree)
      return false;
    auto new_tree_branch_ptr = tree->Branch(config.GetName().c_str(),
                                            std::add_pointer_t<decltype(entity)>(&this->data));
    return bool(new_tree_branch_ptr);
  });
}
void Branch::InitDataPtr() {
  ApplyT([this](auto entity) {
    if (entity)
      throw std::runtime_error("Data ptr is already initialized");
    this->data = new typename std::remove_pointer<decltype(entity)>::type;
  });
}

size_t ATI2::Branch::size() const {
  return ApplyT([](auto entity_ptr) -> size_t {
    if constexpr (is_event_header_v < decltype(entity_ptr) >) {
      throw std::runtime_error("Size is not implemented for EventHeader variable");
    } else {
      return entity_ptr->GetNumberOfChannels();
    }
  });
}

BranchChannel::BranchChannel(Branch *branch, size_t i_channel) : branch(branch), i_channel(i_channel) {
  UpdatePointer();
}

BranchChannel Branch::operator[](size_t i_channel) { return BranchChannel(this, i_channel); }

BranchChannel Branch::NewChannel() {
  CheckMutable(true);
  ApplyT([this](auto entity_ptr) {
    if constexpr (is_event_header_v < decltype(entity_ptr) >) {
      throw std::runtime_error("Not applicable for EventHeader");
    } else {
      auto channel = entity_ptr->AddChannel();
      channel->Init(this->config);
      Freeze();
    }
  });
  return operator[](size() - 1);
}
Variable Branch::NewVariable(const std::string &field_name, AnalysisTree::Types type) {
  CheckFrozen(false);
  CheckMutable(true);

  using AnalysisTree::Types;

  if (Types::kFloat == type) {
    config.template AddField<float>(field_name);
  } else if (Types::kInteger == type) {
    config.template AddField<int>(field_name);
  } else if (Types::kBool == type) {
    config.template AddField<bool>(field_name);
  } else {
    /* should never happen */
    assert(false);
  }

  ATI2::Variable v;
  v.name = config.GetName() + "/" + field_name;
  v.field_name = field_name;
  v.parent_branch = this;
  v.id = config.GetFieldId(field_name);
  v.field_type = config.GetFieldType(field_name);
  return v;
}

void Branch::ClearChannels() {
  CheckMutable();
  ApplyT([this](auto entity_ptr) -> void {
    if constexpr (is_event_header_v < decltype(entity_ptr) >) {
      throw std::runtime_error("Not applicable for EventHeader");
    } else {
      entity_ptr->ClearChannels();
    }
  });
}

void Branch::CheckFrozen(bool expected) const {
  if (is_frozen != expected)
    throw std::runtime_error("Branch is frozen");
}
void Branch::CheckMutable(bool expected) const {
  if (is_mutable != expected)
    throw std::runtime_error("Branch is not mutable");
}

ValueHolder Branch::Value(const Variable &v) const {
  if (config.GetType() == AnalysisTree::DetType::kEventHeader) {
    return ValueHolder(v, data);
  }
  throw std::runtime_error("Not implemented for iterable branch");
}
ValueHolder Branch::operator[](const Variable &v) const { return Value(v); }

bool Branch::HasField(const std::string &field_name) const {
  auto field_id = config.GetFieldId(field_name);
  return field_id != AnalysisTree::UndefValueShort;
}
std::vector<std::string> Branch::GetFieldNames() const {
  std::vector<std::string> result;
  auto fill_vector_from_map = [&result] (const std::map<std::string, short> &fields_map) -> void {
    for (auto &element : fields_map) {
      result.push_back(element.first);
    }
  };
  fill_vector_from_map(config.GetMap<float>());
  fill_vector_from_map(config.GetMap<int>());
  fill_vector_from_map(config.GetMap<bool>());
  return result;
}
void Branch::CopyContents(Branch *other) {
  if (this == other) {
    throw std::runtime_error("Copying contents from the same branch makes no sense");
  }
  CheckFrozen();
  CheckMutable();
  other->CheckFrozen();

  if (other->config.GetType() != config.GetType()) {
    throw std::runtime_error("Branch types must be the same");
  }
  if (config.GetType() != AnalysisTree::DetType::kEventHeader) {
    throw std::runtime_error("Only EventHeader is available for Branch::CopyContents");
  }

  auto mapping_it = copy_fields_mapping.find(other);
  if (mapping_it == copy_fields_mapping.end()) {
    /* new mapping is needed */
    /// TODO debug message
    FieldsMapping fields_mapping;
    for (auto &field_name : other->GetFieldNames()) {
      if (!HasField(field_name)) { continue; }
      fields_mapping.mapping.emplace_back(std::make_pair(other->GetFieldVar(field_name), GetFieldVar(field_name)));
    }
    copy_fields_mapping.emplace(other, std::move(fields_mapping));
    mapping_it = copy_fields_mapping.find(other);
  }

  /* evaluate mapping */
  auto src_branch = mapping_it->first;
  const auto mapping = mapping_it->second;

  for (auto &field_pair /* src : dst */: mapping.mapping) {
    this->Value(field_pair.second) = src_branch->Value(field_pair.first);
  }

}

ValueHolder ATI2::BranchChannel::Value(const ATI2::Variable &v) const {
  return ValueHolder(v, data_ptr);
}

ValueHolder BranchChannel::operator[](const Variable &v) const { return Value(v); }

void BranchChannel::UpdateChannel(size_t new_channel) {
  i_channel = new_channel;
  UpdatePointer();
}
void BranchChannel::UpdatePointer() {
  if (i_channel < branch->size()) {
    data_ptr = branch->ApplyT([this](auto entity_ptr) -> void * {
      if constexpr (Branch::is_event_header_v<decltype(entity_ptr)>) {
        throw std::runtime_error("Getting channel of the EventHeader is not implemented");
      } else {
        return &entity_ptr->GetChannel(this->i_channel);
      }
    });
  } else {
    data_ptr = nullptr;
  }
}

BranchChannelsIter &BranchChannelsIter::operator++() {
  i_channel++;
  current_channel->UpdateChannel(i_channel);
  return *this;
}

ValueHolder Variable::operator*() const { return parent_branch->Value(*this); }

void Variable::Print(std::ostream &os) const {
  os << name << "(id = " << id << ")" << std::endl;
}

namespace Impl {

template<typename Entity>
inline Entity *DataT(void *data_ptr) { return reinterpret_cast<Entity *>(data_ptr); }
template<typename Entity>
inline const Entity *DataT(const void *data_ptr) { return reinterpret_cast<const Entity *>(data_ptr); }

template<typename Functor, typename DataPtr>
auto ApplyToEntity(AnalysisTree::DetType det_type, DataPtr ptr, Functor &&functor) {
  using AnalysisTree::DetType;
  if (DetType::kParticle == det_type) {
    return functor(DataT<AnalysisTree::Particle>(ptr));
  } else if (DetType::kTrack == det_type) {
    return functor(DataT<AnalysisTree::Track>(ptr));
  } else if (DetType::kModule == det_type) {
    return functor(DataT<AnalysisTree::Module>(ptr));
  } else if (DetType::kHit == det_type) {
    return functor(DataT<AnalysisTree::Hit>(ptr));
  } else if (DetType::kEventHeader == det_type) {
    return functor(DataT<AnalysisTree::EventHeader>(ptr));
  }
  /* unreachable */
  assert(false);
}

template<typename Entity, typename ValueType>
ValueType ReadValue(const Variable &v, const Entity *e) {
  using AnalysisTree::Types;

  if (v.GetFieldType() == Types::kFloat) {
    return (ValueType) e->template GetField<float>(v.GetId());
  } else if (v.GetFieldType() == Types::kInteger) {
    return (ValueType) e->template GetField<int>(v.GetId());
  } else if (v.GetFieldType() == Types::kBool) {
    return (ValueType) e->template GetField<bool>(v.GetId());
  }
  /* unreachable */
  assert(false);
}

template<typename Entity, typename ValueType>
inline
void SetValue(const Variable &v, Entity *e, ValueType new_value) {
  using AnalysisTree::Types;

  if (v.GetFieldType() == Types::kFloat) {
    e->template SetField<float>(new_value, v.GetId());
    return;
  } else if (v.GetFieldType() == Types::kInteger) {
    e->template SetField<int>(new_value, v.GetId());
    return;
  } else if (v.GetFieldType() == Types::kBool) {
    e->template SetField<bool>(new_value, v.GetId());
    return;
  }
  /* unreachable */
  assert(false);
}

} // namespace Impl
float ValueHolder::GetVal() const {
  return Impl::ApplyToEntity(v.GetParentBranch()->config.GetType(),
                             data_ptr, [this](auto entity_ptr) {
        using Entity = std::remove_const_t<std::remove_pointer_t<decltype(entity_ptr)>>;
        return Impl::ReadValue<Entity, float>(this->v, entity_ptr);
      });
}

int ValueHolder::GetInt() const {
  return Impl::ApplyToEntity(v.GetParentBranch()->config.GetType(),
                             data_ptr, [this](auto entity_ptr) {
        using Entity = std::remove_const_t<std::remove_pointer_t<decltype(entity_ptr)>>;
        return Impl::ReadValue<Entity, int>(this->v, entity_ptr);
      });
}

bool ValueHolder::GetBool() const {
  return Impl::ApplyToEntity(v.GetParentBranch()->config.GetType(),
                             data_ptr, [this](auto entity_ptr) {
        using Entity = std::remove_const_t<std::remove_pointer_t<decltype(entity_ptr)>>;
        return Impl::ReadValue<Entity, bool>(this->v, entity_ptr);
      });
}
ValueHolder::operator float() const {
  return GetVal();
}
void ValueHolder::SetVal(float val) const {
  v.GetParentBranch()->CheckMutable(true);
  Impl::ApplyToEntity(entity_type, data_ptr, [this, val](auto entity_ptr) {
    Impl::SetValue(v, entity_ptr, val);
  });
}
void ValueHolder::SetVal(int val) const {
  v.GetParentBranch()->CheckMutable(true);
  Impl::ApplyToEntity(entity_type, data_ptr, [this, val](auto entity_ptr) {
    Impl::SetValue(v, entity_ptr, val);
  });
}
void ValueHolder::SetVal(bool val) const {
  v.GetParentBranch()->CheckMutable(true);
  Impl::ApplyToEntity(entity_type, data_ptr, [this, val](auto entity_ptr) {
    Impl::SetValue(v, entity_ptr, val);
  });
}

ValueHolder &ValueHolder::operator=(const ValueHolder &other) {
  v.GetParentBranch()->CheckMutable(true);

  using AnalysisTree::Types;
  if (other.v.GetFieldType() == Types::kFloat) {
    SetVal(other.GetVal());
  } else if (other.v.GetFieldType() == Types::kInteger) {
    SetVal(other.GetInt());
  } else if (other.v.GetFieldType() == Types::kBool) {
    SetVal(other.GetBool());
  } else {
    /* unreachable */
    assert(false);
  }

  return *this;
}
