NAME = webserv
CLANG = clang++
FLAG = -Wall -Wextra -Werror
# SANITIZE = -g3 -fsanitize=address
INCLUDE = -I ./include
SRCS = Frame.cpp	\
	Server.cpp	\
	Socket.cpp	\
	Config.cpp	\
	Location.cpp	\
	Request.cpp		\
	Utils.cpp		\
	CGI.cpp 		\
	Response.cpp	\
	FDManager.cpp \


OBJS = $(addprefix ./srcs/, $(SRCS:.cpp=.o))

all : $(NAME)
	sed "s~PWD~${PWD}~g" ./config/template.conf > ./config/tester2.conf
	sed "s~PWD~${PWD}~g" ./config/template2.conf > ./config/my.conf

%.o : %.cpp
	$(CLANG) $(FLAG) $(SANITIZE) $(INCLUDE) -c $^ -o $@

$(NAME) : $(OBJS)
	$(CLANG) $(FLAG) $(SANITIZE) $(INCLUDE) $^ -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)
	rm -rf ./tmp/[0-9]*

run : $(NAME)
	./$(NAME)

re : fclean all
