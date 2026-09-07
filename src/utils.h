#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/xattr.h>
#include <unistd.h>

#define _cleanup_(x) __attribute__((__cleanup__(x)))

#define KEY "user.postit.note"
#define DEFAULT_EDITOR "vim"

static inline void cleanup(void *p)
{
  void **ptr = p;
  free(*ptr);
}

static inline void close_file(FILE **fp)
{
  if(fp && *fp) fclose(*fp);
}

static inline void unlink_temp(char **path)
{
  if(path && *path)
    {
      unlink(*path);
      free(*path);
    }
}

typedef enum {
  CMD_UNKNOWN,
  CMD_ADD,
  CMD_SHOW,
  CMD_EDIT,
  CMD_RM,
  CMD_HELP
} Command;


Command parse_command(const char *cmd);
int validate_optarg(const char *arg);
