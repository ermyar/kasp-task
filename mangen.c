#include "calc.h"
#include "utils.h"
#include <unistd.h>

#define VERSION "v1.1.2"

pcre *re = NULL;

int main(int argc, char *argv[]) {
  int state = 0;

  // parsing options
  while ((state = getopt(argc, argv, "vhe:")) != -1) {
    switch (state) {
    case 'e':
      int err = create_pcre(optarg);
      if (err != 0) {
        return err;
      }
      break;
    case 'v':
      printf("mangen %s 2025 by Ermachkov Yaroslav\n", VERSION);
      version_message();
      return 0;
    case 'h':
      help_message();
      return 0;
    case '?':
      printf("found wrong option!\n---- %s ----\n", optarg);
      break;
    }
  }

  return calc(argc, argv);
}
