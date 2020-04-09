#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "handlers.h"

int main(void)
{
  // Setup channels
  int ch_join[2]; // 0 is rx, 1 is tx
  int rc = chmake(ch_join);
  assert(rc == 0);

  int ch_leave[2]; // 0 is rx, 1 is tx
  rc = chmake(ch_leave);
  assert(rc == 0);

  int ch_sender[2]; // 0 is rx, 1 is tx
  rc = chmake(ch_sender);
  assert(rc == 0);

  int b = bundle();
  assert(b >= 0);

  // Setup TCP stack
  struct ipaddr addr;
  rc = ipaddr_local(&addr, NULL, 8081, 0);
  assert(rc == 0);
  int ls = tcp_listen(&addr, 10);
  assert(ls >= 0);

  rc = bundle_go(b, state_handler(ch_join[0], ch_leave[0]));
  assert(rc == 0);

  rc = bundle_go(b, sender_handler(ch_sender[0]));
  assert(rc == 0);

  // TCP + WS event loop
  while (1)
  {
    int s = tcp_accept(ls, NULL, -1);
    assert(s >= 0);

    assert(rc == 0);
    rc = bundle_go(b, receiver_handler(s, ch_sender[1], ch_join[1]));
    assert(rc == 0);
  }

  hclose(b);
}