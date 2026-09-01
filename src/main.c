#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/xattr.h>

#define KEY "user.postit.note"

typedef enum {
  CMD_UNKNOWN,
  CMD_ADD,
  CMD_SHOW,
  CMD_EDIT,
  CMD_RM,
  CMD_HELP
} Command;

Command parse_command(const char *cmd)
{
  if(strcmp(cmd, "add") == 0) return CMD_ADD;
  if(strcmp(cmd, "show") == 0) return CMD_SHOW;
  if(strcmp(cmd, "edit") == 0) return CMD_EDIT;
  if(strcmp(cmd, "rm") == 0) return CMD_RM;
  if(strcmp(cmd, "help") == 0) return CMD_HELP;
  return CMD_UNKNOWN;
}

int validate_optarg(const char *arg)
{
  if (arg == NULL || *arg == '\0') return -1;
  return 0;
}

int handle_add(int argc, char *argv[])
{
  char key[256];
  snprintf(key, sizeof(key), "%s", KEY);
  
  int opt;
  optind = 2;

  opterr = 0;

  while((opt = getopt(argc, argv,"s:")) != -1)
    {
      switch (opt) {
      case 's':
	if(validate_optarg(optarg) < 0)
	  {
	    fprintf(stderr, "%s add: invalid argument for '-s': '%s'\n", argv[0], optarg);
	    return -1;
	  }
	snprintf(key, sizeof(key), "user.postit.%s", optarg);
	break;
      case '?':
	fprintf(stderr, "%s add: invalid option '-%c'\n", argv[0], optopt);
	return -1;
      }
    }

  if (argc - optind < 2)
    {
      fprintf(stderr, "%s add: missing text and target file\n", argv[0]);
      return -1;
    }

  const char *text = argv[optind];
  const char *file = argv[optind + 1];

  if (lsetxattr(file, key, text, strlen(text), XATTR_CREATE) == -1){
    if (errno == EEXIST)
      {
	fprintf(stderr, "%s add: attribute already specified for %s\n", argv[0], file);
      }
    else
      {
	perror(argv[0]);
      }
    return -1;
  };

  /*
  for(int i = 0; i < argc; i++){
    printf("optind:%d, %s\n", optind, argv[i]);}
  */

  return 0;
}

int handle_show(int argc, char *argv[])
{
  int show_all = 0;
  int filter_set = 0;
  char key[256];
  snprintf(key, sizeof(key), "%s", KEY);
  
  int opt;
  optind = 2;


  opterr = 0;
  
  while((opt = getopt(argc, argv,"f:a")) != -1)
    {
      switch (opt) {
      case 'f':
	filter_set = 1;
	if(validate_optarg(optarg) < 0)
	  {
	    fprintf(stderr, "%s show: invalid argument for '-f': '%s'\n", argv[0], optarg);
	    return -1;
	  }
	snprintf(key, sizeof(key), "user.postit.%s", optarg);
	break;
      case 'a':
	show_all = 1;
	break;
      case '?':
	if (optopt == 'f')
	  {
	  fprintf(stderr, "%s show: option '-f' requires an argument\n", argv[0]);
	  }
	else
	  {
	    fprintf(stderr, "%s show: invalid option '-%c'\n", argv[0], optopt);
	  }
	return -1;
      }
    }

  if (show_all && filter_set)
    {
      fprintf(stderr, "%s show: -a and -f are mutually exclusive\n", argv[0]);
      return -1;
    }
  
  if (argc - optind < 1)
    {
      fprintf(stderr, "%s show: missing target file\n", argv[0]);
      return -1;
    }
  
  const char *file = argv[optind];

  ssize_t list_size = listxattr(file, NULL, 0);
  if (list_size < 0)
    {
      perror(argv[0]);
      return -1;
    }
  if (list_size == 0)
    {
      printf("No attributes\n");
      return 0;
    }

  char *list = malloc(list_size);
  if(list == NULL)
    {
      perror(argv[0]);
      return -1;
    }

  ssize_t attribute_names_list = listxattr(file, list, list_size);
  if(attribute_names_list == -1)
    {
      free(list);
      perror(argv[0]);
      return -1;
    }

  char *p = list;
  while(p < list + list_size){
    if (!show_all && strcmp(p, key) != 0)
      {
	p += strlen(p) + 1;
	continue;
      }
    
    ssize_t attribute_size = getxattr(file, p, NULL, 0);
    if (attribute_size == -1)
      {
	free(list);
	perror(argv[0]);
	return -1;
      }

    char *attribute_value = malloc(attribute_size + 1);
    if(attribute_value == NULL)
      {
	free(list);
	perror(argv[0]);
	return -1;
      }

    ssize_t attribute = getxattr(file, p, attribute_value, attribute_size);
    if (attribute == -1)
      {
	free(list);
	free(attribute_value);
	perror(argv[0]);
	return -1;
      }

    attribute_value[attribute_size] = '\0';

    printf("%s: %s\n", p, attribute_value);
    free(attribute_value);
    p += strlen(p) + 1;
  }
  free(list);
  return 0;
}


int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "%s: missing command\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  switch (parse_command(argv[1]))
    {
    case CMD_ADD:
      if (handle_add(argc, argv) < 0) exit(EXIT_FAILURE);
      break;
    case CMD_SHOW:
      if (handle_show(argc, argv) < 0) exit(EXIT_FAILURE);
      break;
    case CMD_EDIT:
      break;
    case CMD_RM:
      break;
    case CMD_HELP:
      break;
    case CMD_UNKNOWN:
      fprintf(stderr, "%s: unknown command '%s'\n", argv[0], argv[1]);
      exit(EXIT_FAILURE);
    }

  return EXIT_SUCCESS;
}
 
