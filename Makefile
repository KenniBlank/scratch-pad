CC = gcc

CFLAGS = -Wall -Wextra -Wshadow

DEBUGFLAGS = -g -DRELEASE
RELEASEFLAGS = -O2

LIBS = -lSDL2 -lSDL2_image -lm -lSDL2_ttf

CFiles = main.c
App = "Scratch Pad"

ifeq ($(BUILD),RELEASE)
    CFLAGS += $(DEBUGFLAGS)
else
    CFLAGS += $(RELEASEFLAGS)
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
	@rm -f "$(App)"
	@rm -f Images/__image__*.png
