#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Define a structure to store a point
typedef struct {
    int x, y;
    bool connect;
} Point;

// Dynamic array to store points
Point* points = NULL;
int pointCount = 0;
int pointCapacity = 0;

// Function to add a point to the array
void addPoint(int x, int y, bool connect) {
    if (pointCount >= pointCapacity) {
        // Resize the array if needed
        pointCapacity = pointCapacity == 0 ? 1 : pointCapacity * 2;
        points = realloc(points, pointCapacity * sizeof(Point));
        if (!points) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
    }
    points[pointCount].x = x;
    points[pointCount].y = y;
    points[pointCount].connect = connect;
    pointCount++;
}

// Function to redraw all stored points
void redrawPoints(SDL_Renderer* renderer) {
        // Background Color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw Color
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        for (int i = 0; i < pointCount - 1; i++) {
                if (points[i].connect && points[i + 1].connect) {
                    SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
                }
        }
}

int main() {

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Drawing with Points",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        700,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    SDL_Event event;
    bool running = true;
    bool isDrawing = false;

    // Clear to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_c) {
                        pointCount = 0; // Reset points

                        // Clear board and render white Background 
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderClear(renderer);
                        SDL_RenderPresent(renderer);
                    }
                    printf("Key pressed: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                    break;

                case SDL_KEYUP:
                    printf("Key released: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        isDrawing = true;
                        addPoint(event.button.x, event.button.y, isDrawing); // Store the initial point
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        redrawPoints(renderer);
                        isDrawing = false;
                        addPoint(event.motion.x, event.motion.y, isDrawing);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (isDrawing) {
                        addPoint(event.motion.x, event.motion.y, isDrawing); // Store the new point
                    }
                    break;
            }
        }

        if (isDrawing) {
                redrawPoints(renderer); // Redraw all stored points
                SDL_RenderPresent(renderer);
        }

        SDL_Delay(1000/144); // 144 FPS
    }

    // Cleanup
    free(points); // Free the dynamically allocated memory
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
