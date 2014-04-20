/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 11:54:25
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-20 10:55:07
*/

#ifndef FT_ORDERS_H
#define FT_ORDERS_H

#include <stdlib.h>		/* NULL */
#include <stdio.h>		/* printf */
#include <sys/types.h>	/* size_t */
#include <string.h>		/* strcmp */
#include <math.h>		/* M_PI */

#include "cJSON.h"
#include "cbot.h"
#include "constantes.h"
#include "ft_utils.h"

/***********
 *  PROTO  *
 ***********/

void ft_orders_track_parse(cJSON *data, t_track_info *track);
int ft_orders_piece_count(cJSON *current);
void ft_orders_piece_parse(cJSON *current, t_track_info *data);

double ft_orders_max_speed_get(t_track_piece *pieces, int id);
void ft_orders_compute(t_track_info *trackInfo, t_order *orders);
void ft_orders_reenable(t_order *orders);
void ft_orders_add(t_order *order, double pos, int type, int valueInt, double valueDouble, int status);
cJSON *ft_orders_next_get(t_car_basic *carInfo, t_order *orders);


#endif /* FT_ORDERS_H */
