#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <sstream>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  // cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  // cerr << "Warning: get_URL() has not been implemented yet.\n";
  TCPSocket socket;
  Address address( host, "http" );
  // cout << address.to_string() << endl;
  socket.connect( address );
  // vector<Ref<string>> buffer;
  ostringstream oss;
  oss << "GET " << path << " HTTP/1.1" << "\r\n";
  oss << "Host: " << host << "\r\n";
  oss << "Connection: close" << "\r\n\r\n";
  string line = oss.str();
  // cout << line << endl;
  // buffer.push_back(Ref<string>(std::move(line)));
  // socket.write(buffer);
  socket.write( line );
  // cout << socket.write_count() << endl;
  while ( !socket.eof() ) {
    socket.read( line );
    cout << line;
  }
  socket.close();
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
