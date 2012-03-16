# By Yann LEGER

NAME	=	sysMeasureTool

CC	=	gcc -m64

SRC	=	sysMeasureTool.c

OBJ     =       $(SRC:.c=.o)

CFLAGS	=	-O0 -Wall

$(NAME)	:	$(OBJ)
		$(CC) $(OBJ) -o $(NAME)

all	:	$(NAME)

asm	:
		$(CC) $(CFLAGS) -S $(SRC)

clean	:
		$(RM) $(OBJ)

fclean	:	clean
		$(RM) $(NAME)

re	:	fclean all

