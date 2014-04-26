/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-27 00:52:17
*/

#include "ft_utils.h"

/********************
 *  MAIN FUNCTIONS  *
 ********************/

void ft_main_loop(int sock, char *botName)
{
	int tick = -1;
	cJSON *json = NULL;
	cJSON *msg = NULL, *msg_type = NULL;
	cJSON *msgData = NULL;
	t_data data = {NULL, NULL, NULL, GAME_STATUS_WAITING, NULL, NULL};
	data.botName = botName;

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
	data.carInfo->lap = -1;

	/* main loop */
	while ((json = read_msg(sock)) != NULL)
	{
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		/* handle recieved data and updated info */
		msgData = cJSON_GetObjectItem(json, "gameTick");
		if (msgData)
			tick = msgData->valueint;
		else
			tick = -1;
		ft_main_data_parse(json, &data, tick);

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
	{
		printf("clean exit, writing orders to file.\n");
		/* save orders to file */
		ft_orders_file_save_to(data.orders, data.trackDataPath);
	}
	else
		printf("dirty exit, dont write anything.\n");

	/* free every malloc */
	free(data.trackInfo->pieces);
	free(data.carInfo);
	free(data.trackInfo);
	free(data.orders);
}

void ft_main_data_parse(cJSON *json, t_data *data, int tick)
{
	cJSON *msgData = NULL;
	char *msgType = NULL;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	msgData = cJSON_GetObjectItem(json, "data");

	if (msgType == NULL)
	{
		printf("NO msgType field found. %d %s\n", __LINE__, __FILE__);
		return;
	}

	if (strcmp(msgType, "gameInit") == 0)
	{
		/* parse track info and get orders */
		ft_init_track_parse(msgData, data->trackInfo);
		ft_init_get_orders(data, msgData);

		/* printf for debug */
		ft_print_raw_data("GAME INIT", json);
		ft_print_gameInit_data(data);
	}
	else if (strcmp(msgType, "gameStart") == 0)
	{
		int i;

		if (data->carInfo->lap != -1)
		{
			for (i = 0; i < MAX_ORDERS; ++i)
			{
				if (data->orders[i].status == ORDER_STATUS_ENABLED
					&& data->orders[i].pos < data->carInfo->pos)
				{
					data->orders[i].status = ORDER_STATUS_SENDED;
				}
			}
		}
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		cJSON *posData = ft_utils_find_car_pos(data->botName, msgData->child);
		ft_update_car_data(posData, data, tick);

		/* printf for debug */
		if (tick % PRINT_CAR_POS_MODULO == 0 && tick != -1)
			ft_print_raw_data("CAR POS", json);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		cJSON *name = NULL;

		/* osef des autres joueurs */
		name = ft_utils_field_find("name", msgData);

		if (name == NULL)
			printf("WARNING %d %s.\n", __LINE__, __FILE__);
		else
		{
			if (strcmp(name->valuestring, data->botName) == 0)
				ft_main_new_lap(data, json);
			else
				printf("## player %s finished a turn.\n", name->valuestring);
		}
	}
	else if (strcmp(msgType, "spawn") == 0)
	{
		/* TODO */
	}
	else if (strcmp(msgType, "tournamentEnd") == 0)
	{
		data->gameStatus = GAME_STATUS_TOURNAMENT_END;
	}
	else if (strcmp(msgType, "join") == 0 || strcmp(msgType, "yourCar") == 0)
	{
		printf("JOIN / YOUR CAR\n");
	}
	else
	{
		ft_print_raw_data(msgType, json);
	}

	/* new lap - on ci qualif */
	if (data->carInfo->pos < data->carInfo->posOld)
		ft_main_new_lap(data, json);
	/* respawn - ci hack */
	if (data->carInfo->pos == data->carInfo->posOld)
		data->carInfo->speedWanted = 10;

	/* printf for debug */
	if (tick % 20 == 0)
		printf("gameTicks\tpos\tspeed\twanted\tangle\t\torder\n");
	if (tick != -1)
		printf("%d\t\t%.2f\t%.1f\t%.1f\t%.1f", tick, data->carInfo->pos,
			data->carInfo->speedActual, data->carInfo->speedWanted, data->carInfo->angle);
}


void ft_main_new_lap(t_data *data, cJSON *json)
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

cJSON *ft_utils_find_car_pos(char *botName, cJSON *msgData)
{
	int found = 0;

	while (!found && msgData != NULL)
	{
		cJSON *currentName = NULL;

		currentName = ft_utils_field_find("name", msgData);

		if (currentName != NULL)
		{
			if (strcmp(botName, currentName->valuestring) == 0)
				found = 1;
			else
				msgData = msgData->next;
		}
		else
		{
			printf("WARNING %d %s.\n", __LINE__, __FILE__);
			msgData = msgData->next;
		}
	}

	if (found)
		return msgData;
	else
		return NULL;
}


void ft_orders_update(t_order *orders, t_track_info *trackInfo)
{
	int i;
	double speed;

	for (i = 0; i < trackInfo->nbElem; ++i)
	{
		if (trackInfo->pieces[i].maxAngle < UPDATE_SPEED_MAX_ANGLE_UPDATE_BIG)
		{
			speed = (UPDATE_SPEED_MAX_ANGLE_UPDATE_BIG - trackInfo->pieces[i].maxAngle) / 100.0;
			if (trackInfo->pieces[i].maxAngle > UPDATE_SPEED_MAX_ANGLE_UPDATE_SLOW1)
				speed /= 5;
			if (trackInfo->pieces[i].maxAngle > UPDATE_SPEED_MAX_ANGLE_UPDATE_SLOW2)
				speed /= 10;

			/* actual */
			orders[i*2].speed += speed;
			if (orders[i*2].speed > 10)
				orders[i*2].speed = 10;

			/* next */
			orders[i*2+1].speed += speed;
			if (orders[i*2+1].speed > 10)
				orders[i*2+1].speed = 10;

			printf("%d %.2f\n", i, speed);
		}
		else if (trackInfo->pieces[i].maxAngle > UPDATE_SPEED_ANGLE_SLOW_DOWN)
		{
			/* current */
			orders[i*2].speed -= 0.1;
			orders[i*2+1].speed -= 0.1;
			/* previous */
			if (i != 0)
			{
				orders[i*2-1].speed -= 0.1;
				orders[i*2-2].speed -= 0.1;
			}
			
		}
	}
}


cJSON *ft_main_msg_make(cJSON *msgType, t_data *data)
{
	cJSON *msg = NULL;
	char *msgTypeName = NULL;

	msgTypeName = msgType->valuestring;
	if (msgTypeName != NULL)
	{
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
	}

	/* nothing to say -> ping (or speed if beginning of game) */
	if (msg == NULL)
	{
		if (data->carInfo->speedActual < 2)
		{
			data->carInfo->speedWanted = 5;
			msg = throttle_msg(0.5);
		}
		else
			msg = ping_msg();	
	}

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

void ft_update_car_data(cJSON *msgData, t_data *data, int tick)
{
	static double oldInPieceDistance = 0;
	/*static double oldAngle = 0;*/
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

	if (speed >= 0 && tick > 0)
	{
		data->carInfo->speedOld = data->carInfo->speedActual;
		data->carInfo->speedActual = speed;
	}

	/* pos */
	data->carInfo->posOld = data->carInfo->pos;
	data->carInfo->pos = data->carInfo->pieceIndex +
							data->carInfo->inPieceDistance / data->trackInfo->pieces[data->carInfo->pieceIndex].length;

	/* prepare for next loop */
	oldInPieceDistance = data->carInfo->inPieceDistance;
	/*oldAngle = data->carInfo->angle;*/
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
	char *s = NULL;

	printf("\n==> %s\n", type);

	s = cJSON_Print(data);
	if (s != NULL)
	{
		printf("%s\n", s);
		free(s);
	}
}


void ft_print_gameInit_data(t_data *data)
{
	int i;

	/* track data */
	printf("Track : (right=%d curve=%d)\n", PIECE_TYPE_RIGHT, PIECE_TYPE_CURVE);
	printf("i (%d)\ttype\tlen\tangle\n", data->trackInfo->nbElem);
	for (i = 0; i < data->trackInfo->nbElem; ++i)
	{
		printf("%d\t%d\t%.1f\t%d\n", i, data->trackInfo->pieces[i].type,
			data->trackInfo->pieces[i].length , data->trackInfo->pieces[i].angle);
	}
	printf("\n");

	/* orders data */
	printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
		ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
	printf("status\tpos\ttype\tspeed\tswitch\n");
	for (i = 0; i < MAX_ORDERS; ++i)
	{
		if (data->orders[i].status != ORDER_STATUS_DISABLED)
		{
			printf("%d\t%.1f\t%d\t%.1f\t%d\n", data->orders[i].status, data->orders[i].pos,
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
