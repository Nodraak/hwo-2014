/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 14:21:11
*/

#ifndef FT_UTILS_H
#define FT_UTILS_H

#include <stdlib.h>		/* NULL */
#include <stdio.h>		/* printf */
#include <sys/types.h>	/* size_t */
#include <string.h>		/* strcmp */
#include <math.h>		/* M_PI */

#include "cJSON.h"
#include "cbot.h"
#include "ft_graph.h"

#define MAX_SPEED	0.7

typedef struct		s_track_info
{
	double 			length;
	int				nb_elem;
}					t_track_info;

typedef struct		s_all
{
	int				pieceIndex;
	double			inPieceDistance;
	double			speed;
	double			angle;
}					t_all;

cJSON *ft_main_loop(cJSON *json);

void ft_utils_data_parse(cJSON *json, t_all *all);

void ft_update_car_data(cJSON *data, t_all *all);

void ft_utils_info_join_print(cJSON *data);
void ft_utils_info_yourCar_print(cJSON *data);
void ft_utils_data_raw_print(char *type, cJSON *data);


cJSON *ft_utils_field_find(char *s, cJSON* head);
char **ft_utils_track_parse(cJSON *data);
void ft_utils_piece_parse(cJSON *current, t_track_info *data, FILE *f);

#endif /* FT_UTILS_H */
