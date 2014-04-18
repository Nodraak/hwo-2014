/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-18 20:32:35
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

/*****************
 *  USER CONFIG  *
 *****************/
/*
## notes ##

angle :
	left = neg | right = pos

*/

/*#define TRACK_NAME					"keimola"*/
#define TRACK_NAME					"germany"

#define NB_PLAYER					1
/*
	print car pos every PRINT_CAR_POS_MODULO game ticks
	big number like 3000 to desactivate
*/
#define PRINT_CAR_POS_MODULO		3000

/*************
 *  DEFINES  *		note : EDIT AT YOUR OWN RISK
 *************/

/*
	maximum speed in curve, if the car crash, decrease. (unit : between 0 and 10)
*/
#define SPEED_CURVE_HARD			4	/* 6.2 for 100-radius curves */
/*
			edit : nope -- distance used to slow down, per speed unit. (unit : pieceIndex) --
	if the car crash engage the curve to quickly, decrease.
*/
#define SPEED_LOST_PER_TRACK_PIECE	1.9
/*
	Send order SEND_ORDER_OFFSET (unit : pieceIndex) before the car reach the asked pos
	if the car apply order too late, increase 
*/
#define SEND_ORDER_OFFSET			0.60
/*
	All curves with angle below this are seen as right
*/
#define ANGLE_CONSIDERED_AS_RIGHT	30
/*
	diff between current and wanted speed within the speed order are none (0) or full (1)
*/
#define SPEED_DIFF_EXTREM_VALUES	0.10


/**********
 *  ENUM  *
 **********/
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


/*************
 *  STRUCT  *
 *************/
typedef struct		s_track_piece
{
	int				type;
	double 			length;
	int				angle;
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
	int				type;
	int				valueInt;
	double			valueDouble;
	int				status;
}					t_order;

/***********
 *  PROTO  *
 ***********/
void ft_main_loop(int sock);
void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders);

double my_abs(double n);

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo);

void ft_utils_data_raw_print(char *type, cJSON *data);
cJSON *ft_utils_field_find(char *s, cJSON* head);

void ft_utils_track_parse(cJSON *data, t_track_info *track);
void ft_utils_piece_parse(cJSON *current, t_track_info *data, int behaviour);
void ft_utils_order_compute(t_track_info *trackInfo, t_order *orders);
void ft_order_reenable(t_order *orders);
void ft_order_add(t_order *order, int type, int valueInt, double valueDouble, int status);
cJSON *ft_order_get_next(t_car_basic *carInfo, t_order *orders);


#endif /* FT_UTILS_H */
