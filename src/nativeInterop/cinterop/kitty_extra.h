#ifndef KITTY_EXTRA
#define KITTY_EXTRA

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>


#define ESC "\x1B" /* ESCAPE */
#define BKS "\x5c" /* BACKSLASH */

char error_msg[256] = "\0";

typedef struct /* using Kotlin naming convention */
{
	unsigned int rows;
	unsigned int cols;
	unsigned int width;
	unsigned int height;
	unsigned int cellWidth;
	unsigned int cellHeight;

} KittyContext;

KittyContext _ctx = { 0, 0, 0, 0, 0, 0 };

void (*_term_handler)(void) = NULL;

static void kitty_println(const char* msg);

static bool kitty_get_window_size(struct winsize* sz);


static void kitty_set_term_handler(void (*function)(void))
{
    _term_handler = function;
}


void _signal_handler(int signum)
{
    char* msg[100];
    snprintf(msg, 100, "Interrupt signal (%u) received.", signum);
    kitty_println(msg);

    if (signum == SIGWINCH && _term_handler) {
        _term_handler();
    } else {
        kitty_restore_termios();
        exit(0);
    }
}


static KittyContext* kitty_create_context()
{
    kitty_setup_termios();

    // register terminal changed signal (SIGWINCH)
    signal(SIGWINCH, _signal_handler);

    struct winsize sz;
    if (kitty_get_window_size(&sz)) {
        _ctx.rows = sz.ws_row;
        _ctx.cols = sz.ws_col;
        _ctx.width = sz.ws_xpixel;
        _ctx.height = sz.ws_ypixel;
        _ctx.cellWidth = sz.ws_xpixel / sz.ws_col;
        _ctx.cellHeight = sz.ws_ypixel / sz.ws_row;
        return &_ctx;
    }

    return NULL;
}


static void kitty_print_error_msg()
{
    if (error_msg[0]) {
        kitty_restore_termios();
        printf("* %s\n", error_msg);
        kitty_setup_termios();
    }
}


static void kitty_die(const char* msg)
{
    kitty_restore_termios();
    printf("* %s\n", msg);
    exit(1);
}


static void kitty_println(const char* msg)
{
    kitty_restore_termios();
    printf("%s\n", msg);
    kitty_setup_termios();
}


static bool kitty_get_window_size(struct winsize* sz)
{
    ioctl(0, TIOCGWINSZ, sz);
    if (sz->ws_xpixel && sz->ws_ypixel) {
        return true;
    }
    return false;
}


static bool get_random_number()
{
    static bool first_time = true;

    if (first_time) {
        srand(time(NULL));
        first_time = false;
    }

    return abs(rand());
}


static bool kitty_check_graphics_support()
{
    // sending RGB(f=24) image with ID(i)=1 and width(s)=1 and height(v)=1
    static char* graphics_query = ESC "_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA" ESC BKS ESC "[c";
    line line = { 0, { 0 }};

    line = kitty_send_term(graphics_query);
    kdata k = kitty_parse_response(line);
    return k.iid >= 0;
}


static bool kitty_check_local_execution()
{
    char buf[PATH_MAX];
    const size_t BASE64_SIZE = ((PATH_MAX + 2) / 3) * 4;
    char buf64[BASE64_SIZE];

    // get absolute path and convert to base64
    realpath("./assets/testing.png", buf);
    base64_encode(strlen(buf), buf, BASE64_SIZE, buf64);

    // prepare query
    unsigned int id = 1;
    char local_query[BASE64_SIZE + 25];
    snprintf(local_query, BASE64_SIZE + 25, ESC "_Ga=q,i=%i,f=100,t=f;%s" ESC BKS, id, buf64);
    line line = { 0, { 0 }};

    line = kitty_send_term(local_query);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        if (strstr(k.data.buf, ";OK" ESC)) {
            return true;
        }

        char response[50];
        snprintf(response, 50, ESC "_Gi=%i;", id);

        for (int i = 0, j = 0; i < k.data.r + 1; ++i) {
            if (k.data.buf[i] > 31) {
                error_msg[j++] = k.data.buf[i];
            }
        }

        kitty_die(error_msg);
    }

    return false;
}


