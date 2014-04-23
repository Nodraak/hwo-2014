/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-23 12:36:30
*/

#include "ft_utils.h"

void ft_orders_update(t_order *orders, t_track_info *trackInfo);


/********************
 *  MAIN FUNCTIONS  *
 ********************/

void ft_main_loop(int sock)
{
	cJSON *json = NULL;
	cJSON *msg = NULL, *msg_type = NULL;
	t_data data = {NULL, NULL, NULL, GAME_STATUS_WAITING, NULL};

	/* init */
	data.carInfo = calloc(1, sizeof(t_car_info));
	if (data.carInfo == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);
	data.trackInfo = calloc(1, sizeof(t_track_info));
	if (data.trackInfo == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);
	data.orders = calloc(MAX_ORDERS, sizeof(t_order));
	if (data.orders == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);

	/* main loop */
	while ((json = read_msg(sock)) != NULL)
	{
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		/* handle recieved data and updated info */
		ft_main_data_parse(json, &data);

		/* prepare and send response */
		msg = ft_main_msg_make(msg_type, &data);
		write_msg(sock, msg);

		/* clean for next loop */
		cJSON_Delete(msg), msg = NULL;
		cJSON_Delete(json), json = NULL;

		printf("\n"); /* flush all output for this loop / game tick */
	}

	printf("Read null msg, \n");
	if (data.gameStatus == GAME_STATUS_TOURNAMENT_END)
		printf("clean exit, writing orders to file.\n");
	else
		printf("dirty exit, dont write anything.\n");

	/* save orders to file */
	ft_orders_file_save_to(data.orders, data.trackDataPath);

	/* free every malloc */
	free(data.trackInfo->pieces);
	free(data.carInfo);
	free(data.trackInfo);
	free(data.orders);
}

void ft_main_data_parse(cJSON *json, t_data *data)
{
	cJSON *msgData = NULL;
	char *msgType = NULL;
	int tick = -1;

	msgData = cJSON_GetObjectItem(json, "gameTick");
	if (msgData)
		tick = msgData->valueint;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	msgData = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "gameInit") == 0)
	{
		/* parse track info and get orders */
		ft_init_track_parse(msgData, data->trackInfo);
		ft_init_get_orders(data, msgData);

		/* printf for debug */
		ft_print_raw_data("GAME INIT", json);
		ft_print_gameInit_data(data);
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		ft_update_car_data(msgData, data);

		/* printf for debug */
		if (tick % PRINT_CAR_POS_MODULO == 0 && tick != -1)
			ft_print_raw_data("CAR POS", json);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		int i;

		ft_orders_reenable(data->orders);
		if (data->carInfo->lap > 0)
			ft_orders_update(data->orders, data->trackInfo);
		data->carInfo->lap++;

		ft_print_lapFinished(data, json);

		for (i = 0; i < data->trackInfo->nbElem; ++i)
			data->trackInfo->pieces[i].maxAngle = 0;
	}
	else if (strcmp(msgType, "spawn") == 0)
	{
		data->carInfo->speedWanted = 5;
		/* TODO */
	}
	else if (strcmp(msgType, "tournamentEnd") == 0)
	{
		data->gameStatus = GAME_STATUS_TOURNAMENT_END;
	}
	else
	{
		ft_print_raw_data(msgType, json);
	}

	/* printf for debug */
	if (tick % 20 == 0)
		printf("gameTicks\tpos\tspeed\twanted\tangle\t\torder\n");
	if (tick != -1)
		printf("%d\t\t%.2f\t%.1f\t%.1f\t%.1f", tick, data->carInfo->pos,
			data->carInfo->speedActual, data->carInfo->speedWanted, data->carInfo->angle);
}


void ft_orders_update(t_order *orders, t_track_info *trackInfo)
{
	int i;
	double speed;

	for (i = 0; i < trackInfo->nbElem; ++i)
	{
		if (trackInfo->pieces[i].maxAngle < 50 && orders[i*2].speed < 10)
		{
			speed = (50.0 - trackInfo->pieces[i].maxAngle) / 100.0;
			/*orders[i*2].speed += speed;*/

			printf("%d %.2f\n", i, speed);
		}
	}


#if 0

	/* slow down before the curves */
	for (i = nbOrder-1; i >= 0+1; --i)
	{
		if (orders[i-1].speed > orders[i].speed + 0.5*SPEED_LOST_PER_TRACK_PIECE)
			orders[i-1].speed = orders[i].speed + 0.5*SPEED_LOST_PER_TRACK_PIECE;
	}


	/*
	for (i = 0; i < data->trackInfo->nbElem; ++i)
	{
		if (data->trackInfo->pieces[i].type == PIECE_TYPE_RIGHT)
			printf("   %d\n", data->trackInfo->pieces[i].maxAngle);
		else
			printf("%d %d\n", i, data->trackInfo->pieces[i].maxAngle);
	}*/
#endif
}


cJSON *ft_main_msg_make(cJSON *msgType, t_data *data)
{
	cJSON *msg = NULL;
	char *msgTypeName = NULL;

	msgTypeName = msgType->valuestring;
	if (strcmp("gameStart", msgTypeName) == 0)
	{
		if (data->gameStatus == GAME_STATUS_WAITING)
			data->gameStatus = GAME_STATUS_QUALIF_START;
		else if (data->gameStatus == GAME_STATUS_QUALIF_END)
			data->gameStatus = GAME_STATUS_RACE_START;
		else
			printf("Unexpected \"gameStart\" recieved\n");

		msg = throttle_msg(1);
		printf("--> order initial speed : 1 (gameStart)\n");
	}
	else if (strcmp("gameEnd", msgTypeName) == 0)
	{
		if (data->gameStatus == GAME_STATUS_QUALIF_START)
			data->gameStatus = GAME_STATUS_QUALIF_END;
		else if (data->gameStatus == GAME_STATUS_RACE_START)
			data->gameStatus = GAME_STATUS_RACE_END;
		else
			printf("Unexpected \"gameEnd\" recieved\n");
	}
	else if (strcmp("carPositions", msgTypeName) == 0)
	{
		if (data->gameStatus == GAME_STATUS_QUALIF_START
			|| data->gameStatus == GAME_STATUS_RACE_START)
			msg = ft_orders_next_get(data->carInfo, data->orders);
	}

	if (msg == NULL) /* nothing to say -> ping */
		msg = ping_msg();

	return msg;
}

