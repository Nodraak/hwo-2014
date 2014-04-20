/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-20 18:28:23
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
	t_car_basic *all = NULL;
	t_track_info *trackInfo = NULL;
	t_order *orders = NULL;
	int isRaceStarted = 0;

	/* init */
	all = calloc(1, sizeof(t_car_basic));
	if (all == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);
	trackInfo = calloc(1, sizeof(t_track_info));
	if (trackInfo == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);
	orders = calloc(MAX_ORDERS, sizeof(t_order));
	if (orders == NULL)
		error("Error malloc %d %s\n", __LINE__, __FILE__);

	/* main loop */
	while ((json = read_msg(sock)) != NULL)
	{
		/* handle recieved data and updated info */
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		ft_utils_data_parse(json, all, trackInfo, orders);

		/* prepare response */
		msg_type_name = msg_type->valuestring;
		if (strcmp("gameInit", msg_type_name) == 0)
		{
			msg = throttle_msg(1);
			printf("--> order initial speed : 1 (gameInit)\n");
		}
		else if (strcmp("gameStart", msg_type_name) == 0)
		{
			isRaceStarted = 1;
			msg = throttle_msg(1);
			printf("--> order initial speed : 1 (gameStart)\n");
		}
		else if (strcmp("carPositions", msg_type_name) == 0)
		{
			if (isRaceStarted && msg == NULL)
				msg = ft_orders_next_get(all, orders);
		}

		/* send data */
		if (msg == NULL) /* nothing to say -> ping */
				msg = ping_msg();
		write_msg(sock, msg);

		cJSON_Delete(msg), msg = NULL;
		cJSON_Delete(json), json = NULL;

		printf("\n"); /* flush all output for this loop / game tick */
	}

	printf("Read null msg, cleaning and quitting ...\n");

	free(trackInfo->pieces);

	free(all);
	free(trackInfo);
	free(orders);
}



void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo, t_order *orders)
{
	cJSON *data = NULL;
	char *msgType = NULL;
	int tick = -1, i;

	data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		tick = data->valueint;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "gameInit") == 0)
	{
		/* analyse track */
		ft_orders_track_parse(data, trackInfo);
		ft_orders_compute(trackInfo, orders);
#if 0
		/* load from file */
+		if (name != NULL && strcmp(name->valuestring, "Keimola") == 0)
+		{
+			FILE *f = NULL;
+			int valueInt, idOrder = 0, orderType;
+			double pos, valueDouble;
+
+			printf("Info : knowned race, loading file ...\n");
+			f = fopen("raceData/Keimola.txt", "r");
+			if (f == NULL)
+				error("Error fopen %d %s\n", __LINE__, __FILE__);
+			while (!feof(f))
+			{
+				int ret = fscanf(f, "%lf\t%d\t%lf\t%d", &pos, &orderType, &valueDouble, &valueInt);
+				if (ret == 4)
+				{
+					ft_order_add(&orders[idOrder], pos, orderType, valueInt, valueDouble, ORDER_STATUS_ENABLED);
+					idOrder++;
+				}
+				else
+				{
+					while (idOrder < MAX_ORDERS)
+					{
+						ft_order_add(&orders[idOrder], 0, 0, 0, 0, ORDER_STATUS_DISABLED);
+						idOrder++;
+					}
+					break;
+				}
+			}
+		}
+		else /* guess */
#endif

		/* printf for debug */
		ft_utils_data_raw_print("GAME INIT", json);

		printf("Track : (right=%d curve=%d)\n",
			PIECE_TYPE_RIGHT, PIECE_TYPE_CURVE);
		printf("i\ttype\tlen\tangle\n");
		for (i = 0; i < trackInfo->nbElem; ++i)
			printf("%d\t%d\t%.1f\t%d\n",
				i, trackInfo->pieces[i].type , trackInfo->pieces[i].length , trackInfo->pieces[i].angle);
		printf("\n");

		printf("Orders : (type : speed=%d switch=%d) (switch : left=%d right=%d)\n",
			ORDER_TYPE_SPEED, ORDER_TYPE_SWITCH, ORDER_SWITCH_LEFT, ORDER_SWITCH_RIGHT);
		printf("pos\ttype\tspeed\tswitch\n");
		for (i = 0; i < MAX_ORDERS; ++i)
		{
			if (orders[i].status != ORDER_STATUS_DISABLED)
				printf("%.1f\t%d\t%.1f\t%d\n",
					orders[i].pos, orders[i].type, orders[i].valueDouble, orders[i].valueInt);
		}
		printf("\n");
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		ft_update_car_data(data, all, trackInfo, orders);

		/* printf for debug */
		if (tick % PRINT_CAR_POS_MODULO == 0 && tick != -1)
			ft_utils_data_raw_print("CAR POS", json);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		ft_utils_data_raw_print("NEW LAP", json);
		ft_orders_reenable(orders);
		all->lap++;

		printf("maxAngle data :\n");
		for (i = 0; i < trackInfo->nbElem; ++i)
		{
			if (trackInfo->pieces[i].type == PIECE_TYPE_RIGHT)
				printf("   %d\n", trackInfo->pieces[i].maxAngle);
			else
				printf("%d %d\n", i, trackInfo->pieces[i].maxAngle);
		}
		printf("speed orders :\npos\tspeed\n");
		for (i = 0; orders[i].type == ORDER_TYPE_SPEED; ++i)
			printf("%.3f\t%.1f\n",
				orders[i].pos, orders[i].valueDouble);

		printf("\a\a\a\a\a\n");
	}
	else if (strcmp(msgType, "spawn") == 0)
	{
		all->speedWanted = 5;
		/* TODO */
	}
	else
	{
		ft_utils_data_raw_print(msgType, json);
	}

	/* printf for debug */
	if (tick % 20 == 0)
		printf("gameTicks\tpos\tspeed\twanted\tangle\t\torder\n");
	if (tick != -1)
		printf("%d\t\t%.2f\t%.1f\t%.1f\t%.1f", tick, all->pos, all->speedActual, all->speedWanted, all->angle);
}


