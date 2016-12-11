build:
	gcc -lmraa -lm -w -g -lpthread -std=c99 -o lab4_part1 lab4_part1.c
	gcc -lmraa -lm -w -g -lpthread -std=c99 -o lab4_part2 lab4_part2.c
clean:
	rm lab4_part1
	rm lab4_part2
	rm log*.txt
dist:
	tar -cvzf lab4-604675793.tar.gz README Makefile lab4*.c log*.txt 
