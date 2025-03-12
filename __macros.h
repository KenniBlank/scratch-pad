// #define FOLDER "/home/kenni/Experimental/scratch-pad/Images/"
#define FOLDER "/home/kenni/Pictures/"

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 600
#define FPS 144
#define POINTS_THRESHOLD 1 // In pixel: basically how much gap should be between points minimum

#define unpack_color(color) (color.r), (color.g), (color.b), (color.a)
#define swap(a, b) do { \
    typeof(*a) temp = *a; \
    *a = *b; \
    *b = temp; \
} while (0)
