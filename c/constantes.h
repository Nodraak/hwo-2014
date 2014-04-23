/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 12:09:18
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-26 20:10:30
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
#define ID_TRACK						1

#if (ID_TRACK == 1)
	#define TRACK_NAME					"keimola"
#elif (ID_TRACK == 2)
	#define TRACK_NAME					"germany"
#elif (ID_TRACK == 3)
	#define TRACK_NAME					"usa"
#elif (ID_TRACK == 4)
	#define TRACK_NAME					"france"
#endif
/*
	number of player for the game
*/
#define NB_PLAYER					1

/*
	print car pos every PRINT_CAR_POS_MODULO game ticks
	big number like 3000 to desactivate
*/
#define PRINT_CAR_POS_MODULO		500


/*************
 *  DEFINES  *		note : EDIT AT YOUR OWN RISK
 *************/

/*
	maximum speed in curve, if the car crash, decrease. (unit : between 0 and 10)
*/
#define SPEED_CURVE_RADIUS_0		2
#define SPEED_CURVE_RADIUS_50		4
#define SPEED_CURVE_RADIUS_100		6
#define SPEED_CURVE_RADIUS_150		8
#define SPEED_CURVE_RADIUS_200		9

/*
	when slowing down, speed diff per each track piece
	if the car crash engage the curve to quickly, decrease.
*/
#define SPEED_LOST_PER_TRACK_PIECE	2

/*
	Send order SEND_ORDER_OFFSET (unit : pieceIndex) before the car reach the asked pos
	if the car applys order too late, increase 
*/
#define SEND_ORDER_OFFSET			0.0

/*
	if the diff between current and wanted speed is greater than this, the speed
	order will be none (0) or full (1)
*/
#define SPEED_DIFF_EXTREM_VALUES	0.10

/*
	Max angle below which the speed is updated
*/
#define UPDATE_SPEED_MAX_ANGLE_UPDATE_BIG	50
#define UPDATE_SPEED_MAX_ANGLE_UPDATE_SLOW1	30
#define UPDATE_SPEED_MAX_ANGLE_UPDATE_SLOW2	45

/*
	i like to recode everything
*/
#define ft_abs(n)					((n) < 0 ? -(n) : (n))

/*
	debug stuff
*/
#ifndef NOT_AUTO_BUILD
	#define ft_log_ft_name(s)			printf(">>>>> %s %s - %d %s\n", s, __FUNCTION__, __LINE__, __FILE__)
#else
	#define ft_log_ft_name(s)			;
#endif

/**********
 *  ENUM  *
 **********/

typedef enum	e_game_status
{
	GAME_STATUS_WAITING,
	GAME_STATUS_QUALIF_START,
	GAME_STATUS_QUALIF_END,
	GAME_STATUS_RACE_START,
	GAME_STATUS_RACE_END,
	GAME_STATUS_TOURNAMENT_END
}				t_game_status;

typedef enum	e_piece_type
{
	PIECE_TYPE_RIGHT,
	PIECE_TYPE_CURVE
}				t_piece_type;

typedef enum	e_order_type
{
	ORDER_TYPE_SPEED,
	ORDER_TYPE_SWITCH
}				t_order_type;

typedef enum	e_order_status
{
	ORDER_STATUS_ENABLED,
	ORDER_STATUS_DISABLED,
	ORDER_STATUS_SENDED
}				t_order_status;

typedef enum	e_switch_type
{
	ORDER_SWITCH_RIGHT,
	ORDER_SWITCH_LEFT
}				t_switch_type;


/*************
 *  STRUCT  *
 *************/
typedef struct		s_track_piece
{
	/* pre-computed */
	t_piece_type	type;
	double 			length;
	int				angle;
	int				radius;
	int				isSwitch;
	/* live-computed */
	int				maxAngle;
}					t_track_piece;

typedef struct		s_track_info
{
	double 			lengthTotal;
	int				nbElem;
	t_track_piece	*pieces;
}					t_track_info;

typedef struct		s_car_info
{
	double			pos;
	int				lap;

	int				pieceIndex;
	double			inPieceDistance;
	double			speedActual;
	double			speedWanted;
	double			speedOld;
	double			lastSpeedOrder;
	double			angle;
}					t_car_info;

#define MAX_ORDERS	300 /* TODO : malloc this stuff */
typedef struct		s_order
{
	double			pos;
	t_order_type	type;
	t_switch_type	switchDir;
	double			speed;
	t_order_status	status;
}					t_order;

typedef struct		s_data
{
	t_car_info		*carInfo;
	t_track_info	*trackInfo;
	t_order			*orders;
	t_game_status	gameStatus;
	char			*trackDataPath;
	char 			*botName;
}					t_data;

extern int quit;

#endif /* CONSTANTES_H */
