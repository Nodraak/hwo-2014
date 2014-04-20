/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-20 19:41:10
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

char *ft_trackName_get(cJSON *data);

void ft_main_loop(int sock);
void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders, char **trackName);

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo, t_order *orders);

void ft_utils_data_raw_print(char *type, cJSON *data);
cJSON *ft_utils_field_find(char *s, cJSON* head);

#endif /* FT_UTILS_H */
