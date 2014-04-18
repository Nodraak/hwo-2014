/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-18 13:56:27
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

/*
	maximum speed in curve, if the car crash, decrease. (unit : between 0 and 10)
*/
#define SPEED_DURING_TURN		6.1
/*
	distance used to slow down, per speed unit. (unit : pieceIndex)
	if the car crash engage the curve to quickly, increase.
	edit : the above is false, this is just the distance from curve the car start to slow down
*/
#define DISTANCE_TO_SLOW_DOWN	1.5


/* acceleration start, before end of curve. (unit : pieceIndex) */
#define ACC_DISTANCE			1.7

/*
	Keimola (finland)
	germany
*/


//#define DISABLE_ORDERS
//#define TRACK_NAME				"germany"

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

enum e_order_type
{
	ORDER_TYPE_SPEED,
	ORDER_TYPE_SWITCH
};

enum e_order_status
{
	ORDER_STATUS_ENABLED,
	ORDER_STATUS_DISABLED,
	ORDER_STATUS_SENDED
};

enum e_switch_type
{
	ORDER_SWITCH_RIGHT,
	ORDER_SWITCH_LEFT
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
	double			speedActual;
	double			speedWanted;
	double			angle;
}					t_car_basic;

#define MAX_ORDERS	100
typedef struct		s_order
{
	/*int				nbElem;*/
	double			pos;
	int				type;
	int				valueInt;
	double			valueDouble;
	int				status;
}					t_order;

void ft_main_loop(int sock);
void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders);

double my_abs(double n);

/*double ft_utils_get_speed(t_car_basic *all);
cJSON *ft_utils_get_switch(t_car_basic *all);*/

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo);

void ft_utils_data_raw_print(char *type, cJSON *data);
cJSON *ft_utils_field_find(char *s, cJSON* head);

void ft_utils_track_parse(cJSON *data, t_track_info *track);
void ft_utils_piece_parse(cJSON *current, t_track_info *data, int behaviour);
void ft_utils_order_compute(t_track_info *trackInfo, t_order *orders);
void ft_order_reenable(t_order *orders);
void ft_order_add(t_order *order, double pos, int type, int valueInt, double valueDouble, int status);
cJSON *ft_order_get_next(t_car_basic *carInfo, t_order *orders);


#endif /* FT_UTILS_H */
