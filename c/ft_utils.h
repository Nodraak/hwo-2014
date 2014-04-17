/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 19:18:30
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

#define MAX_SPEED		6.5

enum e_piece_parse
{
	PIECE_PARSE_COUNT,
	PIECE_PARSE_BUILD
};

enum e_piece_type
{
	PIECE_TYPE_RIGHT,
	PIECE_TYPE_CURVE
};

typedef struct		s_track_piece
{
	int				type;
	double 			length;
}					t_track_piece;

typedef struct		s_track_info
{
	double 			lengthTotal;
	int				nbElem;
	t_track_piece	*pieces;
}					t_track_info;

typedef struct		s_car_basic
{
	double			pos;

	int				pieceIndex;
	double			inPieceDistance;
	double			speed;
	double			angle;
}					t_car_basic;


void ft_main_loop(int sock);
void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo);

double my_abs(double n);

double ft_utils_get_speed(t_car_basic *all);
cJSON *ft_utils_get_switch(t_car_basic *all);

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo);

void ft_utils_data_raw_print(char *type, cJSON *data);
cJSON *ft_utils_field_find(char *s, cJSON* head);

void ft_utils_track_parse(cJSON *data, t_track_info *track);
void ft_utils_piece_parse(cJSON *current, t_track_info *data, int behaviour);



#endif /* FT_UTILS_H */
