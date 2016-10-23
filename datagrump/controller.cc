#include <iostream>
#include <set>
#include <queue>
#include <set>
#include <ctime>
#include <math.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

float i = 10;
std::priority_queue< pair<int,int>, std::vector<pair<int,int>>, std::greater<pair<int,int>>> outstandingPackets;
std::set<int> receivedPackets;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = i;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{

  outstandingPackets.push(std::make_pair(send_timestamp, sequence_number));

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  cerr << i << endl;

  // AIMD
  receivedPackets.insert(sequence_number_acked);
  std::pair<int,int> oldestPacket;
  bool is_in;
  oldestPacket = outstandingPackets.top();
  outstandingPackets.pop();
  is_in = receivedPackets.find(oldestPacket.second) != receivedPackets.end();
  while(is_in) {
    receivedPackets.erase(oldestPacket.second);
    oldestPacket = outstandingPackets.top();
    outstandingPackets.pop();
    is_in = receivedPackets.find(oldestPacket.second) != receivedPackets.end();
  };

  if(timestamp_ack_received - oldestPacket.first > timeout_ms()) {
    i *= .7;
  }
  else {
    i += 1.2/i;
  }
  if(i<2){
    i=2;
  }

  // Delay Scheme
  /*if(i<2){
    i=2;
  }
  if(timestamp_ack_received-send_timestamp_acked > 300) {
    i /= 2.1;
  }
  else {
    i += .08/i;
  }*/
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
