Impersonate
===========

Impersonate is a simple tool that hijacks getpwuid_r call and injects an arbitrary username in the result. It's indeed very hacky and dirty and if you rely on it for something serious, there must be something seriously wrong with what you are doing!

The original motivation for this is to easily impersonate arbitrary users while interacting with Hadoop configured with "Simple" security as Hadoop client relies on getpwuid_r call to determine the user. 

More specifically, Hadoop relies on com.sun.security.auth.module.getUsername() which relies on getpwuid_r.

How it works
===========

Impersonate generates a shared library to a temp folder, puts it in LD_PRELOAD and passes LD_PRELOAD env variable to the subsequent call. The parameter passing between the impersonate itself and the getpwuid_r is via an environment variable.

The dynamic generation of the .so file is there to hide the LD_PRELOAD step and make the utility more innocuous looking, apart from making it more portable and easier to use!


Usage
===========

Usage:  impersonate NAME COMMAND [ARG1 [ARG2 ...]]

Example: impersonate hdfs hadoop fs -cat /user/very/secure/important.txt  
Output: [Super secret data here]

Example: impersonate jack hadoop fs -rmr /user/jack

Build
===========

Three step build is as follows:

- Create shared object file containing hijacking functions.
- Run xxd tool to turn shared object to a C array
- Compile a binary and embed the shared object file to extracted on the fly.

Credits
===========

Omid Aladini

