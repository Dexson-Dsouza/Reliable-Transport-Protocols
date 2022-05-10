#include "../include/simulator.h"
#include <vector>
#include <string.h>
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

/* called from layer 5, passed the data to be sent to other side */


static int windowA;
static int send_base;
static int nxtseq_no;

static int windowB;
static int expctd_seq_no;
static int last_timer = -1;

static float RTtime = 35;
vector<pkt> buffer;
static struct pkt RCVbuffer[1010];
vector<int> timer;



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
  new_packet->acknum=-1;
  new_packet->seqnum=nxtseq_no;
  strcpy(new_packet->payload, message.data);
  new_packet->checksum=chcksum(*new_packet);
  return new_packet;
}



void A_output(struct msg message)
{
  buffer.push_back(*make_packt(message));
  timer.push_back(0);
  if(nxtseq_no>=send_base && (nxtseq_no<send_base+windowA)){
    timer[nxtseq_no]=get_sim_time();
    tolayer3(0, buffer[nxtseq_no]);
  }
  if(send_base==nxtseq_no){
    starttimer(0,RTtime);
    last_timer=send_base;
  }
  nxtseq_no++;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  if(packet.checksum==chcksum(packet)&& (packet.acknum<send_base + windowA)&& packet.acknum >= send_base){
   stoptimer(0);

   buffer[packet.seqnum].acknum=true;
   
   int sbase=send_base;
   while(sbase<nxtseq_no){
     if(buffer[sbase].acknum == -1 && get_sim_time() - timer[sbase] < RTtime){
       int time = RTtime - (get_sim_time()-timer[sbase]);
       starttimer(0,time);
       last_timer=sbase;
       break;
     }
     sbase++;
   }

    sbase=send_base;

    while(sbase<nxtseq_no){
      if(get_sim_time() - timer[sbase] > RTtime && buffer[sbase].acknum == -1){
        tolayer3(0,buffer[sbase]);
        timer[sbase]=get_sim_time();
        
      }
      sbase++;
    }

    if(send_base==packet.acknum){
      while(buffer[send_base].acknum!=false){
        send_base++;
        if(buffer[send_base + windowA].acknum == -1 && send_base + windowA < nxtseq_no  ){
          tolayer3(0, buffer[send_base + windowA]);

          timer[send_base + windowA] = get_sim_time();
        }
      }
    }
   } 
  }


/* called when A's timer goes off */
void A_timerinterrupt()
{
  tolayer3(0, buffer[last_timer]);
  timer[last_timer] = get_sim_time();
  int sbase=send_base;

  while(sbase<nxtseq_no){
    if(RTtime>get_sim_time() - timer[sbase] && buffer[sbase].acknum == -1){
      int time = RTtime - (get_sim_time() - timer[sbase]);
      starttimer(0, time);
      last_timer = sbase;
      break;
      sbase++;
    }
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  send_base=0;
  nxtseq_no=0;
  windowA=getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  struct pkt *ackn = new pkt();
  ackn->acknum = packet.seqnum;
  ackn->seqnum = packet.seqnum;
  ackn->checksum = chcksum(*ackn);

  if(packet.checksum == chcksum(packet) && packet.seqnum >= 0 && packet.seqnum < expctd_seq_no){
    tolayer3(1, *ackn);
  }
  else if(chcksum(packet)==packet.checksum &&   expctd_seq_no + windowB > packet.seqnum  && expctd_seq_no<=packet.seqnum ){
    tolayer3(1, *ackn);
    RCVbuffer[packet.seqnum] = packet;
    RCVbuffer[packet.seqnum].acknum = 1; 

    if(packet.seqnum==expctd_seq_no){
      while(RCVbuffer[expctd_seq_no].acknum == 1){
        tolayer5(1,RCVbuffer[expctd_seq_no].payload);
        expctd_seq_no++;

      }

    }
  }
  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  expctd_seq_no=0;
  windowB=getwinsize();
  int size=0;
  while(size<1010){
    RCVbuffer[size].acknum=0;
    RCVbuffer[size].seqnum=0;
    memset(RCVbuffer[size].payload,0, sizeof(RCVbuffer[size].payload));
    size++;
  }
}