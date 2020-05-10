/*
 * "Optimize" a list of dependencies as spit out by gcc -MD
 * for the kernel build
 * ===========================================================================
 *
 * Author       Kai Germaschewski
 * Copyright    2002 by Kai Germaschewski  <kai.germaschewski@gmx.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 *
 * Introduction:
 *
 * gcc produces a very nice and correct list of dependencies which
 * tells make when to remake a file.
 *
 * To use this list as-is however has the drawback that virtually
 * every file in the kernel includes autoconf.h.
 *
 * If the user re-runs make *config, autoconf.h will be
 * regenerated.  make notices that and will rebuild every file which
 * includes autoconf.h, i.e. basically all files. This is extremely
 * annoying if the user just changed CONFIG_HIS_DRIVER from n to m.
 *
 * So we play the same trick that "mkdep" played before. We replace
 * the dependency on autoconf.h by a dependency on every config
 * option which is mentioned in any of the listed prequisites.
 *
 * kconfig populates a tree in include/config/ with an empty file
 * for each config symbol and when the configuration is updated
 * the files representing changed config options are touched
 * which then let make pick up the changes and the files that use
 * the config symbols are rebuilt.
 *
 * So if the user changes his CONFIG_HIS_DRIVER option, only the objects
 * which depend on "include/linux/config/his/driver.h" will be rebuilt,
 * so most likely only his driver ;-)
 *
 * The idea above dates, by the way, back to Michael E Chastain, AFAIK.
 *
 * So to get dependencies right, there are two issues:
 * o if any of the files the compiler read changed, we need to rebuild
 * o if the command line given to the compile the file changed, we
 *   better rebuild as well.
 *
 * The former is handled by using the -MD output, the later by saving
 * the command line used to compile the old object and comparing it
 * to the one we would now use.
 *
 * Again, also this idea is pretty old and has been discussed on
 * kbuild-devel a long time ago. I don't have a sensibly working
 * internet connection right now, so I rather don't mention names
 * without double checking.
 *
 * This code here has been based partially based on mkdep.c, which
 * says the following about its history:
 *
 *   Copyright abandoned, Michael Chastain, <mailto:mec@shout.net>.
 *   This is a C version of syncdep.pl by Werner Almesberger.
 *
 *
 * It is invoked as
 *
 *   fixdep <depfile> <target> <cmdline>
 *
 * and will read the dependency file <depfile>
 *
 * The transformed dependency snipped is written to stdout.
 *
 * It first generates a line
 *
 *   cmd_<target> = <cmdline>
 *
 * and then basically copies the .<target>.d file to stdout, in the
 * process filtering out the dependency on autoconf.h and adding
 * dependencies on include/config/my/option.h for every
 * CONFIG_MY_OPTION encountered in any of the prequisites.
 *
 * It will also filter out all the dependencies on *.ver. We need
 * to make sure that the generated version checksum are globally up
 * to date before even starting the recursive build, so it's too late
 * at this point anyway.
 *
 * The algorithm to grep for "CONFIG_..." is bit unusual, but should
 * be fast ;-) We don't even try to really parse the header files, but
 * merely grep, i.e. if CONFIG_FOO is mentioned in a comment, it will
 * be picked up as well. It's not a problem with respect to
 * correctness, since that can only give too many dependencies, thus
 * we cannot miss a rebuild. Since people tend to not mention totally
 * unrelated CONFIG_ options all over the place, it's not an
 * efficiency problem either.
 *
 *(Note: it'd be easy to port over the complete mkdep state machine,
 *  but I don't think the added complexity is worth it)
 */
/*
 * Note 2: if somebody writes HELLO_CONFIG_BOOM in a file, it will depend onto
 * CONFIG_BOOM. This could seem a bug(not too hard to fix), but please do not
 * fix it! Some UserModeLinux files(look at arch/um/) call CONFIG_BOOM as
 * UML_CONFIG_BOOM, to avoid conflicts with /usr/include/linux/autoconf.h,
 * through arch/um/include/uml-config.h; this fixdep "bug" makes sure that
 * those files will have correct dependencies.
 *
 * Note by Jan Nowotsch:
 * 	This code has been borrowed from the linux kernel build system.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include "util.h"
#include "hashtbl.h"


/* static prototypes */
static void parse_dep_file(void *map, size_t len);
static void parse_prereq(const char *map, size_t len);
static void update_prereq(const char *m, int slen);


