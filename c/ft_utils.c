/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-18 20:31:57
*/

#include "ft_utils.h"

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
		}

		/* send data */
		if (msg == NULL) /* nothing to say -> ping */
				msg = ping_msg();
		write_msg(sock, msg);

		cJSON_Delete(msg), msg = NULL;
		cJSON_Delete(json), json = NULL;

		printf("\n"); /* flush all output for this game tick */
	}

	printf("Read null msg, quitting ...\n");
}



void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders)
{
	cJSON *data = NULL;
	char *msgType = NULL;
	int tick = 0;

	data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		tick = data->valueint;
	printf("tick=%d", tick);

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "join") == 0)
	{
		ft_utils_data_raw_print("JOIN", json);
	}
	else if (strcmp(msgType, "yourCar") == 0)
	{
		ft_utils_data_raw_print("YOUR CAR", json);
	}
	else if (strcmp(msgType, "gameInit") == 0)
	{
		cJSON *name = NULL;
		int i;

		ft_utils_data_raw_print("GAME INIT", json);

		ft_utils_track_parse(data, trackInfo);

		printf("Track : (right=%d curve=%d)\n", PIECE_TYPE_RIGHT, PIECE_TYPE_CURVE);
		for (i = 0; i < trackInfo->nbElem; ++i)
			printf("%d - type=%d length%.3f radius=%d\n",
				i, trackInfo->pieces[i].type , trackInfo->pieces[i].length , trackInfo->pieces[i].angle);
		printf("\n");

		name = ft_utils_field_find("name", json);

		ft_utils_order_compute(trackInfo, orders);

		printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
			ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
		for (i = 0; orders[i].status != ORDER_STATUS_DISABLED; ++i)
			printf("pieceIndex=%d\ttype=%d\tvalueD=%.1f\tvalueI=%d\n", i, orders[i].type, orders[i].valueDouble, orders[i].valueInt);
		printf("\n");
	}
	else if (strcmp(msgType, "gameStart") == 0)
	{
		ft_utils_data_raw_print("GAME START", json);
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		if (tick % PRINT_CAR_POS_MODULO == 0)
			ft_utils_data_raw_print("CAR POS", json);

		ft_update_car_data(data, all, trackInfo);
		printf("\t\tpos=%3.2f spe=%2.2f ang=%2.1f",
				all->pos, all->speedActual, all->angle);
	}
	else if (strcmp(msgType, "gameEnd") == 0)
	{
		ft_utils_data_raw_print("GAME END", json);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		ft_utils_data_raw_print("NEW LAP", json);
		ft_order_reenable(orders);
	}
	else
	{
		ft_utils_data_raw_print(msgType, json);
	}
}



double my_abs(double n)
{
	return (n < 0 ? -n : n);
}


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
	printf("==> %s\n%s\n\n", type, s);
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
				if (my_abs(angle) > ANGLE_CONSIDERED_AS_RIGHT)
					data->pieces[data->nbElem].type = PIECE_TYPE_CURVE;
				else
					data->pieces[data->nbElem].type = PIECE_TYPE_RIGHT;
				data->pieces[data->nbElem].length = (double)2 * M_PI * radius * abs(angle) / 360;
				data->pieces[data->nbElem].angle = my_abs(angle);
			}

			data->nbElem ++;
			current = current->next;
		}
	}
}

void ft_utils_order_compute(t_track_info *trackInfo, t_order *orders)
{
	int idOrder = 0, idTrack = 0, i;
	int nbOrder;

	/***********
	 *  SPEED  *
	 ***********/
	for (idTrack = 0; idTrack < trackInfo->nbElem; ++idTrack)
	{
		/* curve : slow down */
		if (trackInfo->pieces[idTrack].type == PIECE_TYPE_CURVE)
		{
			ft_order_add(&orders[idOrder], ORDER_TYPE_SPEED, 0, SPEED_CURVE_HARD, ORDER_STATUS_ENABLED);
			idOrder++;
		}
		/* right piece : accelerate */
		else
		{
			ft_order_add(&orders[idOrder], ORDER_TYPE_SPEED, 0, 10, ORDER_STATUS_ENABLED);
			idOrder++;
		}
	}
	nbOrder = idOrder;

	/* parse from first to last to accelerate when incoming right */
	for (i = 0+1; i < nbOrder; ++i)
	{
		/* if right and curve before, increase before's speed */
		if (trackInfo->pieces[i].type == PIECE_TYPE_RIGHT && trackInfo->pieces[i-1].type == PIECE_TYPE_CURVE)
			orders[i-1].valueDouble = 10;
	}

	/* parse from last to first order to slow down before the curve */
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
	





	/* disable unused slots - TODO : malloc only needed memory */
	for (i = nbOrder; i < MAX_ORDERS; ++i)
		ft_order_add(&orders[i], 0, 0, 0, ORDER_STATUS_DISABLED);
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


void ft_order_add(t_order *order, int type, int valueInt, double valueDouble, int status)
{
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
		if (carInfo->pos + SEND_ORDER_OFFSET >= i && orders[i].status == ORDER_STATUS_ENABLED)
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
			msg = make_msg("switchLane", cJSON_CreateString("Right")), printf("\t\torder switch right");
		if (ptr->valueInt == ORDER_SWITCH_LEFT)
			msg = make_msg("switchLane", cJSON_CreateString("Left")), printf("\t\torder switch left");
	}
	else
	{
		if (my_abs(carInfo->speedActual - carInfo->speedWanted) > SPEED_DIFF_EXTREM_VALUES) /* big diff, extrem values */
		{
			if (carInfo->speedActual > carInfo->speedWanted) /* 0 to 10 values */
				newSpeed = 0;
			else
				newSpeed = 1;
		}
		else
			newSpeed = carInfo->speedWanted/10;

		/* if angle > 50 and order to increase speed, dont order anything */
		if (my_abs(carInfo->angle) > 50 &&  carInfo->speedWanted > carInfo->speedActual)
			return NULL;

		msg = throttle_msg(newSpeed), printf("\t\torder %.2f -> %.2f", newSpeed, carInfo->speedWanted/10);
	}

	return msg;
}

