CFLAGS = -Wall
DEBUG = -g
LIBS = -lSDL2 -lSDL2_image -lm -lSDL2_ttf

CFiles = main.c
App = Scratch\ Pad

all: compile

compile:
	gcc $(CFiles) -o $(App) $(CFLAGS) $(LIBS) $(DEBUG)

run: compile
	./$(App)
	@echo -e "\nProgram Return Value: " $$?

move: compile
	@mv ./$(App) ~/
	@echo "Successfull!"

clean:
	@rm -f $(App)
	@rm Images/__image__*.png
