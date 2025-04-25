#pragma once
#include <pcre.h>
#include <stdio.h>
#include <string.h>

#define print_err(msg, ret)                                                    \
  do {                                                                         \
    fprintf(stderr, msg "\n");                                                 \
    return ret;                                                                \
  } while (0)

void version_message();
void help_message();
int create_pcre(char *);
