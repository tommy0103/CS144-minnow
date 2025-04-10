#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  // return Wrap32 { 0 };
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // // Your code here.
  // debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  // return {};
  uint64_t real_value = raw_value_ - zero_point.raw_value_;
  if(real_value < checkpoint) {
    uint64_t div1 = (checkpoint - real_value) / (1ull << 32);
    uint64_t div2 = (checkpoint - real_value - 1) / (1ull << 32) + 1;
    uint64_t expected_value1 = real_value + div1 * (1ull << 32);
    uint64_t expected_value2 = real_value + div2 * (1ull << 32);
    // real_value += div1 * (1ull << 32); 
    if(checkpoint - expected_value1 < expected_value2 - checkpoint) {
      real_value = expected_value1;
    }
    else {
      real_value = expected_value2;
    }
  }
  return real_value;
}
