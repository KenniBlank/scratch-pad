#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_clipboard.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wchar.h>

#include "__macros.h"
#include "__struct.h"

void addPoint(int x, int y, int line_thickness, bool connect);
void RenderPoint(SDL_Renderer* renderer, Point p1, Point p2);
void ReRenderAllPoints(SDL_Renderer* renderer);

void add_user_input(char key_value);
void pop_user_input();
void RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int window_width, bool highlight);
void RenderIcons(SDL_Renderer* renderer, SDL_Texture* texture, size_t x, size_t y, size_t w, size_t h, SDL_Color color);

bool blinker_toggle_state();

// Helper Functions:
void setPixel(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, float intensity);
void better_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness);
void SaveAsImage(SDL_Renderer* renderer);
char* replace(const char* str, const char* old_substr, const char* new_substr);
char* append_string(char *s1, char *s2);

SDL_Texture* LoadImageAsTexture(const char* path, SDL_Renderer* renderer);
bool collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);

typedef struct {
        char *img_file_path, *Name;
        SDL_Color Color;
        size_t scale_x, scale_y;
        SDL_Texture* Texture;
} Icons;

Icons* all_icons = NULL;
size_t total_icons = 0;

/* Global Variables */
// Dynamic array to store points
Point* points = NULL;
size_t pointCount = 0;
size_t pointCapacity = 0;

char* usr_inputs = NULL;  // Dynamic string to store user input characters
size_t usr_inputs_len = 0; // Current length (number of characters stored, excluding the null terminator)
size_t usr_inputs_capacity = 0; // Capacity of the usr_inputs array

// Colors:
SDL_Color text_color;
SDL_Color background_color;

