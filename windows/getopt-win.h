#ifndef _GETOPT_WIN_H
#define _GETOPT_WIN_H

extern int optind, opterr;
extern char *optarg;

int getopt(int argc, char *argv[], char *optstring);

#define HAVE_GETOPT 1

#endif /* !_GETOPT_WIN_H */
