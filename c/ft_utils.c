/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-18 14:10:41
*/

#include "ft_utils.h"

/*
## notes ##

angle :
	left = neg | right = pos



work time :
	16 avril : 19h-2h -> 7h
	17 avril : 13h-23h (minus eat time : 2h) -> 8h

*/


/* pieceid || direction (-1 left | 0 none | +1 right) || done */
int switchLane[5][3] = {
	{3,1,0},
	{8,-1,0},
	{13,0,0},
	{18,1,0},
	{25,0,0}
};

/********************
 *  MAIN FUNCTIONS  *
 ********************/

void ft_main_loop(int sock)
{
	cJSON *json = NULL;
	cJSON *msg = NULL, *msg_type = NULL;
	char *msg_type_name = NULL;
	t_car_basic all = {0, 0, 0, 0, 0, 0};
	t_track_info *trackInfo = NULL;
	t_order *orders = NULL;
	int isRaceStarted = 0;

	/* init */
	orders = calloc(MAX_ORDERS, sizeof(t_order));
	if (orders == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);
	trackInfo = calloc(1, sizeof(t_track_info));
	if (trackInfo == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);

	/* main loop */
	while ((json = read_msg(sock)) != NULL)
	{
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		/* handle recieved data and updated info */
		ft_utils_data_parse(json, &all, trackInfo, orders);

		/* prepare data */
		msg_type_name = msg_type->valuestring;
		if (strcmp("gameStart", msg_type_name) == 0)
		{
			isRaceStarted = 1;
		}
		else if (!strcmp("carPositions", msg_type_name))
		{
			if (isRaceStarted)
				msg = ft_order_get_next(&all, orders);
		
			/*double speed = ft_utils_get_speed(&all);
			cJSON *switchOrder = ft_utils_get_switch(&all);
			
			if (switchOrder != NULL)
				msg = switchOrder;
			else
				msg = throttle_msg(speed);*/
		}
		else
		{
			log_message(msg_type_name, json);
		}

		/* send data */
		if (msg == NULL) /* nothing to say -> ping */
				msg = ping_msg();
		write_msg(sock, msg);

		cJSON_Delete(msg), msg = NULL;
		cJSON_Delete(json), json = NULL;
	}

	printf("Read null msg, quitting ...\n");
}



void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders)
{
	cJSON *data = NULL;
	char *msgType = NULL;
	char *s = NULL;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "join") == 0)
	{
		printf("==> JOIN\n");
	}
	else if (strcmp(msgType, "yourCar") == 0)
	{
		printf("==> YOUR CAR (color=%s)\n", cJSON_GetObjectItem(data, "color")->valuestring);
		s = cJSON_Print(json);
		printf("%s\n", s);
		free(s);
	}
	else if (strcmp(msgType, "gameInit") == 0)
	{
		cJSON *name = NULL;
		int i;

		printf("==> GAME INIT\n");

		ft_utils_track_parse(data, trackInfo);
		name = ft_utils_field_find("name", json);

		/* load from file */
		if (name != NULL && strcmp(name->valuestring, "Keimola") == 0)
		{
			FILE *f = NULL;
			int valueInt, idOrder = 0, orderType;
			double pos, valueDouble;

			printf("Info : knowned race, loading file ...\n");
			f = fopen("raceData/Keimola.txt", "r");
			if (f == NULL)
				error("Error fopen %d %s\n", __LINE__, __FILE__);
			while (!feof(f))
			{
				int ret = fscanf(f, "%lf\t%d\t%lf\t%d", &pos, &orderType, &valueDouble, &valueInt);
				if (ret == 4)
				{
					ft_order_add(&orders[idOrder], pos, orderType, valueInt, valueDouble, ORDER_STATUS_ENABLED);
					idOrder++;
				}
				else
				{
					while (idOrder < MAX_ORDERS)
					{
						ft_order_add(&orders[idOrder], 0, 0, 0, 0, ORDER_STATUS_DISABLED);
						idOrder++;
					}
					break;
				}
			}
		}
		else /* guess */
		{
			printf("Info : unknowned race, guessing from data ...\n");

			ft_utils_order_compute(trackInfo, orders);
		}

		printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
			ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
		for (i = 0; orders[i].status != ORDER_STATUS_DISABLED; ++i)
			printf("pos=%.2f\ttype=%d\tvalueD=%.1f\tvalueI=%d\n", orders[i].pos, orders[i].type, orders[i].valueDouble, orders[i].valueInt);
		printf("\n");
	}
	else if (strcmp(msgType, "gameStart") == 0)
	{
		printf("==> GAME START\n");
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		/*ft_utils_data_raw_print("CAR POS", data);*/
		ft_update_car_data(data, all, trackInfo);
		printf("pos=%3.2f | speed=%2.2f angle=%2.1f\n",
				all->pos, all->speedActual, all->angle);
	}
	else if (strcmp(msgType, "gameEnd") == 0)
	{
		ft_utils_data_raw_print("GAME END", data);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		printf("==> New lap\n");
		ft_utils_data_raw_print(msgType, json);
		ft_order_reenable(orders);
	}
	else
	{
		ft_utils_data_raw_print(msgType, json);
	}

	/*data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		printf("gameTick=%d\n", data->valueint);*/
}



