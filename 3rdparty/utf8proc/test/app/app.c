#include <stdio.h>
#include <utf8proc.h>

int
main(void)
{
  printf("%s\n", utf8proc_version());
  return 0;
}
