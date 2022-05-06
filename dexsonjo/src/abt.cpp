
#include "../include/simulator.h"
#include <string.h>
#include <queue>
#include <vector>
using namespace std;
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */

static int seq1;
static int global_ack = 1;
static int seq2;
float timeout = 4.0;
vector<msg> buffer;
struct pkt current;

int checksum(struct pkt packet)
{
  int check_s = 0;
  for (int i = 0; i < 20; i++)
  {
    check_s += packet.payload[i];
  }
  check_s += packet.seqnum;
  check_s += packet.acknum;
  return check_s;
}

struct pkt *make_packet(struct msg message)
{
  struct pkt *new_pkt = new pkt();
  new_pkt->seqnum = seq1;
  new_pkt->acknum = -1;
  strcpy(new_pkt->payload, message.data);
  new_pkt->checksum = checksum(*new_pkt);
  return new_pkt;
}

void A_output(struct msg message)
{
  if (global_ack == 1)
  {
    if (buffer.empty())
    {
      global_ack = 0;
      current = *make_packet(message);
      tolayer3(0, current);
      starttimer(0, timeout);
    }
    else
    {
      global_ack = 0;
      current = *make_packet(buffer[0]);
      buffer.erase(buffer.begin());
      buffer.push_back(message);
      tolayer3(0, current);
      starttimer(0, timeout);
    }
  }
  else
  {
    buffer.push_back(message);
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  if (packet.acknum == seq1)
  {
    global_ack = 1;
    seq1 = 1 - seq1; // flip sequence
    stoptimer(0);
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{

  tolayer3(0, current);
  starttimer(0, timeout);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  global_ack = 1;
  seq1 = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  struct pkt *ack = new pkt();
  ack->acknum = packet.seqnum;
  ack->seqnum = packet.seqnum;
  ack->checksum = checksum(*ack);
  if (checksum(packet) != packet.checksum)
  {
    return;
  }
  if (seq2 == packet.seqnum)
  {
    tolayer5(1, packet.payload);
    tolayer3(1, *ack);
    seq2 = 1 - seq2; // flip sequence
  }
  else if (seq2 != packet.seqnum)
  {
    tolayer3(1, *ack);
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  seq2 = 0;
}