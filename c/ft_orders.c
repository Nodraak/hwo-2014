/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 11:53:31
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-19 12:41:18
*/

#include "ft_orders.h"

/*****************
 *  TRACK PARSE  *
 *****************/

void ft_orders_track_parse(cJSON *data, t_track_info *track)
{
	cJSON *pieces = NULL;
	int i;

	pieces = ft_utils_field_find("pieces", data)->child;

	/* get how many pieces there is : nbElem */
	track->nbElem = ft_orders_piece_count(pieces);

	/* malloc */
	track->pieces = calloc(track->nbElem, sizeof(t_track_piece));
	if (track->pieces == NULL)
		error("Error malloc %d %s.\n", __LINE__, __FILE__);

	/* fill the malloc */
	ft_orders_piece_parse(pieces, track);

	/* calc total length of track - useless, but fun for make statistics */
	for (i = 0; i < track->nbElem; ++i)
		track->lengthTotal += track->pieces[i].length;

	printf("lengthTotal=%.1f - nbElem=%d\n", track->lengthTotal, track->nbElem);
}


/*
TRACK INFO :

					-----				 -----
		current -> |piece|   	 ->		|piece|       ->	NULL
					-----				 -----
					  v					   v
					 ---				 ------		 -----
				    |len| -> NULL		|radius| -> |angle| -> NULL
				     ---				 ------		 -----
note :
	-> : next
	v  : child
*/

int ft_orders_piece_count(cJSON *current)
{
	int n = 0;

	while (current != NULL)
	{
		n ++;
		current = current->next;
	}

	return n;
}

void ft_orders_piece_parse(cJSON *current, t_track_info *data)
{
	int id = 0;

	while (current != NULL && id < data->nbElem)
	{
		cJSON *piece = current->child;
		int len = 0, radius = 0, angle = 0, isSwitch = 0;

		/* get current piece info */
		while (piece != NULL)
		{
			if (strcmp(piece->string, "length") == 0)
				len = piece->valueint;
			else if (strcmp(piece->string, "radius") == 0)
				radius = piece->valueint;
			else if (strcmp(piece->string, "angle") == 0)
				angle = piece->valueint;
			else if (strcmp(piece->string, "switch") == 0)
				isSwitch = 1;

			piece = piece->next;
		}

		/* if (len != 0) => not curve */
		if (len != 0)
		{
			data->pieces[id].type = PIECE_TYPE_RIGHT;
			data->pieces[id].length = len;
		}
		else
		{
			if (ft_abs(angle) < ANGLE_CONSIDERED_AS_RIGHT)
				data->pieces[id].type = PIECE_TYPE_CURVE_LIGHT;
			else
				data->pieces[id].type = PIECE_TYPE_CURVE_HARD;

			data->pieces[id].length = 2.0 * M_PI * radius * ft_abs(angle) / 360;
			data->pieces[id].angle = angle;
			data->pieces[id].radius = radius;
		}
		data->pieces[id].isSwitch = isSwitch;

		id ++;
		current = current->next;
	}

	if (id != data->nbElem)
		error("nbElem not matching %d %s.\n", __LINE__, __FILE__);
}

/********************
 *  ORDERS COMPUTE  *
 ********************/

double ft_orders_max_speed_get(t_track_piece *piece)
{
	double radius = ft_abs(piece->radius);

	if (radius > 140)
		return SPEED_CURVE_RADIUS_150;
	else if (radius > 90)
		return SPEED_CURVE_RADIUS_100;
	else if (radius > 40)
		return SPEED_CURVE_RADIUS_50;
	else
		return SPEED_CURVE_RADIUS_0;
}

