/* 
* @Author: Adrien Chardon
* @Date:   2014-04-19 11:53:31
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-20 11:18:37
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
			data->pieces[id].type = PIECE_TYPE_CURVE;

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

double ft_orders_max_speed_get(t_track_piece *pieces, int id)
{
	double radius = ft_abs(pieces[id].radius);
	double ret;
	int angle = 0, i;

	if (pieces[id].type == PIECE_TYPE_RIGHT)
		ret = 10;
	else if (radius < 30)
		ret = SPEED_CURVE_RADIUS_0;
	else if (radius < 60)
		ret = SPEED_CURVE_RADIUS_50;
	else if (radius < 110)
		ret = SPEED_CURVE_RADIUS_100;
	else if (radius < 160)
		ret = SPEED_CURVE_RADIUS_150;
	else
		ret = SPEED_CURVE_RADIUS_200;

	/* if short curve, increase max speed */
	angle = pieces[id].angle, i = id+1;
	while (pieces[i].angle == pieces[id].angle)
		angle += pieces[i].angle, i++;

	if (ft_abs(angle) < 25) /* 22 for example */
		ret += 0.5;
	else if (ft_abs(angle) < 50) /* 45 for example */
		;
	else if (ft_abs(angle) < 100) /* 90 for example */
		ret -= 0.5;
	else if (ft_abs(angle) < 200) /* 180 for example */
		ret -= 1.0;

	/* check error */
	if (ret > 10)
		ret = 10;
	if (ret < 0)
		ret = 0;

	return ret;
}

int ft_orders_is_in_range(double pos, double start, double end)
{
	return (pos > start && pos < end);
}


void ft_orders_compute(t_track_info *trackInfo, t_order *orders)
{
	int idOrder = 0, idTrack = 0, i;
	int nbOrder;
	int nbCurves = 0, idActualSwitch = 0, idCurrentPiece = 0;
	int sortDone = 0;

	/***********
	 *  SPEED  *
	 ***********/
	for (idTrack = 0; idTrack < trackInfo->nbElem; ++idTrack)
	{
		/* if there is only 1 piece like the actual (right / curve), dont order anything */
		if (idTrack < trackInfo->nbElem-1
			&& trackInfo->pieces[idTrack].type == trackInfo->pieces[idTrack+1].type)
		{
			double speed = ft_orders_max_speed_get(trackInfo->pieces, idTrack);

			/* if the order has not been set yet, set it */
			if (idTrack == 0 || speed != orders[idOrder-1].valueDouble)
			{
				ft_orders_add(&orders[idOrder], idTrack, ORDER_TYPE_SPEED, 0, speed, ORDER_STATUS_ENABLED);
				idOrder ++;
			}
		}
	}
	nbOrder = idOrder;

	/* accelerate when incoming right */
	for (i = 0+1; i < nbOrder; ++i)
	{
		int current = orders[i].pos;
		int previous = orders[i-1].pos;

		/* if right and curve before, increase before's speed */
		if (trackInfo->pieces[current].type == PIECE_TYPE_RIGHT && trackInfo->pieces[previous].type == PIECE_TYPE_CURVE)
			orders[i].pos -= 0.5;
	}

	/* slow down before the curves */
	for (i = nbOrder-1; i >= 0; --i)
	{
		#define OFFSET 0.5
		double currentPos = orders[i].pos - OFFSET;

		while (ft_orders_is_in_range(currentPos, orders[i-1].pos, orders[i].pos))
		{
			double posDiff = orders[i].pos - currentPos;
			double speed = orders[i].valueDouble + posDiff * SPEED_LOST_PER_TRACK_PIECE;
			if (speed > 10)
				speed = 10;

			ft_orders_add(&orders[idOrder], currentPos, ORDER_TYPE_SPEED, 0, speed, ORDER_STATUS_ENABLED);
			idOrder ++;

			if (speed == 10)
				break;

			currentPos -= OFFSET;
		}
	}
	nbOrder = idOrder;

	/* sort orders */
	while (sortDone == 0)
	{
		sortDone = 1;

		for (i = 0; i < nbOrder-1; ++i)
		{
			if (orders[i].pos > orders[i+1].pos)
			{
				t_order tmp;
				memcpy(&tmp, &orders[i], sizeof(t_order));
				memcpy(&orders[i], &orders[i+1], sizeof(t_order));
				memcpy(&orders[i+1], &tmp, sizeof(t_order));

				sortDone = 0;
			}
		}
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
	while (idCurrentPiece <= trackInfo->nbElem)
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
		} while (idCurrentPiece <= trackInfo->nbElem && trackInfo->pieces[idCurrentPiece].isSwitch == 0);

		if (idCurrentPiece <= trackInfo->nbElem)
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

	/* if order is found, handle it, else check speed */

	if (ptr != NULL && ptr->type == ORDER_TYPE_SPEED)
		carInfo->speedWanted = ptr->valueDouble;

	/* speed or switch lane - default == speed */
	if (ptr != NULL && ptr->type == ORDER_TYPE_SWITCH)
	{
		if (ptr->valueInt == ORDER_SWITCH_RIGHT)
			msg = make_msg("switchLane", cJSON_CreateString("Right")), printf("\t\tswitch right");
		else
			msg = make_msg("switchLane", cJSON_CreateString("Left")), printf("\t\tswitch left");
	}
	else
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

		/* if angle > 50 and order to increase speed, dont order anything
			or if the speed order has yet been sended */
		if ((ft_abs(carInfo->angle) > 50 && carInfo->speedWanted > carInfo->speedActual)
			|| carInfo->lastSpeedOrder == newSpeed)
			return NULL;
		else
		{
			msg = throttle_msg(newSpeed);
			carInfo->lastSpeedOrder = newSpeed;
			printf("\t\t%.2f -> %.2f", newSpeed, carInfo->speedWanted/10);
		}
	}
	
	return msg;
}

