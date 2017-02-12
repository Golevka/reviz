INCLUDE_PATH    := ./src
SOURCE_PATH     := ./src
DEPENDENCY_PATH := ./src/dep
OBJECT_PATH     := ./src/obj

# LDLIBS := -lpthread
# CFLAGS += -Wall -Wextra -pedantic -O3
CFLAGS += -Wall -g3 -Wno-format-security -fno-omit-frame-pointer
# -Werror
# -fsanitize=address 

PROGRAM_NAME := redot

include makefile.mk
