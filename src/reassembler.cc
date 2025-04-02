#include "reassembler.hh"
#include "debug.hh"

using namespace std;

template<typename Map>
Map::iterator insertMap(uint64_t idx, std::string& d, Map& mp) {
  auto it = mp.find(idx);
  if(it != mp.end()) {
    if(it->second.size() < d.size()) {
      it->second = d;
    }
  }
  else {
    auto [it_, _] = mp.insert({idx, d});
    it = it_;
  }
  return it;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  Writer& w = output_.writer();
  Reader& r = output_.reader();
  uint64_t capacity_ = w.available_capacity() + r.bytes_popped();
  if(capacity_ < first_index) return;
  uint64_t delta = 0;
  if(pendingId > first_index) {
    delta = pendingId - first_index;
  }
  if(is_last_substring) {
    overId = first_index + data.size();
  }
  if(data.size() > 0) {
    if(delta > data.size()) {
      return;
    }
    data = data.substr(delta, data.size());
    if(first_index + data.size() > capacity_) { // (first_index + data.size().old - delta)
      data = data.substr(0, data.size() - (first_index + data.size() - capacity_)); 
    }
  }

  {// Coalesce data segments to eliminate overlaps
    uint64_t st = first_index + delta, ed = st + data.size();
    auto it = insertMap(first_index + delta, data, pendingBytes);
    
    auto it1 = it;
    if(it1 != pendingBytes.begin()) {
      --it1; // idx < st
      auto& [idx, d] = *it1;
      if(idx + d.size() > st) {
        if(idx + d.size() >= ed) {
          pendingBytes.erase(it);
          return;
        }// idx + d.size() < ed
        d = d.substr(0, d.size() - (idx + d.size() - st));
      }
    }
    auto it2 = it;
    ++it2; // idx > st
    while(it2 != pendingBytes.end()) {
      auto& [idx, d] = *it2;
      if(ed > idx) {
        if(ed >= idx + d.size()) {
          it2 = pendingBytes.erase(it2);
        }
        else {// ed < idx + d.size()
          auto nidx = ed;
          auto nd = d.substr(ed - idx, d.size());
          it2 = pendingBytes.erase(it2);
          insertMap(nidx, nd, pendingBytes);
          break;
        }
      }
      else {
        break;
      }
    }
  }
  
  // Check data can be sended.
  for(auto it = pendingBytes.begin(); it != pendingBytes.end(); ) {
    auto& [idx, d] = *it;
    if(idx > pendingId) {
      break;
    }
    if(idx + d.size() == overId) {
      w.close();
    }
    if(idx + d.size() <= pendingId) {
      it = pendingBytes.erase(it);
      continue;
    }
    // pendingId - idx > d.size();
    if(idx < pendingId) {
      d = d.substr(pendingId - idx, d.size());
    }
    pendingId += d.size();
    w.push(d);
    it = pendingBytes.erase(it);
  }
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
