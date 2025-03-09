CFLAGS = -Wall
LDFLAGS = -lSDL2

all: compile

compile:
	gcc app.c -o app $(CFLAGS) $(LDFLAGS)
	clear
	./app;

	@echo -e "\nProgram Return Value: " $$?

clean:
	rm -f app