double my_abs(double n)
{
	return (n < 0 ? -n : n);
}


/****************
 *  CALC  ORDER *
 ****************/
#if 0

cJSON *ft_utils_get_switch(t_car_basic *all)
{
	cJSON *msg = NULL;
	int i;

	for (i = 0; i < 5; ++i)
	{
		/* send order 1 piece before, else it is too late for the wanted and it switch the next */
		if (switchLane[i][0] == all->pieceIndex+1 && switchLane[i][2] == 0)
		{
			switchLane[i][2] = 1;
			if (switchLane[i][1] == -1)
				msg = make_msg("switchLane", cJSON_CreateString("Left"));
			else if (switchLane[i][1] == 1)
				msg = make_msg("switchLane", cJSON_CreateString("Right"));
		}
	}

	return msg;
}
#endif


/*****************
 *  UPDATE DATA  *
 *****************/

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo)
{
	cJSON *ret = NULL;
	static double old = 0;
	double speed;

	/* pieceIndex */
	ret = ft_utils_field_find("pieceIndex", data);
	if (ret == NULL)
		exit(45);
	all->pieceIndex = ret->valueint;

	/* inPieceDistance */
	ret = ft_utils_field_find("inPieceDistance", data);
	if (ret == NULL)
		exit(46);
	all->inPieceDistance = ret->valuedouble;

	/* angle */
	ret = ft_utils_field_find("angle", data);
	if (ret == NULL)
		exit(47);
	all->angle = ret->valuedouble;

	/* speed */
	speed = all->inPieceDistance - old;
	old = all->inPieceDistance;
	
	if (speed > 0)
		all->speedActual = speed;

	all->pos = all->pieceIndex + all->inPieceDistance / trackInfo->pieces[all->pieceIndex].length;
}

/***********
 *  UTILS  *
 ***********/

void ft_utils_data_raw_print(char *type, cJSON *data)
{
	char *s = cJSON_Print(data);
	printf("==> %s\n%s\n", type, s);
	free(s);
}


/* find first matching s */
cJSON *ft_utils_field_find(char *s, cJSON* head)
{
	cJSON *current = head, *ret = NULL;

	if (s == NULL)
		return NULL;

	while (current != NULL)
	{
		if (current->string != NULL && strcmp(current->string, s) == 0)
			return current;

		if (current->child != NULL)
		{
			ret = ft_utils_field_find(s, current->child);
			if (ret != NULL && ret->string != NULL && strcmp(ret->string, s) == 0)
				return ret;
		}

		current = current->next;
	}

	return NULL;
}

/*****************
 *  TRACK PARSE  *
 *****************/

void ft_utils_track_parse(cJSON *data, t_track_info *track)
{
	cJSON *current = NULL;
	int i;

	current = ft_utils_field_find("pieces", data)->child;

	/* get nbElem */
	ft_utils_piece_parse(current, track, PIECE_PARSE_COUNT);

	/* malloc */
	track->pieces = calloc(track->nbElem, sizeof(t_track_piece));
	if (track->pieces == NULL)
		error("Error malloc (%d elem) %d %s.\n", track->nbElem, __LINE__, __FILE__);

	/* build */
	track->nbElem = 0;
	ft_utils_piece_parse(current, track, PIECE_PARSE_BUILD);

	for (i = 0; i < track->nbElem; ++i)
		track->lengthTotal += track->pieces[i].length;

	printf("lengthTotal=%.1f - nbElem=%d\n", track->lengthTotal, track->nbElem);
}

void ft_utils_piece_parse(cJSON *current, t_track_info *data, int behaviour)
{
	if (behaviour == PIECE_PARSE_COUNT)
	{
		while (current != NULL)
		{
			data->nbElem ++;
			current = current->next;
		}
	}
	else if (behaviour == PIECE_PARSE_BUILD)
	{
/*
		-> next
		v child

		current -> |---|    ->    |---|            ->         |---| -> NULL
					 v				v
				   |len| -> NULL   |radius| -> |angle| -> NULL
*/
		while (current != NULL)
		{
			cJSON *piece = current->child;
			int len = 0, radius = 0, angle = 0;

			while (piece != NULL)
			{
				if (strcmp(piece->string, "length") == 0)
					len = piece->valueint;
				else if (strcmp(piece->string, "radius") == 0)
					radius = piece->valueint;
				else if (strcmp(piece->string, "angle") == 0)
					angle = piece->valueint;

				piece = piece->next;
			}

			if (len != 0)
			{
				data->pieces[data->nbElem].type = PIECE_TYPE_RIGHT;
				data->pieces[data->nbElem].length = len;
			}
			else
			{
				data->pieces[data->nbElem].type = PIECE_TYPE_CURVE;
				data->pieces[data->nbElem].length = (double)2 * M_PI * radius * abs(angle) / 360;
			}

			data->nbElem ++;
			current = current->next;
		}
	}
}

