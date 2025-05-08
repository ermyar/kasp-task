#include "calc.h"
#include <dirent.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <unistd.h>

static char path[BUFSIZ];
static unsigned char hash[EVP_MAX_MD_SIZE];
static char buf[BUFSIZ];
static size_t baselen = 0;
extern pcre *re;
static int ovector[FILENAME_MAX];

// calculate hash SHA256 sum using openssl lib
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

// dir walk to < path > directory. Current path has length len.
static void dir_walk(size_t len) {
  DIR *dirp;
  struct dirent *dp;

  if ((dirp = opendir(path)) == NULL) {
    print_err("opendir", );
  }

  // walk only by regular files/symlinks
  for (;;) {
    dp = readdir(dirp);

    if (dp == NULL) {
      break;
    }

    if (strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".") == 0) {
      continue;
    }

    if (dp->d_type == DT_REG || dp->d_type == DT_LNK) {

      // check if filename match given regex
      if (re != NULL) {
        size_t filenamelen = strlen(dp->d_name);
        int count = pcre_exec(re, NULL, dp->d_name, filenamelen, 0, 0, ovector,
                              FILENAME_MAX);

        if (count > 0) {
          // if name match we skip this file
          if (ovector[0] == 0 && ovector[1] == filenamelen) {
            continue;
          }
        }
      }

      path[len] = '/';
      strcpy(path + len + 1, dp->d_name);

      unsigned int hash_len = 0;
      int err = get_file_hash(path, hash, &hash_len);

      if (err != 0) {
        printf("error while calc\n");
        continue;
      }

      printf("%s  :  ", path + baselen);
      for (unsigned int i = 0; i < hash_len; ++i) {
        printf("%02x", hash[i]);
      }
      printf("\n");

      path[len] = '\0';
    }
  }

  rewinddir(dirp);

  // recursively go to directories
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

// initialize and start compution (dir_walk)
int calc(int argc, char *argv[]) {
  int ptr = optind;

  // case when no DIR_PATH, then path is "."
  if (ptr == argc) {
    path[0] = '.';
    path[1] = '\0';
    baselen = 2UL;
    dir_walk(1UL);
    return 0;
  }

  // we must go only to one dir (if we want we can fix it easy)
  if (ptr + 1 < argc) {
    perror("more than 1 dir_path, only first will calculated");
  }

  // our path is argv[optind]
  strcpy(path, argv[optind]);

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