int main(void) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
            printf("TTF couldn't be initialized: %s\n", SDL_GetError());
            return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
      "Scratch Pad",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS
    );
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Blending mode enabled
    SDL_SetRenderDrawColor(renderer, unpack_color(background_color)); // First look color
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    IMG_Init(IMG_INIT_PNG);

    int font_size = FONT_SIZE;
    TTF_Font *font = TTF_OpenFont(FontLocation, font_size); // Load the font with the fixed size
    if (!font) {
        printf("Font loading failed: %s\n", TTF_GetError());
        return 1;
    }


    SDL_Cursor* DEFAULT_CURSOR = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    SDL_SetCursor(cursor);

    // Main loop
    SDL_Event event;

    // Essential Variables definition
    bool app_running = true;

    bool DarkMode = false;
    bool isDrawing = false;
    bool eraserMode = false;

    size_t line_thickness = 2;
    int window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;

    text_color = DarkMode? (SDL_Color) {255, 255, 255, 255}: (SDL_Color) {15, 20, 25, 255};
    background_color = DarkMode? (SDL_Color) {15, 20, 25, 255}: (SDL_Color) {255, 255, 255, 255};

    SDL_StartTextInput(); // Enable text input

    bool ctrlA_pressed = false;

    cursor = DEFAULT_CURSOR;
    SDL_SetCursor(cursor);

    while (app_running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    app_running = false;
                    break;

                case SDL_TEXTINPUT:
                    add_user_input(event.text.text[0]);
                    break;

                case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                                SDL_GetWindowSize(window, &window_width, &window_height);
                        }
                        break;

                case SDL_KEYDOWN:
                    // CTRL is super key
                    if (event.key.keysym.mod & KMOD_LCTRL) {
                        switch (event.key.keysym.sym) {
                            case SDLK_x:
                                if (ctrlA_pressed) {
                                        SDL_SetClipboardText(usr_inputs);
                                        ctrlA_pressed = false;
                                        if (usr_inputs) {
                                                free(usr_inputs);
                                                usr_inputs = NULL;
                                                usr_inputs_len = 0;
                                                usr_inputs_capacity = 0;
                                        }
                                }
                                break;

                            case SDLK_d:
                                swap(&text_color, &background_color);
                                DarkMode = !DarkMode;
                                ReRenderAllPoints(renderer);
                                SDL_RenderPresent(renderer);
                                break;

                            case SDLK_s:
                                SaveAsImage(renderer);
                                printf("Image Saved...\n");
                                break;

                            case SDLK_e:
                                eraserMode = !eraserMode;
                                break;

                            case SDLK_a:
                                ctrlA_pressed = !ctrlA_pressed;
                                break;

                            case SDLK_c:
                                if (ctrlA_pressed) {
                                        SDL_SetClipboardText(usr_inputs);
                                        ctrlA_pressed = false;
                                }
                                break;
                            case SDLK_KP_PLUS:
                                font_size += 1;
                                font = TTF_OpenFont(FontLocation, font_size);
                                break;
                            case SDLK_KP_MINUS:
                                font_size -= 1;
                                font = TTF_OpenFont(FontLocation, font_size);
                                break;
                        }
                    }

                    switch (event.key.keysym.sym) {
                        case SDLK_KP_PLUS:
                            line_thickness += 1;
                            break;

                        case SDLK_KP_MINUS:
                            line_thickness -= 1;
                            font_size -= 2;
                            font = TTF_OpenFont(FontLocation, font_size);;
                            break;

                        case SDLK_ESCAPE:
                            app_running = false;
                            break;

                        case SDLK_RETURN:
                            add_user_input('\n');
                            break;

                        case SDLK_BACKSPACE:
                                if (ctrlA_pressed) {
                                        // Reset All
                                        if (points) {
                                                pointCount = 0;
                                                pointCapacity = 0;
                                                free(points);
                                                points = NULL;
                                                // Clear board
                                                SDL_SetRenderDrawColor(renderer, unpack_color(background_color));
                                                SDL_RenderClear(renderer);
                                                SDL_RenderPresent(renderer);
                                        }
                                        if (usr_inputs) {
                                                free(usr_inputs);
                                                usr_inputs = NULL;
                                                usr_inputs_len = 0;
                                                usr_inputs_capacity = 0;
                                        }
                                        ctrlA_pressed = false;
                                } else {
                                        pop_user_input();
                                }
                                break;

                        case SDLK_TAB:
                            add_user_input('\t');
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (eraserMode) {

                    } else {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            isDrawing = true;
                            addPoint(event.button.x, event.button.y, line_thickness, isDrawing);
                            cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
                            SDL_SetCursor(cursor);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (eraserMode) {

                    } else {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            isDrawing = false;
                            addPoint(event.button.x, event.button.y, line_thickness, isDrawing);
                            cursor = DEFAULT_CURSOR;
                            SDL_SetCursor(cursor);
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (eraserMode) {

                    } else {
                        if (isDrawing) {
                            addPoint(event.motion.x, event.motion.y, line_thickness, isDrawing); // Store the new point
                            cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
                            SDL_SetCursor(cursor);
                        }
                    }
                    break;
            }
        }

        // Canvas Color i.e Background Color
        SDL_SetRenderDrawColor(renderer, unpack_color(background_color));
        SDL_RenderClear(renderer);

        ReRenderAllPoints(renderer);
        if (blinker_toggle_state()) {
            add_user_input('_');
            RenderText(renderer, font, usr_inputs, window_width, ctrlA_pressed);
            pop_user_input();
        } else {
            RenderText(renderer, font, usr_inputs, window_width, ctrlA_pressed);
        }

        SDL_RenderPresent(renderer);
    }
    // Cleanup
    SDL_FreeCursor(cursor);

    free(points);
    free(usr_inputs);

    SDL_StopTextInput(); // Disable text input
    TTF_CloseFont(font);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

// Set pixel with intensity blending
void setPixel(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, float intensity) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, (Uint8)(a * intensity));
    SDL_RenderDrawPoint(renderer, x, y);
}

