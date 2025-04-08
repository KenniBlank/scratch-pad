CC = gcc

CFLAGS = -std=gnu2x -Wall -Wextra -Wshadow

DEBUGFLAGS = -g -DRELEASE
RELEASEFLAGS = -O2

LIBS = -lSDL2 -lSDL2_image -lm -lSDL2_ttf -lGL

CFiles = main.c
App = "Scratch Pad"

DEPENDENCIES = dependency/libtinyfiledialogs/tinyfiledialogs.c

ifeq ($(BUILD),RELEASE)
    CFLAGS += $(RELEASEFLAGS)
else
    CFLAGS += $(DEBUGFLAGS)
    # CFiles += $(DEPENDENCIES)
endif

all: compile

compile:
	$(CC) $(CFiles) -o $(App) $(CFLAGS) $(LIBS)

run: compile
	./$(App)
	@echo -e "\nProgram Return Value: $$?"

move: compile
	@mv ./$(App) ~/
	@echo "Successful!"

clean:
	@rm images/__image__*.png
	@rm $(App)
