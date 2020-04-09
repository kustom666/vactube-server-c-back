#include "handlers.h"

static state_t state;

ch_message_t *ch_message_new(size_t buffer_len)
{
  ch_message_t *message = malloc(sizeof(ch_message_t));
  message->recv_buffer_size = sizeof(uint8_t) * buffer_len;
  message->recv_buffer = malloc(message->recv_buffer_size);
  return message;
}

void ch_message_free(ch_message_t *message)
{
  free(message->recv_buffer);
  free(message);
}

coroutine void state_handler(int join_rx, int leave_rx)
{
  int recv_join, recv_leave;
  struct chclause clauses[] =
      {
          {CHRECV, join_rx, &recv_join, sizeof(recv_join)},
          {CHRECV, leave_rx, &recv_leave, sizeof(recv_leave)}};

  while (1)
  {
    int rc = choose(clauses, 2, -1);
    if (rc == -1)
    {
      ZF_LOGE("The state handler failed to receive a join or leave");
    }
    else if (rc == 0)
    {
      int *s_copy = malloc(sizeof(int));
      *s_copy = recv_join;
      state.client_list = g_slist_prepend(state.client_list, s_copy);
    }
    else if (rc == 1)
    {
      GSList *found_list = g_slist_find(state.client_list, &recv_leave);
      free(found_list->data);
      state.client_list = g_slist_remove(state.client_list, &recv_leave);
    }
  }
}

void clients_foreach(int *s, ch_message_t *message)
{
  if (*s != message->sender)
  {
    int rc = msend(*s, message->recv_buffer, message->message_size, -1);
    if (rc != 0)
    {
      ZF_LOGE("Error sending to client %d: %d", *s, errno);
    }
  }
}

coroutine void sender_handler(int sender_chan_rx)
{
  ch_message_t *message = ch_message_new(VAC_MAX_MESSAGE_LEN);
  while (1)
  {
    int rc = chrecv(sender_chan_rx, message, sizeof(message), -1);
    assert(rc == 0);
    g_slist_foreach(state.client_list, (GFunc)clients_foreach, message);
  }
  ch_message_free(message);
}

coroutine void receiver_handler(int s, int sender_chan_tx, int join_chan_tx)
{
  char resource[256];
  char host[256];
  ch_message_t *message = ch_message_new(VAC_MAX_MESSAGE_LEN);
  s = ws_attach_server(s, WS_TEXT, resource, sizeof(resource),
                       host, sizeof(host), -1);
  assert(s >= 0);

  int rc = chsend(join_chan_tx, &s, sizeof(s), -1);
  while (1)
  {
    rc = mrecv(s, message->recv_buffer, message->recv_buffer_size, -1);
    assert(rc != -1);
    message->sender = s;
    message->message_size = rc;
    ZF_LOGI("received: from: %d - %s", s, message->recv_buffer);
    rc = chsend(sender_chan_tx, message, sizeof(message), -1);
    assert(rc == 0);
  }

  s = ws_detach(s, 1000, "OK", 2, -1);
  assert(s >= 0);

  /* Close the TCP connection. */
  rc = tcp_close(s, -1);
  assert(rc == 0);
  ch_message_free(message);
}
