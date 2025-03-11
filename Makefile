CFLAGS = -Wall
DEBUG = -g
LIBS = -lSDL2 -lSDL2_image -lm

all: compile

compile:
	gcc app.c -o app $(CFLAGS) $(LIBS) $(DEBUG)
	./app

	@echo -e "\nProgram Return Value: " $$?

clean:
	@rm -f app
	@rm Images/*.png
