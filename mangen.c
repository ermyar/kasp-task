#include <dirent.h>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define VERSION "v1.0.1"

#define print_err(msg, ret)                                                    \
  do {                                                                         \
    perror(msg);                                                               \
    return ret;                                                                \
  } while (0)

static char path[BUFSIZ];
static unsigned char hash[EVP_MAX_MD_SIZE];
static char buf[BUFSIZ];
static size_t baselen = 0;

static int get_file_hash(char *path, unsigned char *hash,
                         unsigned int *hash_len) {
  FILE *f;
  if ((f = fopen(path, "rb")) == NULL) {
    print_err("fopen", -1);
  }
  EVP_MD_CTX *ctx;

  if ((ctx = EVP_MD_CTX_new()) == NULL)
    print_err("ctx_new", -1);

  if (1 != EVP_DigestInit_ex(ctx, EVP_sha256(), NULL))
    print_err("init_ex fault", -1);

  size_t len = 0;
  while ((len = fread(buf, 1, BUFSIZ, f)) > 0) {
    if (1 != EVP_DigestUpdate(ctx, buf, len))
      perror("update error");
  }

  if (1 != EVP_DigestFinal_ex(ctx, hash, hash_len))
    print_err("final_ex error", -1);

  EVP_MD_CTX_free(ctx);

  if (fclose(f) != 0) {
    print_err("fclose", -1);
  }

  return 0;
}

static void dir_walk(size_t len) {
  DIR *dirp;
  struct dirent *dp;

  if ((dirp = opendir(path)) == NULL) {
    print_err("opendir", );
  }

  for (;;) {
    dp = readdir(dirp);

    if (dp == NULL) {
      break;
    }

    if (strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".") == 0) {
      continue;
    }

    if (dp->d_type == DT_REG || dp->d_type == DT_LNK) {

      path[len] = '/';
      strcpy(path + len + 1, dp->d_name);

      printf("%s  :  ", path + baselen);

      unsigned int hash_len = 0;
      int err = get_file_hash(path, hash, &hash_len);

      path[len] = '\0';

      if (err != 0) {
        printf("error while calc\n");
        continue;
      }

      for (unsigned int i = 0; i < hash_len; ++i) {
        printf("%02x", hash[i]);
      }
      printf("\n");
    }
  }

  rewinddir(dirp);

  for (;;) {
    dp = readdir(dirp);

    if (dp == NULL) {
      break;
    }

    if (strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".") == 0) {
      continue;
    }

    if (dp->d_type == DT_DIR) {
      path[len] = '/';
      strcpy(path + len + 1, dp->d_name);
      dir_walk(len + 1 + strlen(dp->d_name));
      path[len] = '\0';
    }
  }

  if (closedir(dirp) < 0) {
    print_err("closedir", );
  }
}

int main(int argc, char *argv[]) {
  int state = 0;

  while ((state = getopt(argc, argv, "vh")) != -1) {
    switch (state) {
    case 'v':
      printf("mangen %s 2025 by Ermachkov Yaroslav\n", VERSION);
      return 0;
    case 'h':
      printf("Usage:  mangen [DIR_PATH] [OPTIONS]\n"
             "Generate and print manifest of directory in format:\n\n"
             "    <relative path to file from DIR_PATH> : <hash-sum>\n"
             "\nIf no DIR_PATH, working directoty is current. \n\n"
             "Flags description:\n"
             "    -h\t Print usage and this help message and exit.\n"
             "    -v\t Print version and exit.\n");
      return 0;
    case '?':
      printf("found wrong option!\n---- %s ----\n", optarg);
      break;
    }
  }

  int ptr = optind;
  if (ptr == argc) {
    path[0] = '.';
    path[1] = '\0';
    baselen = 2UL;
    dir_walk(1UL);
    return 0;
  }

  if (ptr + 1 < argc) {
    perror("more than 1 dir_path, only first will calculated");
  }

  strcpy(path, argv[ptr]);

  struct stat st;
  if (stat(path, &st) == -1) {
    print_err("stat failed with DIR_PATH", -1);
  }
  if (!S_ISDIR(st.st_mode)) {
    print_err("DIR_PATH not a directory!", -1);
  }

  size_t len = strlen(path);
  baselen = len + 1;
  dir_walk(len);
  return 0;
}
