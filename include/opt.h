#ifndef OPT_H
#define OPT_H


#include <stdarg.h>
#include <inet.h>


/* types */
typedef struct{
	inet_dev_mode_t dev_mode;
	sock_type_t type;

	char *ssid,
		 *password;

	inet_addr_t ip;
	int port;

	int esp_debug;
} opt_t;


/* external variables */
extern opt_t opts;


/* prototypes */
void parse_opt(int argc, char **argv);
void help(char const *prog_name, char const *msg, ...);


#endif // OPT_H