void ft_init_get_orders(t_data *data, cJSON *msgData)
{
	FILE *f = NULL;
	char *trackName = NULL;

	trackName = ft_trackName_get(msgData);
	printf("## trackName=\"%s\".\n", trackName);
	if (trackName != NULL)
	{
		char tmp[1024];
		sprintf(tmp, "./%s.orders", trackName);
		data->trackDataPath = strdup(tmp);
		f = fopen(data->trackDataPath, "r");
	}
	if (trackName == NULL || f == NULL)
	{
		printf("## Unknown track, guessing.\n");
		ft_orders_compute(data->trackInfo, data->orders);
	}
	else
	{
		printf("## Known track, loading from file \"%s\".\n", data->trackDataPath);
		ft_orders_file_load_from(data->orders, f);
		fclose(f);
	}
}

/*****************
 *  UPDATE DATA  *
 *****************/

void ft_update_car_data(cJSON *msgData, t_data *data)
{
	static double oldInPieceDistance = 0, oldAngle = 0;
	double speed;
	cJSON *ret = NULL;

	/* pieceIndex */
	ret = ft_utils_field_find("pieceIndex", msgData);
	if (ret == NULL)
		exit(45);
	data->carInfo->pieceIndex = ret->valueint;

	/* inPieceDistance */
	ret = ft_utils_field_find("inPieceDistance", msgData);
	if (ret == NULL)
		exit(46);
	data->carInfo->inPieceDistance = ret->valuedouble;

	/* angle */
	ret = ft_utils_field_find("angle", msgData);
	if (ret == NULL)
		exit(47);
	data->carInfo->angle = ret->valuedouble;

	/* update max angle */
	if (ft_abs(data->carInfo->angle) > data->trackInfo->pieces[(int)data->carInfo->pos].maxAngle)
		data->trackInfo->pieces[(int)data->carInfo->pos].maxAngle = ft_abs(data->carInfo->angle);

	/* speed */
	speed = data->carInfo->inPieceDistance - oldInPieceDistance;
	
	if (speed > 0)
		data->carInfo->speedActual = speed;

	data->carInfo->pos = data->carInfo->pieceIndex +
							data->carInfo->inPieceDistance / data->trackInfo->pieces[data->carInfo->pieceIndex].length;

	/* prepare for next loop */
	oldInPieceDistance = data->carInfo->inPieceDistance;
	oldAngle = data->carInfo->angle;
}

char *ft_trackName_get(cJSON *data)
{
	cJSON *race, *track, *id;

	race = cJSON_GetObjectItem(data, "race");
	if (race == NULL)
		return NULL;

	track = cJSON_GetObjectItem(race, "track");
	if (track == NULL)
		return NULL;

	id = cJSON_GetObjectItem(track, "id");
	if (id == NULL)
		return NULL;
	
	return id->valuestring;
}

/***********
 *  UTILS  *
 ***********/

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

/***********
 *  PRINT  *
 ***********/

void ft_print_raw_data(char *type, cJSON *data)
{
	char *s = cJSON_Print(data);
	printf("\n==> %s\n%s\n", type, s);
	free(s);
}


void ft_print_gameInit_data(t_data *data)
{
	int i;

	/* track data */
	printf("Track : (right=%d curve=%d)\n", PIECE_TYPE_RIGHT, PIECE_TYPE_CURVE);
	printf("i\ttype\tlen\tangle\n");
	for (i = 0; i < data->trackInfo->nbElem; ++i)
	{
		printf("%d\t%d\t%.1f\t%d\n", i, data->trackInfo->pieces[i].type,
			data->trackInfo->pieces[i].length , data->trackInfo->pieces[i].angle);
	}
	printf("\n");

	/* orders data */
	printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
		ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
	printf("pos\ttype\tspeed\tswitch\n");
	for (i = 0; i < MAX_ORDERS; ++i)
	{
		if (data->orders[i].status != ORDER_STATUS_DISABLED)
		{
			printf("%.1f\t%d\t%.1f\t%d\n", data->orders[i].pos,
				data->orders[i].type, data->orders[i].speed, data->orders[i].switchDir);
		}
	}
	printf("\n");
}

void ft_print_lapFinished(t_data *data, cJSON *json)
{
	int i;

	ft_print_raw_data("NEW LAP", json);

	printf("maxAngle msgdata :\n");
	for (i = 0; i < data->trackInfo->nbElem; ++i)
	{
		if (data->trackInfo->pieces[i].type == PIECE_TYPE_RIGHT)
			printf("   %d\n", data->trackInfo->pieces[i].maxAngle);
		else
			printf("%d %d\n", i, data->trackInfo->pieces[i].maxAngle);
	}

	printf("speed orders :\npos\tspeed\n");
	for (i = 0; data->orders[i].type == ORDER_TYPE_SPEED; ++i)
	{
		printf("%.3f\t%.1f\n",
			data->orders[i].pos, data->orders[i].speed);
	}

	printf("\a\n");
}


