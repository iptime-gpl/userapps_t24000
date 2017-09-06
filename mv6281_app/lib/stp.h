#ifndef _STP_H_
#define _STP_H_

#define STP_CONFIG_FILE "/etc/stp_config"

#define DEFAULT_BRIDGE_PRIORITY         32768
#define DEFAULT_BRIDGE_FORWOARD_DELAY   15
#define DEFAULT_BRIDGE_HELLO_TIME       2
#define DEFAULT_MAX_MESSAGE_AGE         20
#define DEFAULT_PORT_COST               4       // 1Gbps:4, 100Mbps:19, 10Mbps:100
#define DEFAULT_PORT_PRIORITY           128

extern int stp_set_bridge_priority(char *brname, int priority);
extern int stp_get_bridge_priority(char *brname, int *priority);
extern int stp_set_bridge_forward_delay(char *brname, int value);
extern int stp_get_bridge_forward_delay(char *brname, int *value);
extern int stp_set_bridge_hello_time(char *brname, int value);
extern int stp_get_bridge_hello_time(char *brname, int *value);
extern int stp_set_bridge_max_message_age(char *brname, int value);
extern int stp_get_bridge_max_message_age(char *brname, int *value);
extern int stp_set_bridge_port_cost(char *brname, int port, int value);
extern int stp_get_bridge_port_cost(char *brname, int port, int *value);
extern int stp_set_bridge_port_priority(char *brname, int port, int value);
extern int stp_get_bridge_port_priority(char *brname, int port, int *value);
extern int stp_init(char *brname);

#endif
