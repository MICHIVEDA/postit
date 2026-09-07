#include "utils.h"

Command parse_command(const char *cmd)
{
  if(strcmp(cmd, "add") == 0) return CMD_ADD;
  if(strcmp(cmd, "show") == 0) return CMD_SHOW;
  if(strcmp(cmd, "edit") == 0) return CMD_EDIT;
  if(strcmp(cmd, "rm") == 0) return CMD_RM;
  if(strcmp(cmd, "help") == 0) return CMD_HELP;
  return CMD_UNKNOWN;
}

/* for options with arguments check that argument exists and is not empty. */ 
int validate_optarg(const char *arg)
{
  if (arg == NULL || *arg == '\0') return -1;
  return 0;
}
