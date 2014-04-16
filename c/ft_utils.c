/*******************************************************************************
*
*  File Name : utils.c
*
*  Purpose :
*
*  Creation Date : 16-04-14 21:40:19
*
*  Last Modified : 16-04-14 22:03:41
*
*  Created By : Nodraak
*
*******************************************************************************/

#include "ft_utils.h"


void ft_utils_data_parse(cJSON *json)
{
	char *msgType = NULL;
	cJSON *data = NULL;

	msgType = cJSON_GetObjectItem(json, "msgType")->valuestring;
	data = cJSON_GetObjectItem(json, "data");

	if (strcmp(msgType, "join") == 0)
		ft_utils_info_join_print(data);
	else if (strcmp(msgType, "yourCar") == 0)
		ft_utils_info_yourCar_print(data);
	else if (strcmp(msgType, "gameInit") == 0)
		ft_utils_track_parse(data);
		/*ft_utils_data_raw_print("GAME INIT", data);*/
	else if (strcmp(msgType, "gameStart") == 0)
		printf("\t==> GAME START\n");
	else if (strcmp(msgType, "carPositions") == 0)
		ft_utils_data_raw_print("CAR POS", data);
	else if (strcmp(msgType, "gameEnd") == 0)
		ft_utils_data_raw_print("GAME END", data);
	else
		printf("==> unhandled : %s\n", msgType);
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

void ft_utils_track_parse(cJSON *data)
{




}


