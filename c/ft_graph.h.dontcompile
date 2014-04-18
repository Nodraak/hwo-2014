/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 14:44:00
*/

#ifndef FT_GRAPH_H
#define FT_GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cJSON.h"
#include "cbot.h"

#define SIZE_X		120
#define SIZE_Y		60
#define OFFSET_X	600
#define OFFSET_Y	500

typedef struct		s_pos
{
	double			x;
	double			y;
	double			angle; /* like in math : left = pos | right = neg */
}					t_pos;

void ft_graph_init(FILE **f, char ***tab);
void ft_graph_data_parse(FILE *f, int *len, int *radius, int *angle);
void ft_graph_pos_calc(int len, int radius, int angle, t_pos *pos);
void ft_graph_tab_draw(t_pos pos, char **tab);
char **ft_graph_build(void);

#endif
