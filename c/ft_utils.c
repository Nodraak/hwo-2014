/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 14:46:02
*/

#include "ft_utils.h"

/*
## notes ##

angle :
	left = neg | right = pos


*/


cJSON *ft_main_loop(cJSON *json)
{
	cJSON *msg, *msg_type;
	char *msg_type_name;
	t_all all = {0, 0, 0, 0};

	/* handle recieved data and updated info */
	ft_utils_data_parse(json, &all);

	/* send data */
	msg_type = cJSON_GetObjectItem(json, "msgType");
	if (msg_type == NULL)
		error("missing msgType field");

	msg_type_name = msg_type->valuestring;
	if (!strcmp("carPositions", msg_type_name))
	{
		static int ok = 0;
		double speed;
		if (!ok)
		{
			speed = 1;
			if (all.speed >= 0.5)
				ok = 1;
		}
		else
		{
			speed = 0;
		}
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
		ft_utils_info_join_print(data);
	else if (strcmp(msgType, "yourCar") == 0)
		ft_utils_info_yourCar_print(data);
	else if (strcmp(msgType, "gameInit") == 0)
		ptr = ft_utils_track_parse(data);
		/*ft_utils_data_raw_print("GAME INIT", data);*/
	else if (strcmp(msgType, "gameStart") == 0)
		printf("\t==> GAME START\n");
	else if (strcmp(msgType, "carPositions") == 0)
	{
		/*ft_utils_data_raw_print("CAR POS", data);*/
		ft_update_car_data(data, all);
		printf("%d\t%.3f\t%.3f\t%.3f\n", all->pieceIndex, all->inPieceDistance, all->speed, all->angle);

	}
	else if (strcmp(msgType, "gameEnd") == 0)
		ft_utils_data_raw_print("GAME END", data);
	else
		printf("==> unhandled : %s\n", msgType);

	/*data = cJSON_GetObjectItem(json, "gameTick");
	if (data)
		printf("gameTick=%d\n", data->valueint);*/
}


void ft_update_car_data(cJSON *data, t_all *all)
{
	cJSON *ret = NULL;

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
	ret = ft_utils_field_find("inPieceDistance", data);
	if (ret != NULL)
	{
		static double old = 0;
		double speed = ret->valuedouble - old;
		old = ret->valuedouble;

		if (speed >= 0)
			all->speed = speed;
	}
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


