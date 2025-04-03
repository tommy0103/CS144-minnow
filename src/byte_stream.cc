#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), size_( 0 ), read_amount_( 0 ), write_amount_( 0 ), buffer()
{}

void Writer::push( string data )
{
  // (void)data; // Your code here.
  if ( data.size() == 0 || available_capacity() == 0 )
    return;
  if ( data.size() + size_ <= capacity_ ) {
    buffer.push( data );
    write_amount_ += data.size();
    size_ += data.size();
  } else {
    uint64_t length = available_capacity();
    buffer.push( data.substr( 0, length ) );
    write_amount_ += length;
    size_ += length;
  }
}

void Writer::close()
{
  this->closed_ = true;
}

bool Writer::is_closed() const
{
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - size_;
}

uint64_t Writer::bytes_pushed() const
{
  return write_amount_;
}

string_view Reader::peek() const
{
  return buffer.front();
}

void Reader::pop( uint64_t len )
{
  while ( buffer.size() && len > 0 ) {
    string& str = buffer.front();
    if ( len < str.size() ) {
      str = str.substr( len );
      read_amount_ += len;
      size_ -= len;
      break;
    } else {
      len -= str.size();
      read_amount_ += str.size();
      size_ -= str.size();
      buffer.pop();
    }
  }
}

bool Reader::is_finished() const
{
  return buffer.empty() && closed_;
}

uint64_t Reader::bytes_buffered() const
{
  return size_; // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return read_amount_; // Your code here.
}
