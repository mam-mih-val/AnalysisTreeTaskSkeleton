//
// Created by eugene on 20/02/2021.
//

#ifndef ATTASKSKELETON_AT_TASK_ATI2_ATHELPER_HPP_
#define ATTASKSKELETON_AT_TASK_ATI2_ATHELPER_HPP_

#include <AnalysisTree/EventHeader.hpp>
#include <AnalysisTree/Track.hpp>

namespace ATI2 {

namespace ATHelper {

template<typename Entity, typename Value>
void SetField(Entity *data_ptr, short field_id, Value value) {
  data_ptr->SetField(value, field_id);
};

template<typename Value>
void SetField(AnalysisTree::EventHeader *event_header, short field_id, Value value) {
  using AnalysisTree::EventHeaderFields::EventHeaderFields;
  if (field_id >= 0) {
    event_header->template SetField(value, field_id);
  } else {
    if (field_id == EventHeaderFields::kVertexX) {
      event_header->SetVertexPosition3({value, event_header->GetVertexY(), event_header->GetVertexZ()});
    } else if (field_id == EventHeaderFields::kVertexY) {
      event_header->SetVertexPosition3({event_header->GetVertexX(), value, event_header->GetVertexZ()});
    } else if (field_id == EventHeaderFields::kVertexZ) {
      event_header->SetVertexPosition3({event_header->GetVertexX(), event_header->GetVertexY(), value});
    } else {
      throw std::runtime_error("Invalid field id");
    }
  }
}

template<typename Value>
void SetField(AnalysisTree::Track *track, short field_id, Value value) {
  using AnalysisTree::TrackFields::TrackFields;
  if (field_id >= 0) {
    track->template SetField(value, field_id);
  } else {
    switch (field_id) {
      case TrackFields::kPx: track->SetMomentum3({value, track->GetPy(), track->GetPz()}); break;
      case TrackFields::kPy: track->SetMomentum3({track->GetPx(), value, track->GetPz()}); break;
      case TrackFields::kPz: track->SetMomentum3({track->GetPx(), track->GetPy(), value}); break;
      case TrackFields::kP:
      case TrackFields::kPt:
      case TrackFields::kEta:
      case TrackFields::kPhi: /* transient fields, ignoring */ break;
      default: throw std::runtime_error("Unknown field");
    }

  }

}


}

}

#endif //ATTASKSKELETON_AT_TASK_ATI2_ATHELPER_HPP_
