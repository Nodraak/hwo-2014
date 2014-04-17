/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 19:22:10
*/

#include "ft_utils.h"

/*
## notes ##

angle :
	left = neg | right = pos



work time :
	16 avril : 19h-2h -> 7h
	17 avril : 13h-?? -> 4h

*/



/* start (pieceIndex) || speed (between 0 and 10) */
#define MAX		10
int raceData[MAX][2] = {
	{0, 10},
	{2, 6},
	{7, 10},
	{12, 6},
	{17, 10},
	{18, 6},
	{23, 10},
	{25, 6},
	{34, 10},
	{40, 10} /* end - usefull ? -> yes */
};

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
	cJSON *json;

	while ((json = read_msg(sock)) != NULL)
	{
		cJSON *msg, *msg_type;
		char *msg_type_name;
		t_car_basic all = {0, 0, 0, 0, 0};
		t_track_info trackInfo;

		/* handle recieved data and updated info */
		ft_utils_data_parse(json, &all, &trackInfo);

		/* send data */
		msg_type = cJSON_GetObjectItem(json, "msgType");
		if (msg_type == NULL)
			error("missing msgType field");

		msg_type_name = msg_type->valuestring;
		if (!strcmp("carPositions", msg_type_name))
		{
			double speed = ft_utils_get_speed(&all);
			cJSON *switchOrder = ft_utils_get_switch(&all);
			
			if (switchOrder != NULL)
				msg = switchOrder;
			else
				msg = throttle_msg(speed);
		}
		else
		{
			log_message(msg_type_name, json);
			msg = ping_msg();
		}

		write_msg(sock, msg);

		cJSON_Delete(msg);
		cJSON_Delete(json);
	}
}



void ft_utils_data_parse(cJSON *json, t_car_basic *all, t_track_info *trackInfo)
{
	cJSON *data = NULL;
	char *msgType = NULL;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "join") == 0)
	{
		printf("==> JOIN\n");
	}
	else if (strcmp(msgType, "yourCar") == 0)
	{
		printf("==> YOUR CAR (color=%s)\n", cJSON_GetObjectItem(data, "color")->valuestring);
	}
	else if (strcmp(msgType, "gameInit") == 0)
	{
		cJSON *name = NULL;

		printf("==> GAME INIT\n");

		name = ft_utils_field_find("name", json);

		if (name != NULL && strcmp(name->valuestring, "Keimola") != 0)
			error("Unknown race. %d %s\n", __LINE__, __FILE__);

		ft_utils_track_parse(data, trackInfo);
	}
	else if (strcmp(msgType, "gameStart") == 0)
	{
		printf("==> GAME START\n");
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		/*ft_utils_data_raw_print("CAR POS", data);*/
		ft_update_car_data(data, all, trackInfo);
		printf("i=%2d d=%3.1f | s=%2.1f a=%2.1f\t\t",
				all->pieceIndex, all->inPieceDistance, all->speed, all->angle);

		printf("%.2f\n", all->pos);
	}
	else if (strcmp(msgType, "gameEnd") == 0)
	{
		ft_utils_data_raw_print("GAME END", data);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		int i;
		printf("==> New lap\n");

		for (i = 0; i < 5; ++i)
			switchLane[i][2] = 0;
	}
	else
	{
		printf("==> unhandled : %s\n", msgType);
		ft_utils_data_raw_print("\tdata :", data);		
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

double ft_utils_get_speed(t_car_basic *all)
{
	int i = 0;
	int found = 0;
	int newSpeed; /* between 0 and 10 */

	while (!found)
	{
		if (all->pieceIndex >= raceData[i+1][0])
			i++;
		else
			found = 1;
	}

	newSpeed = raceData[i][1];

	if (my_abs(newSpeed - all->speed) > 0.1) /* if big diff between the two speeds */
	{
		/* force quick change */
		if (newSpeed < all->speed)
			newSpeed = 0;
		else if (newSpeed > all->speed)
			newSpeed = 10;
	}

	return (double)newSpeed/10;
}

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
		all->speed = speed;

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
	FILE *f;
	int i;

	current = ft_utils_field_find("pieces", data)->child;

	/* fopen */
	f = fopen("track.txt", "w");
	if (f == NULL)
		error("Error fopen %d %s.\n", __LINE__, __FILE__);

	/* get nbElem */
	ft_utils_piece_parse(current, track, PIECE_PARSE_COUNT);

	/* malloc */
	track->pieces = calloc(track->nbElem, sizeof(t_track_piece));
	if (track->pieces == NULL)
		error("Error malloc %d %s.\n", __LINE__, __FILE__);

	/* build */
	track->nbElem = 0;
	ft_utils_piece_parse(current, track, PIECE_PARSE_BUILD);

	for (i = 0; i < track->nbElem; ++i)
		track->lengthTotal += track->pieces[i].length;

	/* clean */
	fclose(f);
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


