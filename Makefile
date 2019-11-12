

CC	=	gcc
RM	=	rm
CFLAGS	=	-Wall -g
OBJS	=	main.o hashmap.o rbtree.o

.SILENT:
.SUFFIXES:	.c .o
.c.o:
	echo compiling $< to $@
	$(CC) $(CFLAGS) -c -o $@ $<


all:	$(OBJS)
	$(CC) $(CFLAGS) -o a $(OBJS)


.PHONY:	clean
clean:
	$(RM) *.o a