/*****************
 *  UPDATE DATA  *
 *****************/

void ft_update_car_data(cJSON *data, t_car_basic *all, t_track_info *trackInfo, t_order *orders)
{
	cJSON *ret = NULL;
	double speed, AngleDiff;
	static double oldInPieceDistance = 0, oldAngle = 0;
	int haveOrderInNextTwoPieces = 0, i;
	int action = 0;

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

	/* update max angle if not first lap */
	if (all->lap >= 1 && ft_abs(all->angle) > trackInfo->pieces[(int)all->pos].maxAngle)
		trackInfo->pieces[(int)all->pos].maxAngle = ft_abs(all->angle);

	/* speed */
	speed = all->inPieceDistance - oldInPieceDistance;
	
	if (speed > 0)
		all->speedActual = speed;

	all->pos = all->pieceIndex + all->inPieceDistance / trackInfo->pieces[all->pieceIndex].length;



	/*=== check for speed - if on a bend ===*/
	/* TODO add order insted of hard coded speed order */

	(void)action, (void)AngleDiff, (void)i, (void)haveOrderInNextTwoPieces, (void)orders;
#if 0
	speed = all->speedActual;
	AngleDiff = ft_abs(all->angle) - ft_abs(oldAngle);

	/* slow down */
	if (trackInfo->pieces[(int)all->pos].type != PIECE_TYPE_RIGHT)
	{
		if (ft_abs(AngleDiff) > 5)
			all->speedWanted = 0, action = 1;

		if (ft_abs(all->angle) > 50)
			all->speedWanted = all->speedActual - 1, action = 1;
	}
	/* accelerate */
	if (trackInfo->pieces[(int)all->pos+1].type == PIECE_TYPE_RIGHT)
	{
		if (AngleDiff < 0)
			all->speedWanted = all->speedActual + 1, action = 1;
	}

	if (action)
	{
		/* set back */
		haveOrderInNextTwoPieces = 0;
		for (i = 0; i < MAX_ORDERS; ++i)
		{
			if (orders[i].status == ORDER_STATUS_ENABLED)
			{
				double posDiff = orders[i].pos - all->pos;
				if (posDiff > 0 && posDiff < 1)
					haveOrderInNextTwoPieces = 1;
			}
		}
		if (haveOrderInNextTwoPieces == 0)
		{
			int idOrderFree = 0;
			while (orders[idOrderFree].status != ORDER_STATUS_DISABLED)
				idOrderFree ++;
			ft_orders_add(&orders[idOrderFree], all->pos + 1, ORDER_TYPE_SPEED, 0, speed, ORDER_STATUS_ENABLED);
		}
	}

#endif

	oldInPieceDistance = all->inPieceDistance;
	oldAngle = all->angle;
}

/***********
 *  UTILS  *
 ***********/

void ft_utils_data_raw_print(char *type, cJSON *data)
{
	char *s = cJSON_Print(data);
	printf("\n==> %s\n%s\n", type, s);
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

