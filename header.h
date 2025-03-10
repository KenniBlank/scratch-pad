// Define a structure to store a point
typedef struct {
    int x, y;
    bool connect;
} Point;

// Macros:
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 600
#define FPS 144


#define unpack_color(color) (color.r), (color.g), (color.b), (color.a)
#define swap(a, b) do { \
    typeof(*a) temp = *a; \
    *a = *b; \
    *b = temp; \
} while (0)