// Improved Wu's Anti-Aliased Line Algorithm with Thickness
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

    float dx = (float)(x2 - x1);
    float dy = (float)(y2 - y1);
    float gradient = (dx == 0.0) ? 1.0 : dy / dx;

    // Calculate perpendicular gradient for thickness
    float perpendicular_gradient = (gradient == 0.0) ? 1.0 : -1.0 / gradient;

    // Loop through a range of thickness levels to draw a thick line
    for (int t = -(thickness / 2); t <= (thickness / 2); t++) {
        // Calculate offsets for the current thickness level
        float offset_x = t * cos(atan(perpendicular_gradient));
        float offset_y = t * sin(atan(perpendicular_gradient));

        // Adjust the starting and ending points based on the thickness offset
        int adjusted_x1 = x1 + offset_x;
        int adjusted_y1 = y1 + offset_y;
        int adjusted_x2 = x2 + offset_x;
        int adjusted_y2 = y2 + offset_y;

        // Recalculate gradient for the adjusted line
        float adjusted_dx = adjusted_x2 - adjusted_x1;
        float adjusted_dy = adjusted_y2 - adjusted_y1;
        float adjusted_gradient = (adjusted_dx == 0.0) ? 1.0 : adjusted_dy / adjusted_dx;
        float intery = adjusted_y1 + adjusted_gradient * (adjusted_x1 - x1);

        // Draw the first pixel
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

        // Draw the middle pixels
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
void addPoint(int x, int y, int line_thickness, bool connect) {
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
void ReRenderAllPoints(SDL_Renderer* renderer) {
        // Draw Color
        SDL_SetRenderDrawColor(renderer, unpack_color(text_color));
        if (pointCount != 0) {
            for (size_t i = 0; i < pointCount - 1; i++) {
                    RenderPoint(renderer, points[i], points[i + 1]);
            }
        }
}

int unique_name(char* folder, char* returnValue, size_t returnValueSize) {
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
    char* prefix = malloc(sizeof(char) * returnValueSize);
    strcpy(prefix, returnValue);

    another_name:
    snprintf(returnValue, returnValueSize, "%s%s%03d.png", folder, prefix, count);
    snprintf(command, sizeof(command), "ls \"%s\" | grep \"%s\" | wc -l", folder, returnValue);

    fp = popen(command, "r");
    if (!fp) {
        printf("Failed to run command\n");
        return 1;
    }
    if (fscanf(fp, "%d", &count) != 1) {
        printf("Failed to read file count\n");
        pclose(fp);
        return 1;
    }
    print("%d", count);
    if (count != 0) {
        count++;
        goto another_name;
    }

    pclose(fp);
    free(prefix);
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

    // Check surface -> pixels before using it
    if (!surface -> pixels) {
        printf("Surface pixels are NULL after creation\n");
        SDL_FreeSurface(surface);
        surface = NULL;
        return;
    }

    if (SDL_RenderReadPixels(renderer, NULL, surface -> format -> format, surface -> pixels, surface -> pitch) < 0) {
        printf("Unable to read pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    char filename[256] = "__image__"; // prefix of save file names
    unique_name(FOLDER, filename, sizeof(filename));

    // Save surface to PNG
    if (IMG_SavePNG(surface, filename) != 0) {
        printf("Unable to save frame as PNG: %s\n", IMG_GetError());
    }

    SDL_FreeSurface(surface);
}

void add_user_input(char key_value) {
    if ((size_t)(usr_inputs_len + 1) >= usr_inputs_capacity) {
        // +1 is for the null terminator
        usr_inputs_capacity = (usr_inputs_capacity == 0) ? 2 : usr_inputs_capacity * 2;
        char* temp = realloc(usr_inputs, usr_inputs_capacity * sizeof(char));
        if (!temp) {
            fprintf(stderr, "Memory allocation failed!\n");
            free(usr_inputs);
            exit(EXIT_FAILURE);
        }
        usr_inputs = temp;
    }

    // Append the character
    usr_inputs[usr_inputs_len] = key_value;
    usr_inputs_len++;
    // Null terminate the string
    usr_inputs[usr_inputs_len] = '\0';
}

// Function to replace all occurrences of a substring with another substring
char* replace(const char* str, const char* old_substr, const char* new_substr) {
    // Check for NULL pointers
    if (str == NULL || old_substr == NULL || new_substr == NULL) {
        return "";
    }

    // Calculate lengths of the input strings
    size_t str_len = strlen(str);
    size_t old_substr_len = strlen(old_substr);
    size_t new_substr_len = strlen(new_substr);

    // Count the number of occurrences of old_substr in str
    size_t count = 0;
    const char* tmp = str;
    while ((tmp = strstr(tmp, old_substr))) {
        count++;
        tmp += old_substr_len;
    }

    // Calculate the length of the new string after replacement
    size_t new_len = str_len + count * (new_substr_len - old_substr_len);

    // Allocate memory for the new string
    char* result = (char*)malloc(new_len + 1); // +1 for the null terminator
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    // Perform the replacement
    char* current_pos = result;
    const char* start = str;
    while (count--) {
        const char* found = strstr(start, old_substr);
        size_t segment_len = (found >= start) ? (size_t)(found - start) : 0;

        // Copy the segment before the found substring
        memcpy(current_pos, start, segment_len);
        current_pos += segment_len;

        // Copy the new substring
        memcpy(current_pos, new_substr, new_substr_len);
        current_pos += new_substr_len;

        // Move the start pointer past the old substring
        start = found + old_substr_len;
    }

    // Copy the remaining part of the string
    strcpy(current_pos, start);

    return result;
}

char* append_string(char *s1, char *s2) {
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);

    // Allocate Memory
    char* result = malloc((s1_len + s2_len + 1) * sizeof(char));
    if (!result) return NULL;

    // Copy s1
    for (size_t i = 0; i < s1_len; i++) {
        result[i] = s1[i];
    }

    for (size_t j = 0; j < s2_len; j++) {
        result[s1_len + j] = s2[j];
    }

    result[s1_len + s2_len] = '\0';
    free(s1);
    return result;
}

void RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int window_width, bool highlight) {
    const int PADDING = FONT_SIZE; // Padding for positioning
    int max_width_temp = window_width - 2 * PADDING;
    Uint32 max_width = max_width_temp > 0 ? (Uint32)max_width_temp : 0;

    char *formattedTxt = replace(replace(text, "\t", "    "), " ", "  ");
    if (highlight) {
        formattedTxt = replace(formattedTxt, "_\0", "\0");
    }

    if (!formattedTxt) {
        print("Couldn't Render text");
        return;
    }

    SDL_Color txt_color = text_color, bg_color = background_color;
    if (highlight) swap(&txt_color, &bg_color);

    SDL_Surface *textSurface = TTF_RenderText_Blended_Wrapped(font, formattedTxt, txt_color, max_width);
    free(formattedTxt);
    if (!textSurface) return;

    if (highlight) {
            // Create background surface
            SDL_Surface *bgSurface = SDL_CreateRGBSurfaceWithFormat(
                0, textSurface -> w, textSurface -> h, 32, SDL_PIXELFORMAT_RGBA32);
            if (!bgSurface) {
                SDL_FreeSurface(textSurface);
                return;
            }

            // Fill with highlight color
            SDL_FillRect(bgSurface, NULL,
                        SDL_MapRGBA(bgSurface -> format,
                                   bg_color.r,
                                   bg_color.g,
                                   bg_color.b,
                                   bg_color.a));

            // Blit text onto background
            SDL_BlitSurface(textSurface, NULL, bgSurface, NULL);
            SDL_FreeSurface(textSurface);

            // Use the composed surface for texture creation
            textSurface = bgSurface;
        }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) return;

    int textWidth = textSurface -> w;
    int textHeight = textSurface -> h;
    SDL_FreeSurface(textSurface);

    SDL_Rect textRect = {
        PADDING,
        PADDING,
        textWidth,
        textHeight
    };

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

void pop_user_input() {
        if (usr_inputs_len != 0) {
                usr_inputs_len -= 1;
        }
        usr_inputs[usr_inputs_len] = '\0';
}

bool blinker_toggle_state() {
        static bool state = false;
        static Uint32 last_toggle = 0;
        Uint32 now = SDL_GetTicks(); // Get time in milliseconds

        if (now - last_toggle >= 700) { // If 700 milli second has passed
                state = !state;
                last_toggle = now;
        }
        return state;
}

bool collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
        return ((x1 + width1 > x2) && (x1 < x2 + width2) && (y1 + height1 > y2) && (y1 < y2 + height2));
}

SDL_Texture* LoadImageAsTexture(const char* path, SDL_Renderer* renderer) {
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
                printf("Image Load Error: %s\n", IMG_GetError());
                return NULL;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
}

void RenderIcons(SDL_Renderer* renderer, SDL_Texture* texture, size_t x, size_t y, size_t w, size_t h, SDL_Color color) {
        SDL_SetTextureColorMod(texture, color.r * color.a, color.g * color.a, color.b * color.a);  // Tint the texture

        SDL_Rect dest;
        dest.w = w,
        dest.h = h,
        dest.x = (x - w) / 2;
        dest.y = (y - h) / 2;

        SDL_RenderCopy(renderer, texture, NULL, &dest);
}
