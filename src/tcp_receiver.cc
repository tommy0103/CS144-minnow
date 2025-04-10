#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.SYN) {
    zero_point_ = Wrap32::wrap(0, message.seqno);
    is_connection_started_ = true;
    is_connection_ended_ = false;
  }
  if(message.RST) {
    reassembler_.reader().set_error();
    return;
  }
  if(message.FIN) {
    is_connection_ended_ = true;
  }

  if(!is_connection_started_) {
    return;
  }
  
  uint64_t absolute_index = message.seqno.unwrap(zero_point_, check_point_);
  if(!message.SYN) {
    if(absolute_index == 0) {
      return;
    }
    absolute_index--;
  }
  const uint64_t SEQUENCE_SPACE_SIZE = 1ull << 32;
  if(absolute_index - check_point_ >= SEQUENCE_SPACE_SIZE) {
    check_point_ += SEQUENCE_SPACE_SIZE;
  }
  reassembler_.insert(absolute_index, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage response;
  if(reassembler_.writer().has_error()) {
    response.RST = true;
    return response;
  }
  
  if(is_connection_started_) {
    uint64_t bytes_received = reassembler_.writer().bytes_pushed();
    uint64_t ackno_value = bytes_received + 1;
    bool all_bytes_reassembled = (reassembler_.count_bytes_pending() == 0);
    if(all_bytes_reassembled && is_connection_ended_) {
      ackno_value++;
    }
    
    response.ackno = Wrap32::wrap(ackno_value, zero_point_);
  }
  const uint64_t MAX_WINDOW_SIZE = (1 << 16) - 1;
  uint64_t available_capacity = reassembler_.writer().available_capacity();
  
  response.window_size = min(available_capacity, MAX_WINDOW_SIZE);
  return response;
}
