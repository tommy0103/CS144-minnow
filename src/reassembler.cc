#include "reassembler.hh"
#include "debug.hh"
#include <iostream>
#include <assert.h>

using namespace std;

template<typename Map>
auto insert_map(uint64_t idx, const std::string& d, Map& mp) {
  auto it = mp.find(idx);
  if(it != mp.end()) {
    if(it->second.size() < d.size()) {
      it->second = d;
    }
    else {
      return std::make_pair(it, false);
    }
  }
  else {
    auto [it_, _] = mp.insert({idx, d});
    it = it_;
  }
  return std::make_pair(it, true);
}
auto Reassembler::truncate_segment(uint64_t first_index, std::string& data) {
  uint64_t offset = 0;
  if(pendingId > first_index) {
    offset = pendingId - first_index;
  }
  // if(data.size() > 0) {
  if(data.size() == 0) {
    return std::make_pair(offset, true);
  }
  if(offset > data.size()) {
    return std::make_pair(offset, false);
  }
  if(offset != 0) {
    data = data.substr(offset);
  }

  // pendingId - r.bytes_popped() == w.available_capacity()
  // [pendingId, first_index + delta + data.size()]
  uint64_t capacity_ = output_.writer().available_capacity();
  uint64_t segment_size = first_index + offset + data.size() - pendingId;
  uint64_t remove_size = segment_size - capacity_;
  if(segment_size > capacity_) { // (first_index + data.size().old - delta)
    if(remove_size > data.size()) {
      return std::make_pair(offset, false);
    }
    data = data.substr(0, data.size() - remove_size); 
  }
  return std::make_pair(offset, true);
}
void Reassembler::coalesce_segment(uint64_t first_index, const std::string& data) {
  uint64_t st = first_index, ed = st + data.size();
  auto [it, succ_] = insert_map(first_index, data, pendingBytes);
  if(succ_ == false) {return;}
    
  auto frontSegment = it;
  if(frontSegment != pendingBytes.begin()) {
    --frontSegment; // idx < st
    auto& [index, seg] = *frontSegment;
    if(index + seg.size() > st) {
      if(index + seg.size() >= ed) {
        pendingBytes.erase(it);
        return;
      }// idx + d.size() < ed
      int overlap = index + seg.size() - st;
      seg = seg.substr(0, seg.size() - overlap);
    }
  }
  auto behindSegment = it;
  ++behindSegment; // idx > st
  while(behindSegment != pendingBytes.end()) {
    auto& [index, seg] = *behindSegment;
    if(ed > index) {
      if(ed >= index + seg.size()) {
        behindSegment = pendingBytes.erase(behindSegment);
      }
      else {// ed < idx + d.size()
        auto new_index = ed;
        auto new_seg = seg.substr(ed - index);
        // auto new_seg = seg.substr(ed - index, seg.size() - (ed - index));
        insert_map(new_index, new_seg, pendingBytes);
        behindSegment = pendingBytes.erase(behindSegment);
        break;
      }
    }
    else {
      break;
    }
  }
}

// just for assert, not used
void Reassembler::check_coalesce_segment() {
  if(pendingBytes.empty()) return;
  uint64_t pre_st = 0, pre_ed = 0;
  bool is_first_substring = true;
  for(auto &[idx, d] : pendingBytes) {
    uint64_t st = idx, ed = idx + d.size();
    if(!((pre_st < st && pre_ed <= st) || is_first_substring)) {
        // std::cerr << "Error: Overlapping segments detected!" << std::endl;
      throw exception();    
    }
    is_first_substring = false;
    pre_st = st; pre_ed = ed;
  }
}

void Reassembler::push_continuous_segment(Writer &w) {
  for(auto it = pendingBytes.begin(); it != pendingBytes.end(); ) {
    auto& [index, seg] = *it;
    if(index < pendingId) {
      __throw_domain_error("idx < pendingId");
    }
    if(index != pendingId) {
      break;
    }
    pendingId += seg.size();
    w.push(seg);
    if(index + seg.size() == overId) {
      w.close();
    }
    it = pendingBytes.erase(it);
  }
}
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  Writer& w = output_.writer();
  // Don't do this.
  // Reader& r = output_.reader();
  // uint64_t capacity_ = w.available_capacity() + r.bytes_popped();
  // if(capacity_ < first_index) return;
  
  if(is_last_substring) {
    overId = first_index + data.size();
  }

  auto [offset, succ_] = truncate_segment(first_index, data);
  if(succ_ == false) return;

  // Coalesce data segments to eliminate overlaps
  coalesce_segment(first_index + offset, data);
  // check_coalesce_data(); // No Overlapped Segment.
  
  // Check data can be sended.
  push_continuous_segment(w);
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t bytesCount = 0;
  for(auto& [idx, d]: pendingBytes) {
    bytesCount += d.size();
  }
  return bytesCount;
}
