/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-27 01:36:54
*/

#ifndef FT_UTILS_H
#define FT_UTILS_H

/*************
 *  INCLUDE  *
 *************/
#include <stdlib.h>		/* NULL */
#include <stdio.h>		/* printf */
#include <sys/types.h>	/* size_t */
#include <string.h>		/* strcmp */
#include <math.h>		/* M_PI */

#include "cJSON.h"
#include "cbot.h"
#include "constantes.h"
#include "ft_orders.h"

void ft_orders_update(t_order *orders, t_track_info *trackInfo);
cJSON *ft_utils_find_car_pos(char *botName, cJSON *msgData);
void ft_main_new_lap(t_data *data, cJSON *json);

void ft_main_loop(int sock, char *botName);

void ft_main_data_parse(cJSON *json, t_data *data, int tick);
cJSON *ft_main_msg_make(cJSON *msgType, t_data *data);

void ft_init_get_orders(t_data *data, cJSON *msgData);

void ft_update_car_data(cJSON *msgData, t_data *data, int tick);
char *ft_trackName_get(cJSON *data);
cJSON *ft_utils_field_find(char *s, cJSON* head);

void ft_print_raw_data(char *type, cJSON *data);
void ft_print_gameInit_data(t_data *data);

#endif /* FT_UTILS_H */
