# By Yann LEGER

NAME	=	sysMeasureTool

CC	=	gcc -m64

SRC	=	sysMeasureTool.c	\
		memory.c		\
		network.c		\
		filesystem.c

OBJ     =       $(SRC:.c=.o)

CFLAGS	=	-O -Wall -Winline -fno-omit-frame-pointer -funroll-loops

LDFLAGS =	-lsocket -lnsl

$(NAME)	:	$(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) -o $(NAME)

all	:	$(NAME)

asm	:
		$(CC) $(CFLAGS) -S $(SRC)

clean	:
		$(RM) $(OBJ)

fclean	:	clean
		$(RM) $(NAME)

re	:	fclean all

