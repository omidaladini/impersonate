/*
  Author: Omid Aladini <omidaladini@gmail.com>
*/


#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <pwd.h>

#define SPOOF_UNAME "SPOOF_UNAME"

extern char **environ;

#ifdef NOMAIN

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result) {
    char ** env;
    char key[20], value[20];

    for (env = environ; *env; ++env) {
        sscanf(*env, "%20[^'=']=%20[^'=']", key, value); //borrowed from max key/value size = 10
        if(strcmp(SPOOF_UNAME, key) == 0) 
            break;
    };

    typedef int (*getpwuid_r_funcptr_t)(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
    static getpwuid_r_funcptr_t original_getpwuid_r = 0; 
    if(!original_getpwuid_r) // Cache it as static as dlsym is slow
       original_getpwuid_r = (getpwuid_r_funcptr_t) dlsym(RTLD_NEXT, "getpwuid_r");

    int orig_result = original_getpwuid_r(getuid(), pwd, buf, buflen, result);
    (*result)->pw_name = strdup(value);  //strdup is a POSIX call
    return orig_result;
}
#else

int main(int argc, char *argv[]) {

  if(argc < 3) { 
    printf("Usage:   $ %s NAME PROG [ARG...]\n", argv[0]);
    printf("Example: $ %s hadoop fs -mv ...\n",  argv[0]);
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
  setenv(SPOOF_UNAME, *(argv+1), 1);

  // Run the victim executable 
  execvpe(argv[2], argv+2, environ); 
}

#endif


