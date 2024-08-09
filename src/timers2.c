#include "timers2.h"
#include "heap.h"
#include <limits.h>

/* get timer struct pointer from the heap node */
#define pnd_timer_unwrap(ptr) \
  (ptr == NULL) ? NULL : container_of(ptr, pnd_timer_t, hnode)

/* min-heap - answers question: should swap? */
int pnd_timers_comparator(struct heap_node *a, 
                          struct heap_node *b) {
  pnd_timer_t *child = pnd_timer_unwrap(a);
  pnd_timer_t *parent = pnd_timer_unwrap(b);
  
  return child->timeout < parent->timeout;
}

void pnd_timers_heap_init(pnd_io_t *ctx)
{
  heap_init(&ctx->timers, pnd_timers_comparator);  
}

void pnd_timer_init(pnd_io_t *ctx, pnd_timer_t *timer)
{
  timer->ctx = ctx;
  timer->timeout = 0;
  timer->interval = 0;
  timer->active = false;
  timer->data = NULL;
  timer->on_timeout = NULL;
  heap_init_node(&timer->hnode);
}

void pnd_timer_start(pnd_timer_t *timer, 
                     pnd_timer_callback_t cb, 
                     uint64_t timeout) {
  if (timer->active) {
    return;  
  }

  timer->active = true;
  timer->timeout = timer->ctx->now + timeout;
  timer->on_timeout = cb;
  heap_insert(&timer->ctx->timers, &timer->hnode);
}

void pnd_timer_repeat(pnd_timer_t *timer, 
                      pnd_timer_callback_t cb, 
                      uint64_t interval) {
  timer->interval = interval;
  pnd_timer_start(timer, cb, interval);
}

void pnd_timer_stop(pnd_timer_t *timer)
{
  if (!timer->active) {
    return;
  }

  timer->active = false;
  heap_remove(&timer->ctx->timers, &timer->hnode);
}

void pnd_timers_run(pnd_io_t *ctx) {
  pnd_timer_t *min;

  while(true) {
    min = pnd_timer_unwrap(heap_peek(&ctx->timers));
    if (min == NULL)
      break;

    if (min->timeout > ctx->now)
      break;

    pnd_timer_stop(min);
    min->on_timeout(min);

    if (min->interval != 0)
      pnd_timer_start(min, min->on_timeout, min->interval);
  }
}

// return next timeout - useful for epoll
int pnd_timers_next(pnd_io_t *ctx) {
  pnd_timer_t *min = pnd_timer_unwrap(heap_peek(&ctx->timers));
  if (min == NULL)
    return -1;

  uint64_t next_timeout = min->timeout - ctx->now;
  return (next_timeout >= INT_MAX) ? INT_MAX : next_timeout;
}