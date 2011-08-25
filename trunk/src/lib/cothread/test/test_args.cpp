/*****
 * cothread parameterized function example
 *****
 * entry point to cothreads cannot take arguments.
 * this is due to portability issues: each processor,
 * operating system, programming language and compiler
 * can use different parameter passing methods, so
 * arguments to the cothread entry points were omitted.
 *
 * however, the behavior can easily be simulated by use
 * of a specialized co_switch to set global parameters to
 * be used as function arguments.
 *
 * in this way, with a bit of extra red tape, one gains
 * even more flexibility than would be possible with a
 * fixed argument list entry point, such as void (*)(void*),
 * as any number of arguments can be used.
 *
 * this also eliminates race conditions where a pointer
 * passed to co_create may have changed or become invalidated
 * before call to co_switch, as said pointer would now be set
 * when calling co_switch, instead.
 *****/

#include "test.h"

cothread_t thread[3];

namespace co_arg {
  int param_x;
  int param_y;
};

//one could also call this co_init or somesuch if they preferred ...
void co_switch(cothread_t thread, int param_x, int param_y) {
  co_arg::param_x = param_x;
  co_arg::param_y = param_y;
  co_switch(thread);
}

void co_entrypoint() {
int param_x = co_arg::param_x;
int param_y = co_arg::param_y;
  printf("co_entrypoint(%d, %d)\n", param_x, param_y);
  co_switch(thread[0]);

//co_arg::param_x will change here (due to co_switch(cothread_t, int, int) call changing values),
//however, param_x and param_y will persist as they are thread local

  printf("co_entrypoint(%d, %d)\n", param_x, param_y);
  co_switch(thread[0]);
  throw;
}

int main() {
  printf("cothread parameterized function example\n\n");

  thread[0] = co_active();
  thread[1] = co_create(65536, co_entrypoint);
  thread[2] = co_create(65536, co_entrypoint);

//use specialized co_switch(cothread_t, int, int) for initial co_switch call
  co_switch(thread[1], 1, 2);
  co_switch(thread[2], 4, 8);

//after first call, entry point arguments have been initialized, standard
//co_switch(cothread_t) can be used from now on
  co_switch(thread[2]);
  co_switch(thread[1]);

  printf("\ndone\n");
#if defined(_MSC_VER) || defined(__DJGPP__)
  getch();
#endif
  return 0;
}
