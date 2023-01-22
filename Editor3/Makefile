EXEC_NAME=ed3

CC=clang++
LD=clang++

BIN_DIR=bin
INC_DIR=include
SRC_DIR=src
OBJ_DIR=obj
SUBDIRS=.

INCLUDES_DIRS=
LIBS_DIRS=/usr/local/lib

LIBS=
DEFINES= EDITOR_COMPILE_UNIX EDITOR_COMPILE_LITTLE_ENDIAN

CFLAGS=-g -Wall
LDFLAGS=
ARFLAGS=rcs

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d

FULL_EXEC=$(BIN_DIR)/$(EXEC_NAME)
FULL_CFLAGS=$(CFLAGS) -I$(INC_DIR) $(addprefix -I, $(INCLUDES_DIRS)) $(addprefix -D, $(DEFINES)) $(DEPFLAGS)
FULL_LDFLAGS=$(LDFLAGS) $(addprefix -L, $(LIBS_DIRS)) $(addprefix -l, $(LIBS))

SRC_SUBDIRS=$(addprefix $(SRC_DIR)/, $(SUBDIRS))
INC_SUBDIRS=$(addprefix $(INC_DIR)/, $(SUBDIRS))

INCS=$(wildcard *.hpp $(foreach fd, $(INC_SUBDIRS), $(fd)/*.hpp))
SRCS=$(wildcard *.cpp $(foreach fd, $(SRC_SUBDIRS), $(fd)/*.cpp))

OBJS=$(subst $(SRC_DIR), $(OBJ_DIR), $(SRCS:.cpp=.o))
DEPFILES := $(OBJS:.o=.d)

all: $(FULL_EXEC)

$(FULL_EXEC): $(OBJS)
	$(CC) $(OBJS) $(FULL_LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(FULL_CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)/*.exe $(BIN_DIR)/*.out $(BIN_DIR)/*.bin $(OBJ_DIR)/* $(FULL_EXEC)

run:
	$(FULL_EXEC)

$(DEPFILES):

include $(wildcard $(DEPFILES))
