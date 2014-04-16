/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:53:27
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 01:56:34
*/

#ifndef FT_GRAPH_H
#define FT_GRAPH_H

#include <stdio.h>
void ft_graph_build(void);


/*#ifdef ENABLE_GRAPH*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct		s_pos
{
	double			x;
	double			y;
	double			angle; /* like in math : left = pos | right = neg */
}					t_pos;




/*#endif*/

#endif
