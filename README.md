Impersonate
===========

Impersonate is a simple tool to hijack getuid and geteuid calls and return arbitrary ids instead. It's indeed very hacky and dirty and if you rely on it for something serious, there must be something seriously wrong with what you are doing!

The original motivation for this is to easily impersonate arbitrary users while interacting with Hadoop configured with "Simple" security as Hadoop client relies on geteuid call to determine the user.

Usage
===========

Usage:  impersonate UID_OR_NAME COMMAND [ARG1 [ARG2 ...]]

Example: impersonate root whoami  
Output: root

Example: impersonate 12345 id  
Output: uid=12345 gid=1525(omid) ...

Example: impersonate hdfs hadoop fs -cat /user/very/secure/important.txt  
Output: [Super secret data here]

Example: impersonate jack hadoop fs -rmr /user/jack

How it works
===========

The program dynamically generates a shared object file and places it on LD_PRELOAD so that getuid and geteuid get hijacked by the so file. The dynamic generation of the .so file is there to hide the LD_PRELOAD step and make the utility more innocuous looking, apart from making it more portable and easier to use!

Build
===========

Three step build is as follows:

- Create shared object file containing hijacking functions.
- Run xxd tool to turn shared object to a C array
- Compile a binary and embed the shared object file to extracted on the fly.

Credits
===========

Omid Aladini

