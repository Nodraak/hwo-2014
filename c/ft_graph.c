/* 
* @Author: Adrien Chardon
* @Date:   2014-04-16 23:43:50
* @Last Modified by:   Adrien Chardon
* @Last Modified time: 2014-04-17 01:56:34
*/

#include "ft_graph.h"


void ft_graph_build(void)
{
/*#ifdef ENABLE_GRAPH*/
	t_pos pos = {0, 0, 0};
	t_pos min = {0, 0, 0}, max = {0, 0, 0};
	FILE *f;
	char s[1000];
	char tab[50][120];
	int i, j;
	int id = 0;

	memset(tab, ' ', 50*120*1);

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

			if (pos.angle < 0)
				pos.angle += 360;
			if (pos.angle >= 360)
				pos.angle -= 360;

		}

		if (pos.x < min.x)
			min.x = pos.x;
		if (pos.y < min.y)
			min.y = pos.y;
		if (pos.x > max.x)
			max.x = pos.x;
		if (pos.y > max.y)
			max.y = pos.y;

		printf("data %d %d %d\n", len, angle, radius);
		printf("\t\t\t\tcurrent %.3f %.3f | %.3f\n", pos.x, pos.y, pos.angle);

		if (50-(int)(pos.y+500)/10 >= 0 && 50-(int)(pos.y+500)/10 < 50
			&& (int)(pos.x+600)/10 >= 0 && (int)(pos.x+600)/10 < 120)
			tab[50-(int)(pos.y+500)/10][(int)(pos.x+600)/10] = '0' + (id%10);
		if (50-(int)(pos.y+500)/10 >= 0 && 50-(int)(pos.y+500)/10 < 50
			&& (int)(pos.x+600)/10-1 >= 0 && (int)(pos.x+600)/10-1 < 120)
			tab[50-(int)(pos.y+500)/10][(int)(pos.x+600)/10-1] = '0' + (id/10);
		id++;
	}

	printf("min : x=%.3f y=%.3f\n", min.x, min.y);
	printf("max : x=%.3f y=%.3f\n", max.x, max.y);

	for (j = 0; j < 50; ++j)
	{
		for (i = 0; i < 120; ++i)
			printf("%c", tab[j][i]);
		printf("\n");
	}

	fclose(f);
/*#else
	printf("Info : no graph enabled. Continuing ...\n");
#endif*/
}

/*#ifdef ENABLE_GRAPH

#endif*/
