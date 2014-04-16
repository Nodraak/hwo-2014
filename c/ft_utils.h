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

void ft_utils_track_parse(cJSON *data);

#endif /* FT_UTILS_H */
