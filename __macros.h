#define FontLocation "/home/kenni/Experimental/scratch-pad/fonts/ComingSoon_bold.ttf"

#ifdef RELEASE
    #define FOLDER "images/"
#else
    #define FOLDER "/home/kenni/Pictures/"
#endif

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 600

// TODO: Render in this so that resizing doesn't affect
#define RENDER_WINDOW_WIDTH 3840
#define RENDER_WINDOW_HEIGHT 2160

#define FONT_SIZE 16
#define POINTS_THRESHOLD 1 // In pixel: basically how much gap minimum should be between points minimum

#define print(fmt, ...) \
    do { \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout); \
    } while(0)

#define unpack_color(color) (color.r), (color.g), (color.b), (color.a)
#define swap(a, b) \
    do { \
        typeof(*a) temp = *a; \
        *a = *b; \
        *b = temp; \
    } while (0)
