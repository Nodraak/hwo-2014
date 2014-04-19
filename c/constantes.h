/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 12:09:18
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-19 12:33:59
*/

#ifndef CONSTANTES_H
#define CONSTANTES_H

/*
	notes : angles : left = neg | right = pos
*/

/*****************
 *  USER CONFIG  *
 *****************/

/*
	name of the track to join - define one and only one
*/
#define TRACK_NAME					"keimola"
/*#define TRACK_NAME					"germany"*/

/*
	number of player for the game
*/
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
#define SPEED_CURVE_RADIUS_0		1
#define SPEED_CURVE_RADIUS_50		3.8
#define SPEED_CURVE_RADIUS_100		6.2
#define SPEED_CURVE_RADIUS_150		8

/*
	when slowing down, speed diff per each track piece
	if the car crash engage the curve to quickly, decrease.
*/
#define SPEED_LOST_PER_TRACK_PIECE	2.0

/*
	Send order SEND_ORDER_OFFSET (unit : pieceIndex) before the car reach the asked pos
	if the car applys order too late, increase 
*/
#define SEND_ORDER_OFFSET			0.50

/*
	All curves with angle below this are seen as right
*/
#define ANGLE_CONSIDERED_AS_RIGHT	30

/*
	diff between current and wanted speed within the speed order are none (0) or full (1)
*/
#define SPEED_DIFF_EXTREM_VALUES	0.20

/*
	i like to recode everything
*/
#define ft_abs(n)					((n) < 0 ? -(n) : (n))

/**********
 *  ENUM  *
 **********/

enum e_piece_type
{
	PIECE_TYPE_RIGHT,
	PIECE_TYPE_CURVE_LIGHT,
	PIECE_TYPE_CURVE_HARD
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
	int				radius;
	int				isSwitch;
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

#define MAX_ORDERS	100 /* TODO : malloc this stuff */
typedef struct		s_order
{
	double			pos;
	int				type;
	int				valueInt;
	double			valueDouble;
	int				status;
}					t_order;

#endif /* CONSTANTES_H */