void ft_utils_order_compute(t_track_info *trackInfo, t_order *orders)
{
	int idOrder = 0, idTrack = 0, nbCurve = 0, haveAccelerate = 0;
#ifndef DISABLE_ORDERS
	/* initial starting order */
	ft_order_add(&orders[idOrder], 0, ORDER_TYPE_SPEED, 0, 10, ORDER_STATUS_ENABLED);
	idOrder++;

	for (idTrack = 0; idTrack < trackInfo->nbElem; ++idTrack)
	{
		if (trackInfo->pieces[idTrack].type == PIECE_TYPE_CURVE)
		{
			nbCurve ++;
			/* if nbCurve == 3, big curve, add slow down to prevent crash */
			if (nbCurve == 2)
			{
				ft_order_add(&orders[idOrder], idTrack-1-DISTANCE_TO_SLOW_DOWN, ORDER_TYPE_SPEED, 0, SPEED_DURING_TURN, ORDER_STATUS_ENABLED);
				idOrder++;
				haveAccelerate = 0;
			}
		}
		else
		{
			nbCurve = 0;
			/* add accelerate, if not yet added */
			if (haveAccelerate == 0)
			{
				if (idTrack != 0) /* first yet added */
				{
					ft_order_add(&orders[idOrder], idTrack-ACC_DISTANCE, ORDER_TYPE_SPEED, 0, 10, ORDER_STATUS_ENABLED);
					idOrder++;
				}
				haveAccelerate = 1;
			}
		}
	}

	ft_order_add(&orders[idOrder], 3-1, ORDER_TYPE_SWITCH, ORDER_SWITCH_RIGHT, 0, ORDER_STATUS_ENABLED);
	idOrder++;
	ft_order_add(&orders[idOrder], 8-1, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, 0, ORDER_STATUS_ENABLED);
	idOrder++;
	ft_order_add(&orders[idOrder], 18-1, ORDER_TYPE_SWITCH, ORDER_SWITCH_RIGHT, 0, ORDER_STATUS_ENABLED);
	idOrder++;
#else
ft_order_add(&orders[idOrder], 0, ORDER_TYPE_SPEED, 0, SPEED_DURING_TURN, ORDER_STATUS_ENABLED);
idOrder++;
#endif
	while (idOrder < MAX_ORDERS)
	{
		ft_order_add(&orders[idOrder], 0, 0, 0, 0, ORDER_STATUS_DISABLED);
		idOrder++;
	}
}

void ft_order_reenable(t_order *orders)
{
	int i;

	for (i = 0; i < MAX_ORDERS; ++i)
	{
		if (orders[i].status == ORDER_STATUS_SENDED)
			orders[i].status = ORDER_STATUS_ENABLED;
	}
}


void ft_order_add(t_order *order, double pos, int type, int valueInt, double valueDouble, int status)
{
	order->pos = pos;
	order->type = type;
	order->valueInt = valueInt;
	order->valueDouble = valueDouble;
	order->status = status;
}


cJSON *ft_order_get_next(t_car_basic *carInfo, t_order *orders)
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
		if (orders[i].pos - carInfo->pos < 0.05 && orders[i].status == ORDER_STATUS_ENABLED)
		{
			ptr = &orders[i];
			ptr->status = ORDER_STATUS_SENDED;
			break;
		}
	}

	/* if order is found, handle it */
	if (ptr != NULL && ptr->type == ORDER_TYPE_SPEED)
		carInfo->speedWanted = ptr->valueDouble;
	
	/* speed or switch lane - default == speed */
	if (ptr != NULL && ptr->type == ORDER_TYPE_SWITCH)
	{
		if (ptr->valueInt == ORDER_SWITCH_RIGHT)
			msg = make_msg("switchLane", cJSON_CreateString("Right")), printf("\t\t\t\tswitch right\n");
		if (ptr->valueInt == ORDER_SWITCH_LEFT)
			msg = make_msg("switchLane", cJSON_CreateString("Left")), printf("\t\t\t\tswitch left\n");
	}
	else
	{
		if (my_abs(carInfo->speedActual - carInfo->speedWanted) > 0.10) /* big diff, extrem values */
		{
			if (carInfo->speedActual > carInfo->speedWanted) /* 0 to 10 values */
				newSpeed = 0;
			else
				newSpeed = 1;
		}
		else
			newSpeed = carInfo->speedWanted/10;

		msg = throttle_msg(newSpeed), printf("\t\t\t\tspeed %.2f -> %.2f\n", newSpeed, carInfo->speedWanted/10);
	}

	return msg;
}

