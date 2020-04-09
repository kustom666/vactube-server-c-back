#ifndef VACTUBE_HANDLERS
#define VACTUBE_HANDLERS

#define VACTUBE_JOIN_MESSAGE 1
#define VACTUBE_LEAVE_MESSAGE 2

#include <assert.h>
#include <search.h>
#include <gmodule.h>
#include <libdill.h>
#include "config.h"

typedef struct ch_message_t
{
  int sender;
  int message_size;
  uint8_t *recv_buffer;
  size_t recv_buffer_size;
} ch_message_t;

typedef struct state_t
{
  GSList *client_list;
} state_t;

ch_message_t *ch_message_new(size_t buffer_len);
void ch_message_free(ch_message_t *message);
coroutine void state_handler(int join_rx, int leave_rx);
coroutine void sender_handler(int sender_chan_rx);
coroutine void receiver_handler(int s, int sender_chan_tx, int join_chan_tx);
#endif