NAME = server
CLANG = clang++
FLAG = -Wall -Wextra -Werror
SANITIZE = -g3 -fsanitize=address
INCLUDE = -I ./include
SRCS = Frame.cpp	\
	Server.cpp	\
	Socket.cpp	\
	Config.cpp	\
	Location.cpp	\
	Request.cpp		\
	Utils.cpp		\


OBJS = $(addprefix ./srcs/, $(SRCS:.cpp=.o))

all : $(NAME)
	
%.o : %.cpp
	$(CLANG) $(FLAG) $(SANITIZE) $(INCLUDE) -c $^ -o $@

$(NAME) : $(OBJS)
	$(CLANG) $(FLAG) $(SANITIZE) $(INCLUDE) $^ -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

run : $(NAME)
	./$(NAME)

re : fclean all
