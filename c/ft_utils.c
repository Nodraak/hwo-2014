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

/*
## notes ##

angle :
	left = neg | right = pos


*/

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


/* find first matching s */
cJSON *ft_utils_field_find(char *s, cJSON* head)
{
	cJSON *current = head, *ret = NULL;

	if (s == NULL)
		return NULL;

	while (current != NULL)
	{
		if (strcmp(current->string, s) == 0)
			return current;

		if (current->child != NULL)
		{
			ret = ft_utils_field_find(s, current->child);
			if (ret != NULL && strcmp(ret->string, s) == 0)
				return ret;
		}

		current = current->next;
	}

	return NULL;
}

void ft_utils_track_parse(cJSON *data)
{
	cJSON *current = data;

	current = ft_utils_field_find("pieces", data)->child;

	ft_utils_piece_parse(current, 0);
}

void ft_utils_piece_parse(cJSON *current, int level)
{
	int i;
	
	while (current != NULL)
	{
		for (i = 0; i < level; ++i)
			printf("\t");

		if (cJSON_False == current->type)
			printf("%s : False\n", current->string);
		else if (cJSON_True == current->type)
			printf("%s : True\n", current->string);
		else if (cJSON_NULL == current->type)
			printf("cJSON_NULL == current->type | %d %s\n", __LINE__, __FILE__);
		else if (cJSON_Number == current->type)
			printf("%s : %.3f\n", current->string, current->valuedouble);
		else if (cJSON_String == current->type)
			printf("%s : %s\n", current->string, current->valuestring);
		else if (cJSON_Array == current->type || cJSON_Object == current->type)
			printf("new stuff :\n"), ft_utils_piece_parse(current->child, level+1);

		current = current->next;
	}
}


