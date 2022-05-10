#include "../include/simulator.h"
#include <string.h>
#include <vector>
#include <iostream>
using namespace std;

/* **********************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
************************/

/*** STUDENTS WRITE THE NEXT SEVEN ROUTINES ***/

static int base;
static int nxtseq_no;
static int msg_seq;
static int N;
static float RTtime;
static int expctd_seq_no;
static int buffersize;


vector<pkt> buffer;

int chcksum(struct pkt packet){
  int checksum=0;
  int payload_size=0;
  while(payload_size<20)
  { 
    checksum+= packet.payload[payload_size]; 
    payload_size++;
  }
  checksum+=packet.seqnum;
  checksum+=packet.acknum;
  return checksum;

}


struct pkt *make_packt(struct msg message){
  struct pkt *new_packet= new pkt();
  new_packet->seqnum=msg_seq;
  new_packet->acknum=-1; 
  strcpy(new_packet->payload, message.data);
  new_packet->checksum=chcksum(*new_packet);
  return new_packet;
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  cout << "next seq " << nxtseq_no << endl;
    
  if(nxtseq_no<base+N){
    buffer.push_back(*make_packt(message));
    std::cout<<"A new packet Start"<<endl;
    tolayer3(0,buffer[msg_seq]);
    if(base==nxtseq_no){
      starttimer(0,RTtime);
    }
    nxtseq_no++;
  }
  else{
    buffer.push_back(*make_packt(message));
    buffersize++;
  }
  msg_seq++;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  cout << "pkt ack" << packet.acknum << endl;
  if(chcksum(packet)==packet.checksum &&  packet.acknum>=base){
    base=packet.acknum+1;
    stoptimer(0);
    if(base<nxtseq_no){
      cout << "check entry"  << endl;
      starttimer(0,RTtime);
    }
    cout << "next seq" << nxtseq_no << endl;
    int i=nxtseq_no;
    while(i<base+N && buffersize!=0){
      tolayer3(0,buffer[i]);

      if(base==nxtseq_no){
        starttimer(0,RTtime);
        nxtseq_no;
        buffersize--;
        nxtseq_no++;
      }
      i++;
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  starttimer(0,RTtime);
  int i=base;
  while(i<nxtseq_no){
    tolayer3(0,buffer[i]);
    i++;
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  RTtime=20;
  N= getwinsize();
  base=0;
  nxtseq_no=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  cout << "expected seq" << expctd_seq_no << endl;
  if(packet.checksum==chcksum(packet)&& expctd_seq_no==packet.seqnum){
    struct pkt *ackn = new pkt();
    ackn->seqnum=expctd_seq_no;
    ackn->acknum=expctd_seq_no;
    ackn->checksum=chcksum(*ackn);
    tolayer5(1,packet.payload);
    tolayer3(1,*ackn);
    expctd_seq_no++;  
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  expctd_seq_no=0;
}