void ft_orders_compute(t_track_info *trackInfo, t_order *orders)
{
	int idOrder = 0, idTrack = 0, i;
	int nbOrder;
	int nbCurves = 0, idActualSwitch = 0, idCurrentPiece = 0;
	
	/***********
	 *  SPEED  *
	 ***********/
	/* initial speed */
	for (idTrack = 0; idTrack < trackInfo->nbElem; ++idTrack)
	{
		double speed = ft_orders_max_speed_get(&trackInfo->pieces[idOrder]);

		/* curve : slow down | right piece : accelerate */
		if (trackInfo->pieces[idTrack].type != PIECE_TYPE_RIGHT)
			ft_orders_add(&orders[idOrder], idOrder, ORDER_TYPE_SPEED, 0, speed, ORDER_STATUS_ENABLED);
		else
			ft_orders_add(&orders[idOrder], idOrder, ORDER_TYPE_SPEED, 0, 10, ORDER_STATUS_ENABLED);

		idOrder++;
	}
	nbOrder = idOrder;

	/* accelerate when incoming right */
	for (i = 0+1; i < nbOrder; ++i)
	{
		/* if right and curve before, increase before's speed */
		if (trackInfo->pieces[i].type == PIECE_TYPE_RIGHT && trackInfo->pieces[i-1].type != PIECE_TYPE_RIGHT)
			orders[i-1].valueDouble = 10;
	}

	/* slow down before the curves */
	for (i = nbOrder-1; i >= 0+1; --i)
	{
		double speedActual = orders[i].valueDouble;
		double speedPrevious = orders[i-1].valueDouble;

		/* if previous speed is too fast and the car will crash, decrease it */
		if (speedPrevious > speedActual + SPEED_LOST_PER_TRACK_PIECE)
			orders[i-1].valueDouble = speedActual + SPEED_LOST_PER_TRACK_PIECE;
	}

	/*****************
	 *  SWITCH LANE  *
	 *****************/
	i = nbOrder;
	/* note on curve : left = neg | right = pos */
	nbCurves = 0, idActualSwitch = 0, idCurrentPiece = 0;

	/* find first switch */
	while (trackInfo->pieces[idActualSwitch].isSwitch == 0)
		idActualSwitch ++;
	idCurrentPiece = idActualSwitch;

	/* set switch orders */
	while (idCurrentPiece <= 39)
	{
		nbCurves = 0;

		do {
			idCurrentPiece ++;

			if (trackInfo->pieces[idCurrentPiece].type != PIECE_TYPE_RIGHT)
			{
				if (trackInfo->pieces[idCurrentPiece].angle < 0)
					nbCurves --;
				else
					nbCurves ++;
			}
		} while (idCurrentPiece <= 39 && trackInfo->pieces[idCurrentPiece].isSwitch == 0);

		if (idCurrentPiece <= 39)
		{
			int order = nbCurves < 0 ? ORDER_SWITCH_LEFT : ORDER_SWITCH_RIGHT;

			ft_orders_add(&orders[i], idActualSwitch-0.5, ORDER_TYPE_SWITCH, order, 0, ORDER_STATUS_ENABLED);
			i ++;
			idActualSwitch = idCurrentPiece;
		}
	}
	nbOrder = i;

	/* disable unused slots - TODO : malloc only needed memory */
	for (i = nbOrder; i < MAX_ORDERS; ++i)
		ft_orders_add(&orders[i], 0, 0, 0, 0, ORDER_STATUS_DISABLED);
}

void ft_orders_reenable(t_order *orders)
{
	int i;

	for (i = 0; i < MAX_ORDERS; ++i)
	{
		if (orders[i].status == ORDER_STATUS_SENDED)
			orders[i].status = ORDER_STATUS_ENABLED;
	}
}


void ft_orders_add(t_order *order, double pos, int type, int valueInt, double valueDouble, int status)
{
	order->pos = pos;
	order->type = type;
	order->valueInt = valueInt;
	order->valueDouble = valueDouble;
	order->status = status;
}


cJSON *ft_orders_next_get(t_car_basic *carInfo, t_order *orders)
{
	cJSON *msg = NULL;
	t_order *ptr = NULL;
	int i;
	double newSpeed = 0; /* 0 to 1 values */

	if (orders == NULL)
		return NULL;

	/* find order */
	for (i = 0; i < MAX_ORDERS; ++i)
	{
		/* if current order is ok for : pos and status */
		if (carInfo->pos + SEND_ORDER_OFFSET >= orders[i].pos && orders[i].status == ORDER_STATUS_ENABLED)
		{
			ptr = &orders[i];
			ptr->status = ORDER_STATUS_SENDED;
			break;
		}
	}

	/* if order is found, handle it */
	if (ptr != NULL)
	{
		if (ptr->type == ORDER_TYPE_SPEED)
			carInfo->speedWanted = ptr->valueDouble;
	
		/* speed or switch lane - default == speed */
		if (ptr->type == ORDER_TYPE_SWITCH)
		{
			if (ptr->valueInt == ORDER_SWITCH_RIGHT)
				msg = make_msg("switchLane", cJSON_CreateString("Right")), printf("\t\torder switch right");
			else
				msg = make_msg("switchLane", cJSON_CreateString("Left")), printf("\t\torder switch left");
		}
		else if (ptr->type == ORDER_TYPE_SPEED)
		{
			if (ft_abs(carInfo->speedActual - carInfo->speedWanted) > SPEED_DIFF_EXTREM_VALUES) /* big diff, extrem values */
			{
				if (carInfo->speedActual > carInfo->speedWanted) /* 0 to 10 values */
					newSpeed = 0;
				else
					newSpeed = 1;
			}
			else
				newSpeed = carInfo->speedWanted/10;

			/* if angle > 50 and order to increase speed, dont order anything */
			if (ft_abs(carInfo->angle) > 50 &&  carInfo->speedWanted > carInfo->speedActual)
				return NULL;
			else
				msg = throttle_msg(newSpeed), printf("\t\torder %.2f -> %.2f", newSpeed, carInfo->speedWanted/10);
		}
	}

	return msg;
}

