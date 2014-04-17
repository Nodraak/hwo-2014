/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 16:50:57
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
int switchLane[7][3] = {
	{3,1,0},
	{8,-1,0},
	{13,0,0},
	{18,1,0},
	{25,0,0},
	{29,-1,0},
	{35,1,0}
};

double my_abs(double n)
{
	return (n < 0 ? -n : n);
}

double ft_utils_get_speed(t_all *all)
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

cJSON *ft_utils_get_switch(t_all *all)
{
	cJSON *msg = NULL;
	int i;

	for (i = 0; i < 7; ++i)
	{
		if (switchLane[i][0] == all->pieceIndex && switchLane[i][2] == 0)
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


cJSON *ft_main_loop(cJSON *json)
{
	cJSON *msg, *msg_type;
	char *msg_type_name;
	static t_all all = {0, 0, 0, 0};

	/* handle recieved data and updated info */
	ft_utils_data_parse(json, &all);

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

	return msg;
}



void ft_utils_data_parse(cJSON *json, t_all *all)
{
	cJSON *data = NULL;
	char *msgType = NULL;
	char **ptr;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "join") == 0)
	{
		ft_utils_info_join_print(data);
	}
	else if (strcmp(msgType, "yourCar") == 0)
	{
		ft_utils_info_yourCar_print(data);
	}
	else if (strcmp(msgType, "gameInit") == 0)
	{
		cJSON *tmp = ft_utils_field_find("name", json);

		if (tmp != NULL && strcmp(tmp->valuestring, "Keimola") != 0)
			error("Unknown race. %d %s\n", __LINE__, __FILE__);

		ptr = ft_utils_track_parse(data);
		/*ft_utils_data_raw_print("GAME INIT", data);*/
	}
	else if (strcmp(msgType, "gameStart") == 0)
	{
		printf("==> GAME START\n");
	}
	else if (strcmp(msgType, "carPositions") == 0)
	{
		/*ft_utils_data_raw_print("CAR POS", data);*/
		ft_update_car_data(data, all);
		printf("i=%2d d=%3.1f | s=%2.1f a=%2.1f\n",
				all->pieceIndex, all->inPieceDistance, all->speed, all->angle);

	}
	else if (strcmp(msgType, "gameEnd") == 0)
	{
		ft_utils_data_raw_print("GAME END", data);
	}
	else if (strcmp(msgType, "lapFinished") == 0)
	{
		printf("==> New lap\n");
		int i;

		for (i = 0; i < 7; ++i)
			switchLane[i][2] = 0;
	}
	else
	{
		printf("==> unhandled : %s\n", msgType);
	}

	/*data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		printf("gameTick=%d\n", data->valueint);*/
}


void ft_update_car_data(cJSON *data, t_all *all)
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
}

void ft_utils_info_join_print(cJSON *data)
{
	char *name = NULL, *key = NULL;

	name = cJSON_GetObjectItem(data, "name")->valuestring;
	key = cJSON_GetObjectItem(data, "key")->valuestring;

	printf("==> JOIN\n"
			"\tname : %s\n"
			"\tkey : %s\n",
			name, key);
}


void ft_utils_info_yourCar_print(cJSON *data)
{
	char *name = NULL, *color = NULL;

	name = cJSON_GetObjectItem(data, "name")->valuestring;
	color = cJSON_GetObjectItem(data, "color")->valuestring;

	printf("==> YOUR CAR\n"
			"\tname : %s\n"
			"\tcolor : %s\n",
			name, color);
}

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

char **ft_utils_track_parse(cJSON *data)
{
	cJSON *current = data;
	t_track_info track = {0, 0};
	FILE *f;

	current = ft_utils_field_find("pieces", data)->child;

	f = fopen("track.txt", "w");
	ft_utils_piece_parse(current, &track, f);
	fclose(f);
	printf("Total length : %.3f\nNb elem : %d\n", track.length, track.nb_elem);

	return ft_graph_build();
}

void ft_utils_piece_parse(cJSON *current, t_track_info *data, FILE *f)
{
	while (current != NULL)
	{
		if (current->string != NULL && strcmp(current->string, "length") == 0)
			data->length += current->valuedouble;

		if (current->string != NULL && strcmp(current->string, "radius") == 0
			&& current->next->string != NULL && strcmp(current->next->string, "angle") == 0)
			data->length += current->valuedouble * abs(current->next->valuedouble) * 2 * M_PI / 360;

		if (cJSON_False == current->type)
			fprintf(f, "%s False\n", current->string);
		else if (cJSON_True == current->type)
			fprintf(f, "%s True\n", current->string);
		else if (cJSON_NULL == current->type)
			fprintf(f, "cJSON_NULL == current->type | %d %s\n", __LINE__, __FILE__);
		else if (cJSON_Number == current->type)
			fprintf(f, "%s %.3f\n", current->string, current->valuedouble);
		else if (cJSON_String == current->type)
			fprintf(f, "%s %s\n", current->string, current->valuestring);
		else if (cJSON_Array == current->type || cJSON_Object == current->type)
		{
			fprintf(f, "new\n");
			data->nb_elem ++;
			ft_utils_piece_parse(current->child, data, f);
		}

		current = current->next;
	}
}


