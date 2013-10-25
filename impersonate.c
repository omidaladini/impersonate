#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <pwd.h>

#define SPOOF_UID "SPOOF_UID"

extern char **environ;

#ifdef NOMAIN

int getuid() { return geteuid(); }

int geteuid() {
    char ** env;
    char key[10], value[10];

    for (env = environ; *env; ++env) {
        sscanf(*env, "%10[^'=']=%10[^'=']", key, value); //borrowed from max key/value size = 10
        if(strcmp(SPOOF_UID, key) == 0) 
            break;
    };

    // Get the original geteuid for fallback
    typedef uid_t (*geteuid_funcptr_t)();
    geteuid_funcptr_t original_geteuid =  (geteuid_funcptr_t) dlsym(RTLD_NEXT, "geteuid");

    char *end_ptr;
    int euid_int = strtol(value, &end_ptr, 10);

    if(euid_int == 0 && end_ptr == value) {
        const struct passwd* passwd_result = getpwnam(value);
        if(passwd_result)
            return passwd_result->pw_uid;
        else
            return original_geteuid();
    } else
        return euid_int;
}

#else

int main(int argc, char *argv[]) {

  if(argc < 3) { 
    printf("Usage:   $ %s PROG UID|NAME PROG [ARG...]\n", argv[0]);
    printf("Example: $ %s 0 whoami\n", SPOOF_UID, argv[0]);
    printf("Output:  $ root\n");
    return 0;
  }

  // Load the hex dump of this file compiled without main (provided by the build)
  // Unfortunately it's hard to make #include take a variable due to C preprocessor
  #include "impersonate.so.xxd" 

  // Create a temp file to contain the shared object
  char temp_file_name[L_tmpnam+strlen(P_tmpdir)];
  strcpy(temp_file_name, P_tmpdir);
  strcpy(temp_file_name, tmpnam(0));

  // Dump the content of the shared object
  FILE *impersonate_so_file = fopen(temp_file_name, "w");
  fwrite(impersonate_so, sizeof(unsigned char), impersonate_so_len, impersonate_so_file);
  fclose(impersonate_so_file);

  // Put the shared object in LD_PRELOAD
  // to shadow functions redefined in this file
  setenv("LD_PRELOAD", temp_file_name, 1);
  setenv(SPOOF_UID, *(argv+1), 1);

  // Run the victim executable 
  execvpe(argv[2], argv+2, environ); 
}

#endif


