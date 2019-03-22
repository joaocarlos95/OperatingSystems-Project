# Operating Systems Project (2015-2016)

### Contributors
- [@hugogaspar8](https://github.com/hugogaspar8) - Hugo Gaspar
- [@joaocarlos95](https://github.com/joaocarlos95) - João Carlos
- [@jtf16](https://github.com/jtf16) - João Freitas

### About
This project consists of the development of a shell (named par-shell) that allows running and monitoring programs executed in parallel on a multi-core machine. It's organized in 5 parts, consisting of:

  1. Develop the par-shell base, allowing the user to run multiple programs at the same time (given a path to them) and exit the pair-shell in an orderly fashion (terminating child processes first).
  1. Create a new thread that monitors the performance of each child process. Create a data structure shared by the two main threads (monitor and main) for performance purposes.
  1. Limit the maximum number of child processes running simultaneously. Implementation of semaphores for synchronization.
  1. Substitution of semaphores by mutex for synchronization and persistent recording of children's performance by writing to a text file.
  1. The last part was to develop the remote shell to redirect input commands to par-shell. In this way, several users could enter commands in the par-shell
