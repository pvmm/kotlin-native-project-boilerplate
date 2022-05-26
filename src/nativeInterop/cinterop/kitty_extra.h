#ifndef KITTY_EXTRA
#define KITTY_EXTRA

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>

#define ESC "\x1B" /* ESCAPE */
#define BKS "\x5c" /* BACKSLASH */

char error_msg[256] = "\0";

static void print_error_msg()
{
    if (error_msg[0]) {
        kitty_restore_termios();
        printf("* %s\n", error_msg);
        kitty_setup_termios();
    }
}


static void die(const char* msg)
{
    kitty_restore_termios();
    printf("* %s\n", msg);
    exit(1);
}


static void println2(const char* msg)
{
    kitty_restore_termios();
    printf("%s\n", msg);
    kitty_setup_termios();
}


static bool get_window_size(struct winsize* sz)
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


static bool check_graphics_support()
{
    // sending RGB(f=24) image with ID(i)=1 and width(s)=1 and height(v)=1
    static char* graphics_query = ESC "_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA" ESC BKS ESC "[c";
    line line = { 0, { 0 }};

    line = kitty_send_term(graphics_query);
    kdata k = kitty_parse_response(line);
    return k.iid >= 0;
}


static bool check_local_execution()
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

        die(error_msg);
    }

    return false;
}


static bool store_image(unsigned int id, const char* fname)
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

        die(error_msg);
    }

    return false;
}


struct Image {
    unsigned int id;
    unsigned int placement;
    unsigned int zindex;
};

static bool _image_cmd(const char* command, struct Image* data)
{
    line line = kitty_send_term(command);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        if (strstr(k.data.buf, ";OK" ESC)) {
            return true;
        }

        char response[50];
        if (!data->placement) {
            snprintf(response, 50, ESC "_Gi=%u;", data->id);
        } else {
            snprintf(response, 50, ESC "_Gi=%u,p=%u;", data->id, data->placement);
        }

        for (int i = strlen(response), j = 0; i < k.data.r + 1; ++i) {
            if (k.data.buf[i] > 31) {
                error_msg[j++] = k.data.buf[i];
            } else {
                error_msg[j++] = 0;
                break;
            }
        }

        die(error_msg);
    }

    return false;
}


static bool display_image(unsigned int id)
{
    struct Image data = { id, 0, 0 };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u" ESC BKS, id);
    return _image_cmd(command, &data);
}


static bool display_image2(unsigned int id, unsigned int placement)
{
    struct Image data = { id, placement, 0 };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u,p=%u" ESC BKS, id, placement);
    return _image_cmd(command, &data);
}


static bool display_image3(unsigned int id, unsigned int placement, unsigned int zindex)
{
    struct Image data = { id, placement, zindex };

    // prepare command
    char command[25];
    snprintf(command, 25, ESC "_Ga=p,i=%u,p=%u,z=%u" ESC BKS, id, placement, zindex);
    return _image_cmd(command, &data);
}

#endif /* end of include guard: KITTY_EXTRA */
