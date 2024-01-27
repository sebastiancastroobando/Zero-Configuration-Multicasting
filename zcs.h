#ifndef __ZCS_H__
#define __ZCS_H__

typedef struct {
    char *attr_name;
    char *value;
} zcs_attribute_t;

typedef void (*zcs_cb_f)(char *, char *);

typedef struct {
	int pid;	// for threads?
	bool is_app;
	zcs_attribute_t attributes;
} zcs_node_t;

// DEFAULT PORT NUMBERS:
#define SERVER_PORT 16000	// changed this from SOURCE
#define APP_PORT 16001		// changed this from DESTINATION, per our meeting

int zcs_init();
int zcs_start(char *name, zcs_attribute_t attr[], int num);
int zcs_post_ad(char *ad_name, char *ad_value);
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen);
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num);
int zcs_listen_ad(char *name, zcs_cb_f cback);
int zcs_shutdown();
void zcs_log();

#endif

