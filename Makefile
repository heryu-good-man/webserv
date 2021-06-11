CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes

NAME = webserv

SRCS =	srcs/main.cpp \


OBJS = $(SRCS:.cpp=.o)



$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@



all : $(NAME)

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all

