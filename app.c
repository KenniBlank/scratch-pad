#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "header.h"

/* Global Variables */
// Dynamic array to store points
Point* points = NULL;
int pointCount = 0;
int pointCapacity = 0;

bool DarkMode = false;
SDL_Color text_color;
SDL_Color background_color;

/* Function Definition: */
void setPixel(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, float intensity);
void better_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2);
void addPoint(SDL_Renderer* renderer, int x, int y, bool connect);
void RenderPoint(SDL_Renderer* renderer, Point p1, Point p2);
void ReRenderPoints(SDL_Renderer* renderer);

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
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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

    // Setup
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Blending mode enabled
    SDL_SetRenderDrawColor(renderer, unpack_color(background_color)); // First look color
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    text_color = DarkMode? (SDL_Color) {255, 255, 255, 255}: (SDL_Color) {0, 0, 0, 255};
    background_color = DarkMode? (SDL_Color) {0, 0, 0, 255}: (SDL_Color) {255, 255, 255, 255};

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    // CTRL is super key
                    if (event.key.keysym.mod & KMOD_LCTRL) {
                        //  CTRL + C to clear the board
                        if (event.key.keysym.sym == SDLK_c) {
                            pointCount = 0; // Reset points

                            // Clear board
                            SDL_SetRenderDrawColor(renderer, unpack_color(background_color));
                            SDL_RenderClear(renderer);
                            SDL_RenderPresent(renderer);
                        }

                        if (event.key.keysym.sym == SDLK_d) {
                            swap(&background_color, &text_color);
                            DarkMode = !DarkMode;
                            ReRenderPoints(renderer);
                            SDL_RenderPresent(renderer);
                        }
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
                        addPoint(renderer, event.button.x, event.button.y, isDrawing);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        isDrawing = false;
                        addPoint(renderer, event.button.x, event.button.y, isDrawing);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (isDrawing) {
                        addPoint(renderer, event.motion.x, event.motion.y, isDrawing); // Store the new point
                    }
                    break;
            }
        }

        ReRenderPoints(renderer); // Redraw all stored points

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS); // 144 FPS
    }

    // Cleanup
    free(points); // Free the dynamically allocated memory
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// Set pixel with intensity blending
void setPixel(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, float intensity) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, r, g, b, (Uint8)(a * intensity));
    SDL_RenderDrawPoint(renderer, x, y);
}

// Improved Wu’s Anti-Aliased Line Algorithm
void better_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    int steep = abs(y2 - y1) > abs(x2 - x1);

    if (steep) {
        swap(&x1, &y1);
        swap(&x2, &y2);
    }

    if (x1 > x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    float dx = x2 - x1;
    float dy = y2 - y1;
    float gradient = (dx == 0.0) ? 1.0 : dy / dx;
    float intery = y1 + gradient * (x1 - x1);

    // First endpoint
    int xend = x1;
    int yend = round(y1);
    float xgap = 1 - (x1 + 0.5 - floor(x1 + 0.5));
    int xpxl1 = xend;
    int ypxl1 = yend;

    if (steep) {
        setPixel(renderer, ypxl1, xpxl1, unpack_color(text_color), (1 - (intery - floor(intery))) * xgap);
        setPixel(renderer, ypxl1 + 1, xpxl1, unpack_color(text_color), (intery - floor(intery)) * xgap);
    } else {
        setPixel(renderer, xpxl1, ypxl1, unpack_color(text_color), (1 - (intery - floor(intery))) * xgap);
        setPixel(renderer, xpxl1, ypxl1 + 1, unpack_color(text_color), (intery - floor(intery)) * xgap);
    }

    intery += gradient;

    // Middle points
    for (int x = xpxl1 + 1; x < x2; x++) {
        int y = floor(intery);
        float f = intery - y;

        if (steep) {
            setPixel(renderer, y, x, unpack_color(text_color), 1 - f);
            setPixel(renderer, y + 1, x, unpack_color(text_color), f);
        } else {
            setPixel(renderer, x, y, unpack_color(text_color), 1 - f);
            setPixel(renderer, x, y + 1, unpack_color(text_color), f);
        }
        intery += gradient;
    }
}

// Function to add a point to the array
void addPoint(SDL_Renderer* renderer, int x, int y, bool connect) {
    if (pointCount >= pointCapacity) {
        // Resize the array if needed
        pointCapacity = pointCapacity == 0 ? 1 : pointCapacity * 2;
        Point* temp = realloc(points, pointCapacity * sizeof(Point));
        if (!temp) {
            fprintf(stderr, "Memory allocation failed!\n");
            free(points); // Free existing memory
            exit(1);
        }
        points = temp;
    }
    points[pointCount].x = x;
    points[pointCount].y = y;
    points[pointCount].connect = connect;
    pointCount++;
}

void RenderPoint(SDL_Renderer* renderer, Point p1, Point p2) {
    if (p1.connect && p2.connect) {
        SDL_SetRenderDrawColor(renderer, unpack_color(text_color));
        better_line(renderer, p1.x, p1.y, p2.x, p2.y);
    }
}

// Function to redraw all stored points
void ReRenderPoints(SDL_Renderer* renderer) {
        // Background Color
        SDL_SetRenderDrawColor(renderer, unpack_color(background_color));
        SDL_RenderClear(renderer);

        // Draw Color
        SDL_SetRenderDrawColor(renderer, unpack_color(text_color));
        for (int i = 0; i < pointCount - 1; i++) {
                RenderPoint(renderer, points[i], points[i + 1]);
        }
}
