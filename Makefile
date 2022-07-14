NAME =	webserv

CC = c++

OBJ_PATH = ./obj
SRC_PATH = ./src

SRC_FILES =	main.cpp			NetSocket.cpp	Event.cpp				\
			Request.cpp 		ServerConfiguration.cpp Client.cpp		\
			CgiCore.cpp			Response.cpp			fileUtils.cpp	\
			Session.cpp			processRequest.cpp		RequestCheck.cpp\
			RequestParse.cpp

OBJS = ${addprefix ${OBJ_PATH}/,${SRC_FILES:.cpp=.obj}}

CXXFLAGS = -Wall -Werror -Wextra -MMD -I. -std=c++98 -pedantic 

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

${OBJ_PATH}/%.obj : ${SRC_PATH}/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@ 

-include $(addprefix ${OBJ_PATH}/,${SRC_FILES:.cpp=.d})

clean:
	@rm -rf ${OBJ_PATH}

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