static bool kitty_store_image(unsigned int id, const char* fname)
{
    char buf[PATH_MAX];
    const size_t BASE64_SIZE = ((PATH_MAX + 2) / 3) * 4;
    char buf64[BASE64_SIZE];

    // get absolute path and convert to base64
    realpath(fname, buf);
    base64_encode(strlen(buf), buf, BASE64_SIZE, buf64);

    // prepare query
    char local_query[BASE64_SIZE + 25];
    snprintf(local_query, BASE64_SIZE + 25, ESC "_Gi=%u,f=100,t=f;%s" ESC BKS, id, buf64);
    line line = kitty_send_term(local_query);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        if (strstr(k.data.buf, ";OK" ESC)) {
            return true;
        }

        char response[50];
        snprintf(response, 50, ESC "_Gi=%u;", id);

        for (int i = strlen(response), j = 0; i < k.data.r + 1; ++i) {
            if (k.data.buf[i] > 31) {
                error_msg[j++] = k.data.buf[i];
            } else {
                error_msg[j++] = 0;
                break;
            }
        }

        kitty_die(error_msg);
    }

    return false;
}


struct Image
{
    unsigned int id;
    unsigned int placement;
    unsigned int zindex;

    bool use_offset;
    struct {
        int x, y;
    } offset_data;

    bool use_viewport;
    struct {
        int x, y, w, h;
    } viewport_data;
};


static bool _image_cmd(const char* command, struct Image* data)
{
    line line = kitty_send_term(command);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        char response[50];

        if (strstr(k.data.buf, ";OK" ESC)) {
            return true;
        }

        if (data != NULL) {
            char cell_offset[20];
            char viewport[20];

            if (data->use_offset) {
                int x = _ctx.cellWidth / data->offset_data.x;
                int off_x = _ctx.cellWidth % data->offset_data.x;
                int y = _ctx.cellHeight / data->offset_data.y;
                int off_y = _ctx.cellHeight % data->offset_data.y;
                kitty_set_position(x, y);
                snprintf(cell_offset, 20, ",X=%u,Y=%u", off_x, off_y);
            } else {
                cell_offset[0] = '\0';
            }

            if (data->use_viewport) {
                snprintf(viewport, 20, ",x=%u,y=%u,w=%u,h=%u", data->viewport_data.x, data->viewport_data.y, data->viewport_data.w, data->viewport_data.h);
            } else {
                viewport[0] = '\0';
            }

            if (!data->placement) {
                snprintf(response, 50, ESC "_Gi=%u%s%s;", data->id, cell_offset, viewport);
            } else {
                snprintf(response, 50, ESC "_Gi=%u,p=%u%s%s;", data->id, data->placement, cell_offset, viewport);
            }
        }

        for (int i = strlen(response), j = 0; i < k.data.r + 1; ++i) {
            if (k.data.buf[i] > 31) {
                error_msg[j++] = k.data.buf[i];
            } else {
                error_msg[j++] = 0;
                break;
            }
        }

        kitty_die(error_msg);
    }

    return false;
}


static bool kitty_free_image(unsigned int id)
{
    // prepare command
    char command[20];
    if (id) {
        snprintf(command, 20, ESC "_Ga=d,d=I,i=%u" ESC BKS, id);
    } else {
        snprintf(command, 20, ESC "_Ga=d,d=A" ESC BKS);
    }

    return _image_cmd(command, NULL);
}


static bool kitty_display_image(unsigned int id)
{
    struct Image data = { id, 0, 0, false, {}, false, {} };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u" ESC BKS, id);
    return _image_cmd(command, &data);
}


static bool kitty_display_image2(unsigned int id, unsigned int placement)
{
    struct Image data = { id, placement, 0, false, {}, false, {} };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u,p=%u" ESC BKS, id, placement);
    return _image_cmd(command, &data);
}


static bool kitty_display_image3(unsigned int id, unsigned int placement, unsigned int zindex)
{
    struct Image data = { id, placement, zindex, false, {}, false, {} };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u,p=%u,z=%u" ESC BKS, id, placement, zindex);
    return _image_cmd(command, &data);
}


static bool kitty_hide_image(unsigned int id, unsigned int placement)
{
    // prepare command
    char command[20];
    if (!id) {
        snprintf(command, 20, ESC "_Ga=d,d=a" ESC BKS);
    } else {
        if (placement) {
            snprintf(command, 20, ESC "_Ga=d,d=i,i=%u,p=%u" ESC BKS, id, placement);
        } else {
            snprintf(command, 20, ESC "_Ga=d,d=i,i=%u" ESC BKS, id);
        }
    }
    return _image_cmd(command, NULL);
}

#endif /* end of include guard: KITTY_EXTRA */
