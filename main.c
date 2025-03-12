#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#include <sys/stat.h>

#include "__macros.h"
#include "__struct.h"

void setPixel(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, float intensity);
void better_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness);
void addPoint(SDL_Renderer* renderer, int x, int y, int line_thickness, bool connect);
void RenderPoint(SDL_Renderer* renderer, Point p1, Point p2);
void ReRenderPoints(SDL_Renderer* renderer);

void SaveAsImage(SDL_Renderer* renderer);

/* Global Variables */
// Dynamic array to store points
Point* points = NULL;
int pointCount = 0;
int pointCapacity = 0;

// Colors:
bool DarkMode = true;
SDL_Color text_color;
SDL_Color background_color;

bool rand_bool(void) {
	if (rand() > RAND_MAX / 2) return true;
	return false;
}

int main(void) {
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

    text_color = DarkMode? (SDL_Color) {255, 255, 255, 255}: (SDL_Color) {15, 20, 25, 255};
    background_color = DarkMode? (SDL_Color) {15, 20, 25, 255}: (SDL_Color) {255, 255, 255, 255};

    int line_thickness = 3;

    SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    SDL_SetCursor(cursor);

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
                            swap(&text_color, &background_color);
                            DarkMode = !DarkMode;
                            ReRenderPoints(renderer);
                            SDL_RenderPresent(renderer);
                        }

                        if (event.key.keysym.sym == SDLK_s){
                            SaveAsImage(renderer);
                            printf("Image Saved...\n");
                        }
                    }

                    if (event.key.keysym.sym == SDLK_KP_PLUS) {
                        line_thickness += 1;
                        printf("++\n");
                    }
                    if (event.key.keysym.sym == SDLK_KP_MINUS) {
                        line_thickness -= 1;
                        printf("--\n");
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
                        addPoint(renderer, event.button.x, event.button.y, line_thickness, isDrawing);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        isDrawing = false;
                        addPoint(renderer, event.button.x, event.button.y, line_thickness, isDrawing);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (isDrawing) {
                        addPoint(renderer, event.motion.x, event.motion.y, line_thickness, isDrawing); // Store the new point
                    }
                    break;
            }
        }
        ReRenderPoints(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }
    // Cleanup
    SDL_FreeCursor(cursor);
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
void better_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness) {
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

    // Calculate the perpendicular slope
    float perpendicular_gradient = (gradient == 0.0) ? 1.0 : -1.0 / gradient;

    // Calculate the offset for thickness
    float thickness_offset = (thickness - 1) / 2.0;

    // Draw multiple lines to create thickness
    for (int i = -thickness_offset; i <= thickness_offset; i++) {
        float offset_x = i * cos(atan(perpendicular_gradient));
        float offset_y = i * sin(atan(perpendicular_gradient));

        // Adjust the starting and ending points for the current parallel line
        int adjusted_x1 = x1 + offset_x;
        int adjusted_y1 = y1 + offset_y;
        int adjusted_x2 = x2 + offset_x;
        int adjusted_y2 = y2 + offset_y;

        // Recalculate intermediate values for the adjusted line
        float adjusted_dx = adjusted_x2 - adjusted_x1;
        float adjusted_dy = adjusted_y2 - adjusted_y1;
        float adjusted_gradient = (adjusted_dx == 0.0) ? 1.0 : adjusted_dy / adjusted_dx;
        float intery = adjusted_y1 + adjusted_gradient * (adjusted_x1 - adjusted_x1);

        // First endpoint
        int xend = adjusted_x1;
        int yend = round(adjusted_y1);
        float xgap = 1 - (adjusted_x1 + 0.5 - floor(adjusted_x1 + 0.5));
        int xpxl1 = xend;
        int ypxl1 = yend;

        if (steep) {
            setPixel(renderer, ypxl1, xpxl1, unpack_color(text_color), (1 - (intery - floor(intery))) * xgap);
            setPixel(renderer, ypxl1 + 1, xpxl1, unpack_color(text_color), (intery - floor(intery)) * xgap);
        } else {
            setPixel(renderer, xpxl1, ypxl1, unpack_color(text_color), (1 - (intery - floor(intery))) * xgap);
            setPixel(renderer, xpxl1, ypxl1 + 1, unpack_color(text_color), (intery - floor(intery)) * xgap);
        }

        intery += adjusted_gradient;

        // Middle points
        for (int x = xpxl1 + 1; x < adjusted_x2; x++) {
            int y = floor(intery);
            float f = intery - y;

            if (steep) {
                setPixel(renderer, y, x, unpack_color(text_color), 1 - f);
                setPixel(renderer, y + 1, x, unpack_color(text_color), f);
            } else {
                setPixel(renderer, x, y, unpack_color(text_color), 1 - f);
                setPixel(renderer, x, y + 1, unpack_color(text_color), f);
            }
            intery += adjusted_gradient;
        }
    }
}

// Function to add a point to the array
void addPoint(SDL_Renderer* renderer, int x, int y, int line_thickness, bool connect) {
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

    bool add_point;

    add_point = !connect ? true: pow(pow(points[pointCount - 1].x - x, 2) + pow(points[pointCount - 1].y - y, 2), 0.5) > POINTS_THRESHOLD ? true: false;

    if (add_point){
        points[pointCount].x = x;
        points[pointCount].y = y;
        points[pointCount].connect = connect;
        points[pointCount].line_thickness = line_thickness;
        pointCount++;
    }
}

void RenderPoint(SDL_Renderer* renderer, Point p1, Point p2) {
    if (p1.connect && p2.connect) {
        SDL_SetRenderDrawColor(renderer, unpack_color(text_color));
        better_line(renderer, p1.x, p1.y, p2.x, p2.y, (p1.line_thickness + p1.line_thickness) / 2);
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

int image_name(char* folder, char* returnValue, size_t returnValueSize) {
    if (!returnValue || returnValueSize == 0) return -1; // Error: Invalid buffer

    // Ensure folder Exists:
    struct stat st = {0};
    if (stat(folder, &st) == -1) {
        mkdir(folder, 0700);
    }

    // Get file count in the folder
    char command[256];
    snprintf(command, sizeof(command), "ls \"%s\" | wc -l", folder);

    FILE *fp = popen(command, "r");
    if (!fp) {
        printf("Failed to run command\n");
        return 1;
    }

    int count = 0;
    if (fscanf(fp, "%d", &count) != 1) {
        printf("Failed to read file count\n");
        pclose(fp);
        return 1;
    }
    pclose(fp);

    // Filepath
    snprintf(returnValue, returnValueSize, "%s__image__%03d.png", folder, count);
    return 0;
}

void SaveAsImage(SDL_Renderer* renderer) {
    int win_width, win_height;
    SDL_GetRendererOutputSize(renderer, &win_width, &win_height);

    // Create surface with current window dimensions
    SDL_Surface *surface = SDL_CreateRGBSurface(0, win_width, win_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        printf("Unable to create surface: %s\n", SDL_GetError());
        return;
    }

    // Check surface->pixels before using it
    if (!surface->pixels) {
        printf("Surface pixels are NULL after creation\n");
        SDL_FreeSurface(surface);
        surface = NULL;
        return;
    }

    if (SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) {
        printf("Unable to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    char filename[256];
    image_name(FOLDER, filename, sizeof(filename));

    // Save surface to PNG
    if (IMG_SavePNG(surface, filename) != 0) {
        printf("Unable to save frame as PNG: %s\n", IMG_GetError());
    }

    SDL_FreeSurface(surface);
}
