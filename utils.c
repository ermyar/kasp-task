#include "utils.h"

#define COMMIT_HASH_LEN 41
extern pcre *re;

// print version message
void version_message() {
  FILE *check_git = fopen(".git/HEAD", "r");
  if (check_git == NULL) {
    return;
  }
  fclose(check_git);

  FILE *pipe = popen("git rev-parse HEAD", "r");
  if (pipe == NULL) {
    return;
  }
  char commit_hash[COMMIT_HASH_LEN];
  if (fgets(commit_hash, COMMIT_HASH_LEN, pipe) == NULL) {
    pclose(pipe);
    return;
  }

  commit_hash[COMMIT_HASH_LEN - 1] = '\0';
  printf("git commit hash: %s\n", commit_hash);

  pclose(pipe);
}

// print help message with these options
void help_message() {
  printf("Usage:  mangen [DIR_PATH] [OPTIONS]\n"
         "Generate and print manifest of directory in format:\n\n"
         "    <relative path to file from DIR_PATH> : <hash-sum>\n"
         "\nIf no DIR_PATH, working directoty is current. \n\n"
         "Flags description:\n");

  struct options {
    char *opt;
    char *desc;
  } list[] = {{"-e pattern", "Except file with name match given pattern."},
              {"-h", "Print usage and this help message and exit."},
              {"-v", "Print version and exit."}};

  size_t len = sizeof(list) / sizeof(list[0]);
  int padding = 0;

  for (size_t ind = 0; ind < len; ++ind) {
    int len = strlen(list[ind].opt);
    padding = (len > padding) ? len : padding;
  }

  for (size_t ind = 0; ind < len; ++ind) {
    printf("  %-*s   %s\n", padding, list[ind].opt, list[ind].desc);
  }
}

// have to free pointer to re
static void clean_pcre() { free(re); }

// compile regex by pcrelib
int create_pcre(char *ex_pattern) {
  int options = 0;
  const char *errors;
  int erroffset;
  re = pcre_compile(ex_pattern, options, &errors, &erroffset, NULL);

  if (re == NULL) {
    fprintf(stderr, "%s\n", errors);
    print_err("wrong pattern", -1);
  }

  atexit(clean_pcre);
  return 0;
}

extern char *path;
extern size_t baselen;
extern void dir_walk(size_t);
