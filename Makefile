CFLAGS = -Wall
DEBUG = -g
LIBS = -lSDL2 -lSDL2_image -lm

CFiles = main.c
App = Scratch\ Pad

all: compile

compile:
	gcc $(CFiles) -o $(App) $(CFLAGS) $(LIBS) $(DEBUG)
	./$(App)
	@echo -e "\nProgram Return Value: " $$?

move:
	mv ./$(App) ~/

clean:
	@rm -f $(App)
	@rm Images/__image__*.png
