an shell program that runs your bash commands in a background/child process, blocks with "&" , probably only takes one command at a time. Keeps a history of the last 10 commands

Use this script to verify memory leaks
script:
`valgrind --leak-check=yes bin/cd-hist`