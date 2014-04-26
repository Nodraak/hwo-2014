#!/bin/bash

rm -f *.orders out*

i=1

while [ $i -le 4 ];
do
	echo $i >> out.main
	echo "-----------------------" >> out.main
	echo "==> build" >> out.main
	(time gcc *.c -Wall -Wextra -ansi -pedantic -D NOT_AUTO_BUILD -D ID_TRACK=$i -lm -o cbot) 2>> out.main

	FILE1="out$i.1"
	FILE2="out$i.2"
	FILE3="out$i.3"
	echo "==> run 1" >> out.main
	(time ./my_run) 1> $FILE1 2>> out.main
	echo "==> run 2" >> out.main
	(time ./my_run) 1> $FILE2 2>> out.main
	echo "==> run 3" >> out.main
	(time ./my_run) 1> $FILE3 2>> out.main

	i=$(($i+1))
done

echo "done" >> out.main
