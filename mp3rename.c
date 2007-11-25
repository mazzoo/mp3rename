/*
 * mp3rename
 * renames files, eliminating odd characters from the filename
 *
 * (c) Matthias Wenzel
 * licensed under GPLv2
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#define VERSION "0.1"

/* tunable by options */
int  opt_pretend    = 0;
int  opt_verbose    = 0;
int  opt_print_dirs = 0;
char * opt_suffix   = ".mp3";

char * argv0;

char buf[PATH_MAX+NAME_MAX+1];

char xtable[][2]={
//	{ ' ', '_'},
	{ '!', '_'},
	{ '"', '_'},
	{ '#', '_'},
	{ '$', '_'},
	{ '%', '_'},
	{ '&', '_'},
	{ '\'', '_'},
	{ '(', '_'},
	{ ')', '_'},
	{ '*', '.'},
	{ '+', '.'},
	{ ':', '_'},
	{ ';', '_'},
	{ '<', '_'},
	{ '=', '-'},
	{ '>', '_'},
	{ '?', '_'},
	{ '@', 'a'},
	{ '[', '_'},
	{ '\\', '_'},
	{ ']', '_'},
	{ '^', '_'},
	{ '`', '_'},
	{ '{', '_'},
	{ '|', 'I'},
	{ '}', '_'},
	{ 0, 0},
};

char * filename(char * n)
{
	int l = strlen(n);
	while	(
		( n[l] != '/' ) &&
		( l != 0)
		)
		l--;
	return &n[l+1];
}

int do_rename_file(char * n)
{

	char * f = filename(n);
	if (	( f[0] == '.') ||
		(!f[0]       ) )
		return -1;

	if ( strlen(f) < strlen(opt_suffix) )
		return -1;

	if (strcasecmp(&(f[strlen(f)-strlen(opt_suffix)]), opt_suffix)){
		return -1;
	}

	char * o = strdup(n);
	char * p = f;

	int did_ren=0;
	while (*p){
		int x=0;
		while (xtable[x++][0]){
			if (*p == xtable[x][0]){
				*p = xtable[x][1];
				did_ren++;
			}
		}
		p++;
	}
	int ret=0;
	if (did_ren){
		if (opt_pretend == 0){
			if (opt_verbose)
				printf("renaming:\n%s\n%s\n", o, n);fflush(stdout);
			ret = rename(o, n);
		}else{
			printf("WOULD rename:\n%s\n%s\n", o, n);fflush(stdout);
		}
	}
	if (ret)
		fprintf(stderr, "rename(\"%s\", \"%s\"): %s\n",
			o,
			n,
			strerror(errno));
	free(o);
	return ret;
}

int process(char * n)
{
	int ret;
	struct stat s;
	ret = lstat(n, &s);
	if ( ret /*&& (errno != ENOTDIR)*/){
		fprintf(stderr, "lstat(\"%s\"): %s\n",
			n,
			strerror(errno));
		return -1;
	}

	/* is a directory */
	if (	( S_ISDIR(s.st_mode)) &&
		(!S_ISREG(s.st_mode)) &&
		(!S_ISLNK(s.st_mode)) ){
		if (opt_print_dirs)
			printf("entering dir %s\n", n);fflush(stdout);

		DIR * d;
		d = opendir(n);
		if (!d){
			fprintf(stderr, "opendir(\"%s\"): %s\n",
				n,
				strerror(errno));
			return -1;
		}

		struct dirent * de;
		while ((de = readdir(d)) != NULL ){
			if (de < 0){
				fprintf(stderr, "readdirr(\"%s\"): %s\n",
					n,
					strerror(errno));
				closedir(d);
				return -1;
			}
			if (de->d_name[0] != '.'){
				memcpy(buf, n, strlen(n));
				buf[strlen(n)] = '/';
				memcpy(&buf[strlen(n)+1], de->d_name, strlen(de->d_name)+1);
				char * buf2 = strdup(buf);
				process(buf2);
				free(buf2);
			}
		}
		closedir(d);
		return 0;
	}

	/* is a regular file */
	if (	( S_ISREG(s.st_mode)) &&
		(!S_ISDIR(s.st_mode)) &&
		(!S_ISLNK(s.st_mode)) ){
		if (n[0] == '.')
			return -1;
		do_rename_file(n);
		return 0;
	}
	return -1;
}

void usage(void)
{
	printf("\n%s version: %s\n\n", argv0, VERSION);
	printf("%s [option(s)] name(s)\n", argv0);
	printf("\n\tname(s)  \tone or more file or directroy names\n");
	printf("\t         \tcontaining .mp3 files to rename (recursively)\n");
	printf("\noption(s):\n");
	printf("\t-s suffix\trename files with suffix instead of\n");
	printf("\t         \tthe default \".mp3\" ending\n");
	printf("\t-v       \tverbose. print a message for each rename\n");
	printf("\t-d       \tprint directory names when entering a new dir\n");
	printf("\t-p       \tpretend. just print what would be renamed,\n");
	printf("\t         \tdon't change the filenames\n");
	printf("\n");
	exit(0);
}

void parse_cmdline(int argc, char *argv[])
{
	int optchar;
	while ((optchar = getopt (argc, argv, "s:vsp")) != -1) {
		switch (optchar) {
			case 's':
				opt_suffix = strdup(optarg);
				break;
			case 'v':
				opt_verbose = 1;
				break;
			case 'd':
				opt_print_dirs = 1;
				break;
			case 'p':
				opt_pretend = 1;
				break;
			default:
				usage();
		}
	}
}

int main(int argc, char ** argv)
{
	argv0 = argv[0];

	parse_cmdline(argc, argv);

	if (!argv[optind])
		usage();

	int i;
	for (i=optind; i<argc; i++)
		if (argv[i][0] != '.')
			process(argv[i]);
	return 0;
}

