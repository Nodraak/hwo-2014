/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 11:54:25
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-25 17:10:36
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

#ifndef M_PI
	#define M_PI 3.14159265436
#endif

/***********
 *  PROTO  *
 ***********/

void ft_orders_file_save_to(t_order *orders, char *path);
void ft_orders_file_load_from(t_order *orders, FILE *f);

void ft_init_track_parse(cJSON *data, t_track_info *track);
int ft_orders_piece_count(cJSON *current);
void ft_orders_piece_parse(cJSON *current, t_track_info *data);

int ft_orders_is_in_range(double pos, double start, double end);
t_order *ft_get_order_between(t_order *orders, double start, double end);
double ft_orders_max_speed_get(t_track_piece *pieces, int id, int maxId);


int ft_orders_compute_speed(t_order *orders, t_track_info *trackInfo);
int ft_orders_compute_switch(t_order *orders, int nbOrder, t_track_info *trackInfo);
void ft_orders_compute(t_track_info *trackInfo, t_order *orders);

void ft_orders_reenable(t_order *orders);
void ft_orders_add(t_order *order, int id, double pos, int type, t_switch_type switchDir, double speed, int status);
cJSON *ft_orders_next_get(t_car_info *carInfo, t_order *orders);


#endif /* FT_ORDERS_H */
