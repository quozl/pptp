#ifndef PQUEUE_H
#define PQUEUE_H

#include <time.h>

/* wait this many seconds for missing packets before forgetting about them */
#define MISSING_TIMEOUT 5

/* assume packet is bad/spoofed if it's more than this many seqs ahead */
#define MISSING_WINDOW 50

/* Packet queue structure: linked list of packets received out-of-order */
typedef struct pqueue {
  struct pqueue *next;
  struct pqueue *prev;
  int seq;
  time_t expires;
  unsigned char *packet;
  int packlen;
} pqueue_t;

int       pqueue_add  (int seq, unsigned char *packet, int packlen);
int       pqueue_del  (pqueue_t *point);
pqueue_t *pqueue_head ();

#endif /* PQUEUE_H */
