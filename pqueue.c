#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h" // for log()
#include "pqueue.h"

static pqueue_t *pq_head = NULL, *pq_tail = NULL;

int pqueue_add (int seq, unsigned char *packet, int packlen) {
  pqueue_t *newent, *point;

  newent = (pqueue_t *)calloc(1, sizeof(pqueue_t));
  if (!newent) {
    log("error allocating newent: %s", strerror(errno));
    return -1;
  }

  newent->packet = (unsigned char *)malloc(packlen);
  if (!newent->packet) {
    log("error allocating packet: %s", strerror(errno));
    return -1;
  }

  memcpy(newent->packet, packet, packlen);
  newent->seq     = seq;
  newent->packlen = packlen;
  newent->expires = time(NULL) + MISSING_TIMEOUT;
  
  for (point = pq_head; point != NULL; point = point->next) {
    if (point->seq == seq) {
      // queue already contains this packet
      log("discarding duplicate packet %d", seq);
      return -1;
    }
    if (point->seq > seq) {
      // gone too far: point->seq > seq and point->prev->seq < seq
      if (point->prev) {
	// insert between point->prev and point
	log("adding %d between %d and %d", 
	    seq, point->prev->seq, point->seq);
	point->prev->next = newent;
      } else {
	// insert at head of queue, before point
	log("adding %d before %d", seq, point->seq);
	pq_head = newent;
      }
      newent->prev = point->prev; // will be NULL, at head of queue
      newent->next = point;
      point->prev = newent;
      return 0;
    }
  }

  /* We didn't find anywhere to insert the packet,
   * so there are no packets in the queue with higher sequences than this one,
   * so all the packets in the queue have lower sequences,
   * so this packet belongs at the end of the queue (which might be empty)
   */
  
  if (pq_head == NULL) {
    log("adding %d to empty queue", seq);
    pq_head = newent;
  } else {
    log("adding %d as tail, after %d", seq, pq_tail->seq);
    pq_tail->next = newent;
  }
  newent->prev = pq_tail;
  pq_tail = newent;

  return 0;
}

int pqueue_del (pqueue_t *point) {
  if (pq_head == point) pq_head = point->next;
  if (pq_tail == point) pq_tail = point->prev;
  if (point->prev) point->prev->next = point->next;
  if (point->next) point->next->prev = point->prev;
  free(point->packet);
  free(point);
  return 0;
}

pqueue_t *pqueue_head () {
  return pq_head;
}
