#include "utils.h"

/* add extended attribute.
   <-s param> to override with custom key
   postit add -s todo "text" file
 */
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

/* show extended attributes
   <-f param> filter attributes with key e.g -f todo.
   <-a param> show all not just from postit.
   default is user.postit.note
 */

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

  _cleanup_(cleanup) char *list = malloc(list_size);
  if(list == NULL)
    {
      perror(argv[0]);
      return -1;
    }

  ssize_t attribute_names_list = listxattr(file, list, list_size);
  if(attribute_names_list == -1)
    {
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
	perror(argv[0]);
	return -1;
      }

    _cleanup_(cleanup) char *attribute_value = malloc(attribute_size + 1);
    if(attribute_value == NULL)
      {
	perror(argv[0]);
	return -1;
      }

    ssize_t attribute = getxattr(file, p, attribute_value, attribute_size);
    if (attribute == -1)
      {
	perror(argv[0]);
	return -1;
      }

    attribute_value[attribute_size] = '\0';

    printf("%s: %s\n", p, attribute_value);
    p += strlen(p) + 1;
  }
  return 0;
}

/* edit extended attributes
   <-s param> to select custom key 
   <-e param> choose text editor
 */
int handle_edit(int argc, char *argv[])
{
  char key[256];
  snprintf(key, sizeof(key), "%s", KEY);

  int opt;
  optind = 2;
  opterr = 0;

  const char *editor = getenv("EDITOR");
  if (!editor) editor = DEFAULT_EDITOR;

  while((opt = getopt(argc, argv, "s:e:")) != -1)
    {
      switch(opt) {
      case 's':
	if (validate_optarg(optarg) < 0)
	  {
	    fprintf(stderr, "%s edit: invalid argument for '-s': '%s'\n", argv[0], optarg);
	    return -1;
	  }
	snprintf(key, sizeof(key), "user.postit.%s", optarg);
	break;
      case 'e':
	if (validate_optarg(optarg) < 0)
	  {
	    fprintf(stderr, "%s edit: invalid argument for '-e': '%s'\n", argv[0], optarg);
	    return -1;
	  }
	editor = optarg;
	break;
      case '?':
	fprintf(stderr, "%s edit: invalid option '-%c'\n", argv[0], optopt);
	return -1;
      }
    }

  if (argc - optind < 1)
    {
      fprintf(stderr, "%s edit: missing target file\n", argv[0]);
      return -1;
    }

  const char *file = argv[optind];

  ssize_t attribute_size = getxattr(file, key, NULL, 0);
  _cleanup_(cleanup) char *attribute = NULL;

  if (attribute_size > 0)
    {
    attribute = malloc(attribute_size + 1);
    if (attribute == NULL)
      {
	perror(argv[0]);
	return -1;
      }
    getxattr(file, key, attribute, attribute_size);
    attribute[attribute_size] = '\0';
    }
  else if (attribute_size < 0 && errno != ENODATA)
    {
      perror(argv[0]);
      return -1;
    }

  _cleanup_(unlink_temp) char *template = strdup("/tmp/postit-XXXXXX");
  if(template == NULL)
    {
      perror(argv[0]);
      return -1;
    }
  
  int fd = mkstemp(template);
  if (fd == -1)
    {
      perror("Failed to create temporary file");
      return -1;
    }

  {
    _cleanup_(close_file) FILE *fp = fdopen(fd, "w+");
    if(fp == NULL)
      {
	perror(argv[0]);
	close(fd);
	return -1;
      }

    if (attribute) fwrite(attribute, 1, strlen(attribute), fp);
  }
  
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "%s %s", editor, template);

  if (system(cmd) != 0) return -1;

  _cleanup_(close_file) FILE *fp_read = fopen(template, "rb");
  if (fp_read == NULL)
    {
      perror(argv[0]);
      return -1;
    }

  fseek(fp_read, 0, SEEK_END);
  size_t length = ftell(fp_read);
  rewind(fp_read);
  
  _cleanup_(cleanup) char *edit = malloc(length + 1);
  if(edit == NULL)
    {
      perror(argv[0]);
      return -1;
    }

  fread(edit, 1, length, fp_read);
  if(length > 0 && edit[length-1] == '\n')
    {
      length--;
    }
  
  edit[length] = '\0';
  
  if (lsetxattr(file, key, edit, strlen(edit), 0) == -1)
    {
      perror(argv[0]);
      return -1;
    }

  return 0;
}


int handle_rm(int argc, char *argv[])
{
  char key[256] = "";
  int opt;
  optind = 2;
  opterr = 0;

  int s = 0;

  while((opt = getopt(argc, argv, "s:")) != -1)
    {
      switch(opt) {
      case 's':
	if (validate_optarg(optarg) < 0)
	  {
	    fprintf(stderr, "%s rm: invalid argument for '-s': '%s'\n", argv[0], optarg);
	    return -1;
	  }
	s = 1;
	snprintf(key, sizeof(key), "user.postit.%s", optarg);
	break;
      case '?':
	fprintf(stderr, "%s rm: invalid option '-%c'\n", argv[0], optopt);
	return -1;
      }
    }

  if ( argc - optind < 1)
    {
      fprintf(stderr, "%s rm: missing target file\n", argv[0]);
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

  _cleanup_(cleanup) char *list = malloc(list_size);
  if(list == NULL)
    {
      perror(argv[0]);
      return -1;
    }

  ssize_t attributes_list = listxattr(file, list, list_size);
  if (attributes_list == -1)
    {
      return -1;
    }

  int found = 0;
  char *p = list;
  while(p < list + list_size){
    if(s)
      {
	if(strcmp(p, key) == 0)
	  {
	    found = 1;
	    if(removexattr(file, key) < 0)
	      {
		if (errno == ENODATA)
		  {
		    fprintf(stderr, "%s rm: attribute '%s' does not exist\n", argv[0], key);
		  }
		else
		  {
		    perror(argv[0]);
		  }
		return -1;
	      }
	    break;
	  }
      }
    else
      {
	if(strncmp(p, "user.postit.", 12) == 0)
	  {
	    if(removexattr(file, p) < 0)
	      {
		return -1;
	      }
	  }
      }
    p += strlen(p) + 1;
  }

  if(s && !found){
    fprintf(stderr, "%s rm: attribute '%s' does not exist\n", argv[0], key);
    return -1;
  }

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
      if (handle_edit(argc, argv) < 0) exit(EXIT_FAILURE);
      break;
    case CMD_RM:
      if (handle_rm(argc,argv) < 0) exit(EXIT_FAILURE);
      break;
    case CMD_HELP:
      break;
    case CMD_UNKNOWN:
      fprintf(stderr, "%s: unknown command '%s'\n", argv[0], argv[1]);
      exit(EXIT_FAILURE);
    }
  return EXIT_SUCCESS;
}
