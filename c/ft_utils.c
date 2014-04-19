/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-19 12:49:29
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
		/* handle recieved data and updated info */
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		ft_utils_data_parse(json, &all, trackInfo, orders);

		/* prepare response */
		msg_type_name = msg_type->valuestring;
		if (strcmp("gameStart", msg_type_name) == 0)
		{
			isRaceStarted = 1;
		}
		else if (strcmp("carPositions", msg_type_name) == 0)
		{
			if (isRaceStarted)
				msg = ft_orders_next_get(&all, orders);
		}

		/* send data */
		if (msg == NULL) /* nothing to say -> ping */
				msg = ping_msg();
		write_msg(sock, msg);

		cJSON_Delete(msg), msg = NULL;
		cJSON_Delete(json), json = NULL;

		printf("\n"); /* flush all output for this loop / game tick */
	}

	printf("Read null msg, quitting ...\n");
}



void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders)
{
	cJSON *data = NULL;
	char *msgType = NULL;
	int tick = -1;

	data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		tick = data->valueint;
	printf("tick=%d", tick);

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "gameInit") == 0)
	{
		int i;

		/* analyse track */
		ft_orders_track_parse(data, trackInfo);
		ft_orders_compute(trackInfo, orders);

		/* printf for debug */
		ft_utils_data_raw_print("GAME INIT", json);

		printf("Track : (right=%d curveLight=%d curveHard=%d)\n",
			PIECE_TYPE_RIGHT, PIECE_TYPE_CURVE_LIGHT, PIECE_TYPE_CURVE_HARD);
		for (i = 0; i < trackInfo->nbElem; ++i)
			printf("%d\ttype=%d\tlength%.3f\tradius=%d\n",
				i, trackInfo->pieces[i].type , trackInfo->pieces[i].length , trackInfo->pieces[i].angle);
		printf("\n");

		printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
			ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
		for (i = 0; orders[i].status != ORDER_STATUS_DISABLED; ++i)
			printf("pos=%.3f\ttype=%d\tvalueD=%.1f\tvalueI=%d\n",
				orders[i].pos, orders[i].type, orders[i].valueDouble, orders[i].valueInt);
		printf("\n");
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		ft_update_car_data(data, all, trackInfo);

		/* printf for debug */
		if (tick % PRINT_CAR_POS_MODULO == 0 && tick != -1)
			ft_utils_data_raw_print("CAR POS", json);

		printf("\t\tpos=%3.2f spe=%2.2f ang=%2.1f",
			all->pos, all->speedActual, all->angle);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		ft_utils_data_raw_print("NEW LAP", json);
		ft_orders_reenable(orders);
	}
	else
	{
		ft_utils_data_raw_print(msgType, json);
	}
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

