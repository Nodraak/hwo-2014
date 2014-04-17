/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:43:50
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 02:28:56
*/

#include "ft_graph.h"


char **ft_graph_build(void)
{
/*#ifdef ENABLE_GRAPH*/

	#define SIZE_X		120
	#define SIZE_Y		60
	#define OFFSET_X	600
	#define OFFSET_Y	500




	t_pos pos = {0, 0, 0};
	t_pos min = {0, 0, 0}, max = {0, 0, 0};
	FILE *f;
	char s[1000];
	char **tab;
	int i, j;
	int id = 0;
	tab = malloc(sizeof(char*)*SIZE_Y);
	if (tab == NULL)
		exit(43);
	for (j = 0; j < SIZE_Y; ++j)
	{
		tab[j] = malloc(sizeof(char)*SIZE_X);
		if (tab[j] == NULL)
			exit(44);
		memset(tab[j], ' ', SIZE_X);
	}

	f = fopen("track.txt", "r");
	if (f == NULL)
		exit(42);

	fgets(s, 999, f);

	while (!feof(f))
	{
		int len = 0, radius = 0, angle = 0;
		char *ptr;
		s[0] = '\0';

		while (strncmp(s, "new", 3) != 0 && !feof(f))
		{
			fgets(s, 999, f);
			ptr = s;
			while (*ptr != ' ')
				ptr++;
			ptr++;

			if (strncmp(s, "length", 6) == 0)
				len = atoi(ptr);
			if (strncmp(s, "radius", 6) == 0)
				radius = atoi(ptr);
			if (strncmp(s, "angle", 5) == 0)
				angle = atoi(ptr);
		}

		if (radius == 0 && angle == 0)
		{
			pos.x += len * cos(-pos.angle*M_PI/180);
			pos.y += len * sin(-pos.angle*M_PI/180);
		}
		else
		{
			int mod = (angle < 0 ? -1 : 1);

			double diffy = - mod * radius * (cos((pos.angle)*M_PI/180) - cos((pos.angle+angle)*M_PI/180));
			double diffx = - mod * radius * (sin((pos.angle)*M_PI/180) - sin((pos.angle+angle)*M_PI/180));

			pos.angle += angle;
			pos.x += diffx;
			pos.y += diffy;

			if (pos.angle < 0) pos.angle += 360;
			if (pos.angle >= 360) pos.angle -= 360;

		}

		if (pos.x < min.x) min.x = pos.x;
		if (pos.y < min.y) min.y = pos.y;
		if (pos.x > max.x) max.x = pos.x;
		if (pos.y > max.y) max.y = pos.y;

		printf("data %d %d %d", len, angle, radius);
		printf("\t\tcurrent %.3f %.3f | %.3f\n", pos.x, pos.y, pos.angle);

		int x = (pos.x+OFFSET_X)/10;
		int y = (pos.y+OFFSET_Y)/10;

		if (y >= 0 && y < SIZE_Y && x >= 0 && x < SIZE_X)
			tab[y][x] = '0' + (id%10);
		if (y >= 0 && y < SIZE_Y && x-1 >= 0 && x-1 < SIZE_X)
			tab[y][x-1] = '0' + (id/10);
		id++;
	}

	printf("\nmin : x=%.3f y=%.3f\t", min.x, min.y);
	printf("max : x=%.3f y=%.3f\n", max.x, max.y);

	for (j = SIZE_Y-1; j >= 0; --j)
	{
		for (i = 0; i < SIZE_X; ++i)
			printf("%c", tab[j][i]);
		printf("\n");
	}

	fclose(f);
	return tab;
/*#else
	printf("Info : no graph enabled. Continuing ...\n");
#endif*/
}

/*#ifdef ENABLE_GRAPH

#endif*/
