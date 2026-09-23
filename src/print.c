#include "print.h"
#include "stdio.h"

void
print_add_help (char *prog)
{
  printf("Usage: %s add [-s KEY] filename\n", prog);
}

void
print_edit_help (char *prog)
{
  printf("Usage: %s edit [-s KEY] [-e EDITOR] filename\n", prog);
}

void
print_help_help (char *prog)
{
  printf("Usage: %s help command\n", prog);
}

void
print_rm_help (char *prog)
{
  printf("Usage: %s rm [-s KEY] filename\n", prog);
}

void
print_show_help (char *prog)
{
  printf("Usage: %s show [-f KEY | -a] filename\n", prog);
}

void
print_usage (char *prog)
{
  printf("Usage: %s command options\n", prog);
}

