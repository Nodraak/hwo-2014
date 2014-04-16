/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-16 23:54:47
*/

#ifndef FT_UTILS_H
#define FT_UTILS_H

#include <stdlib.h>		/* NULL */
#include <stdio.h>		/* printf */
#include <sys/types.h>	/* size_t */
#include <string.h>		/* strcmp */
#include <math.h>		/* M_PI */

#include "cJSON.h"
#include "ft_graph.h"

typedef struct		s_track_info
{
	double 			length;
	int				nb_elem;
}					t_track_info;

void ft_utils_data_parse(cJSON *json);

void ft_utils_info_join_print(cJSON *data);
void ft_utils_info_yourCar_print(cJSON *data);
void ft_utils_data_raw_print(char *type, cJSON *data);


cJSON *ft_utils_field_find(char *s, cJSON* head);
void ft_utils_track_parse(cJSON *data);
void ft_utils_piece_parse(cJSON *current, t_track_info *data, FILE *f);

#endif /* FT_UTILS_H */
