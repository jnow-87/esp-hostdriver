#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <inet.h>
#include <opt.h>


/* macros */
#define DEFAULT_DEV_MODE	INET_AP
#define DEFAULT_TYPE		SOCK_STREAM
#define DEFAULT_SSID		"mainart"
#define DEFAULT_PW			"deadbeef"


/* global variables */
opt_t opts = {
	.dev_mode = DEFAULT_DEV_MODE,
	.type = SOCK_STREAM,
	.ssid = DEFAULT_SSID,
	.password = DEFAULT_PW,
	.ip = 0,
	.port = 0,
	.esp_debug = 0,
};


/* global functions */
void parse_opt(int argc, char **argv){
	int i;
	int opt;
	int long_optind;
	struct option const long_opt[] = {
		{ .name = "dev-mode",	.has_arg = required_argument,	.flag = 0,	.val = 'm' },
		{ .name = "ssid",		.has_arg = required_argument,	.flag = 0,	.val = 's' },
		{ .name = "pw",			.has_arg = required_argument,	.flag = 0,	.val = 'p' },
		{ .name = "connection",	.has_arg = required_argument,	.flag = 0,	.val = 'c' },
		{ .name = "esp-debug",	.has_arg = no_argument,			.flag = 0,	.val = 'd' },
		{ .name = "help",		.has_arg = no_argument,			.flag = 0,	.val = 'h' },
		{ 0, 0, 0, 0}
	};


	/* parse arguments */
	while((opt = getopt_long(argc, argv, "m:t:s:p:c:dh", long_opt, &long_optind)) != -1){
		switch(opt){
		case 'm':
			if(strcmp(optarg, "ap") == 0)			opts.dev_mode = INET_AP;
			else if(strcmp(optarg, "client") == 0)	opts.dev_mode = INET_CLIENT;
			else									help(argv[0], "invalid device mode \"%s\"", optarg);

			break;

		case 't':
			if(strcmp(optarg, "dgram") == 0)		opts.type = SOCK_DGRAM;
			else if(strcmp(optarg, "stream") == 0)	opts.type = SOCK_STREAM;
			else									help(argv[0], "invalid type \"%s\"", optarg);

			break;

		case 's':
			opts.ssid = optarg;
			break;

		case 'p':
			opts.password = optarg;
			break;

		case 'c':
			for(i=strlen(optarg); i>0; i--){
				if(optarg[i] == ':'){
					optarg[i++] = 0;
					opts.ip = inet_addr(optarg);
					break;
				}
			}

			opts.port = atoi(optarg + i);

			break;

		case 'd':
			opts.esp_debug = 1;
			break;

		case 'h':
			help(argv[0], "");
			break;

		case ':':	/* missing argument */
		case '?':	/* invalid option */
		default:	/* something else went wrong */
			help(argv[0], "invalid argument \"%s\"", optarg);
		}
	}

	if(optind < argc)
		help(argv[0], "%d unknown argument(s):", argc - optind);

	if(opts.port == 0)
		help(argv[0], "invalid connection configuration");
}

void help(char const *prog_name, char const *msg, ...){
	va_list lst;


	if(msg){
		va_start(lst, msg);
		vfprintf(stderr, msg, lst);
		fprintf(stderr, "\n\n");
		va_end(lst);
	}

	printf("usage: %s [<options>]", prog_name);

	printf(
		"\n"
		"\n"
		"Options:\n"
		"    %-25s    %s\n"
		"    %-25s    %s\n"
		"    %-25s    %s\n"
		"    %-25s    %s\n"
		"    %-25s    %s\n"
		"    %-25s    %s\n"

		, "-m, --dev-mode=<mode>", "esp01 mode (client or ap)"
		, "-t, --type=<type>", "socket type (dgram or stream)"
		, "-s, --ssid=<ssid>", "ssid"
		, "-p, --pw=<password>", "password"
		, "-c, --connection=<ip>:<port> | <port>", "connect to <ip>:<port> or start server on <port>"
		, "-d, --esp-debug", "print the esp output"
		, "-h, --help", "print this help message"
	);

	exit(msg == 0x0 ? 0 : -1);
}