/* global variables */
char *depfile;
char *conf_header;
char *conf_dir;


/* global functions */
int main(int argc, char *argv[]){
	int fd;
	void *map;
	unsigned int size;


	/* init */
	traps();

	if(argc != 4){
		fprintf(stderr, "Usage: %s <depfile> <config_header> <config_dir>\n", argv[0]);
		return 1;
	}

	depfile = argv[1];
	conf_header = argv[2];
	conf_dir = argv[3];

	/* open and mmap depfile */
	if(file_map(depfile, &fd, &map, &size) != 0)
		return 1;

	/* parse depfile */
	parse_dep_file(map, size);

	/* exit */
	file_unmap(fd, map, size);

	return 0;
}


/* local functions */
/* parse the supplied dependency file */
static void parse_dep_file(void *dmap, size_t len){
	char *m = dmap,
		 *end = m + len,
		 *tgt,
		 *prereq;
	int fd;
	unsigned int size;
	unsigned int confh_len = strlen(conf_header);
	char *fmap;


	/* clear hash table */
	hashtbl_clear();

	/* find target */
	while(m < end && (*m == ' ' || *m == '\\' || *m == '\n' || *m == '\r')) m++;

	tgt = m;

	while(m < end && *m != ':') m++;

	if(m < end)
		*m = 0;

	m++;

	/* print target */
	printf("%s:", tgt);

	/* handle prerequisites */
	while(m < end){
		// find next prerequisites
		while(m < end && (*m == ' ' || *m == '\\' || *m == '\n' || *m == '\r')) m++;

		prereq = m;

		while(m < end && *m != ' ' && *m != '\\' && *m != '\n' && *m != '\r') m++;

		// break if prerequisite is actually a target
		if(m > 0 && m[-1] == ':'){
			*m = 0;
			m++;

			printf("\n\n%s", prereq);
			break;
		}

		if(m < end)
			*m = 0;

		if(m >= end || tgt >= end  || prereq >= end)
			break;

		// parse prerequisite that are not the conf_header
		if(strrcmp(prereq, m - prereq, conf_header, confh_len)){
			printf(" \\\n  %s", prereq);

			if(file_map(prereq, &fd, (void*)&fmap, &size) != 0)
				return;

			parse_prereq(fmap, size);
			file_unmap(fd, fmap, size);
		}

		m++;
	}

	/* print the remainder of the dependency file,
	 * i.e. remaining targets */
	printf("\n%s", m);
}

/* parse prerequisite file for 'CONFIG_' occurences */
static void parse_prereq(const char *map, size_t len){
	const int *end = (const int *)(map + len);
	const int *m   = (const int *)map + 1; 	// start at +1, so that p can never be < map
	const char *p, *q;


	for(; m < end; m++){
		if(*m == INT_CONF)		p = (char*)m;
		else if(*m == INT_ONFI)	p = (char*)m-1;
		else if(*m == INT_NFIG)	p = (char*)m-2;
		else if(*m == INT_FIG_)	p = (char*)m-3;
		else continue;

		if(p > map + len - 7)
			continue;

		if(memcmp(p, "CONFIG_", 7))
			continue;

		for(q = p + 7; q < map + len; q++){
			if(!(isalnum(*q) || *q == '_')){
				if((q-p-7) >= 0)
					update_prereq(p+7, q-p-7);
				break;
			}
		}
	}
}

/* print CONFIG_ prerequisite */
static void update_prereq(const char *m, int slen){
	int i;

	// return if already hashed
	if(hashtbl_add(m, slen))
		return;

	printf(" \\\n    $(wildcard %s", conf_dir);

	for(i=0; i<slen; i++){
		if(m[i] == '_')	putchar('/');
		else			putchar(tolower(m[i]));
	}
	printf(".h)");
}
