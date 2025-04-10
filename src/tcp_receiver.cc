#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.SYN) {
    zero_point_ = Wrap32::wrap(0, message.seqno);
    SYN_ = true;
    FIN_ = false;
  }
  if(message.FIN) {
    FIN_ = true;
  }
  if(message.RST) {
    reassembler_.reader().set_error();
    return;
  }
  uint64_t index = message.seqno.unwrap(zero_point_, check_point_);
  if(!message.SYN) {
    if(index == 0) {
      return;
    } 
    index = index - 1;
  }
  if(index - check_point_ >= (1ull << 32)) {
    check_point_ += (1ull << 32);
  }
  reassembler_.insert(index, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg;
  if(reassembler_.writer().has_error()) {
    msg.RST = true;
    return msg;
  }
  uint64_t bytes_received = reassembler_.writer().bytes_pushed();
  if(SYN_) {
    if(reassembler_.count_bytes_pending() == 0) {
      msg.ackno = Wrap32::wrap(bytes_received, zero_point_) + SYN_ + FIN_;
    }
    else {
      msg.ackno = Wrap32::wrap(bytes_received, zero_point_) + SYN_;
    }
  }
  uint64_t bytes_available = reassembler_.writer().available_capacity();
  if(bytes_available >= (1 << 16)) bytes_available = (1 << 16) - 1;
  msg.window_size = bytes_available;
  return msg;
}
