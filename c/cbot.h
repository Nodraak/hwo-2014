/* 
* @Author: Adrien Chardon
* @Date:   2014-04-17 14:15:16
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 14:28:13
*/

#ifndef CBOT_H
#define CBOT_H

void error(char *fmt, ...);
int connect_to(char *hostname, char *port);
void log_message(char *msg_type_name, cJSON *msg);

cJSON *ping_msg();
cJSON *join_msg(char *bot_name, char *bot_key);
cJSON *throttle_msg(double throttle);
cJSON *make_msg(char *type, cJSON *msg);

cJSON *read_msg(int fd);
void write_msg(int fd, cJSON *msg);

#endif
