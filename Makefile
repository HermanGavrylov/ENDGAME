NAME = endgame

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC_FILES:.c=.o)))

CC = clang
CFLAGS = -std=c11 -fsanitize=address -g -Wall -Wextra -Werror -Wpedantic -I$(INC_DIR)
LDFLAGS += -fsanitize=address
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)

    RAYLIB_PREFIX := $(shell brew --prefix raylib 2>/dev/null)

    ifeq ($(RAYLIB_PREFIX),)
        $(error Raylib not found. Install it with: brew install raylib)
    endif

    CFLAGS += -I$(RAYLIB_PREFIX)/include
    LDFLAGS = -L$(RAYLIB_PREFIX)/lib -lraylib \
              -framework OpenGL -framework Cocoa -framework IOKit \
              -framework CoreAudio -framework CoreVideo

else ifeq ($(UNAME_S),Linux)

    CFLAGS += -I/usr/include
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

else
    $(error Unsupported platform)
endif

MKDIR = mkdir -p
RM = rm -rf
PREFIX = /usr/local/bin

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(NAME) created"

$(OBJ_FILES): | $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiling $<"

$(OBJ_DIR):
	@$(MKDIR) $@

install: $(NAME)
	sudo cp $(NAME) $(PREFIX)/$(NAME)
	@echo "Installed to $(PREFIX)/$(NAME)"

uninstall:
	sudo rm -f $(PREFIX)/$(NAME)
	@echo "Removed from $(PREFIX)"

reinstall: uninstall install

clean:
	@$(RM) $(OBJ_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all install uninstall reinstall clean fclean re