#ifndef FT_UTILS_H
#define FT_UTILS_H

#include <stdlib.h>		/* NULL */
#include <stdio.h>		/* printf */
#include <sys/types.h>	/* size_t */
#include <string.h>		/* strcmp */

#include "cJSON.h"

void ft_utils_data_parse(cJSON *json);

void ft_utils_info_join_print(cJSON *data);
void ft_utils_info_yourCar_print(cJSON *data);
void ft_utils_data_raw_print(char *type, cJSON *data);


cJSON *ft_utils_field_find(char *s, cJSON* head);
void ft_utils_track_parse(cJSON *data);
void ft_utils_piece_parse(cJSON *current, int level);

#endif /* FT_UTILS_H */
