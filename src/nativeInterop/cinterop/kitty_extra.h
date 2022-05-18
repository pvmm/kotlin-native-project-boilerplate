#ifndef KITTY_EXTRA
#define KITTY_EXTRA

#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>

#define ESC "\x1B"

char error_msg[256] = "\0";

static bool get_window_size(struct winsize *sz)
{
    ioctl(0, TIOCGWINSZ, sz);
    if (sz->ws_xpixel && sz->ws_ypixel) {
        return true;
    }
    return false;
}


static bool check_graphics_support()
{
    // sending RGB(f=24) image with ID(i)=1 and width(s)=1 and height(v)=1
    static char* graphics_query = ESC "_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA\x1B\\" ESC "[c";
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
    char local_query[BASE64_SIZE + 23];
    snprintf(local_query, BASE64_SIZE + 23, ESC "_Ga=q,i=31,f=100,t=f;%s" ESC "\\", buf64);
    line line = { 0, { 0 }};

    line = kitty_send_term(local_query);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        if (strstr(k.data.buf, ";OK" ESC)) {
            return true;
        }

        for (int i = 0, j = 0; i < k.data.r + 1; ++i) {
            if (k.data.buf[i] > 31) {
                error_msg[j++] = k.data.buf[i];
            }
        }
    }

    return false;
}

static void print_error_msg()
{
    if (error_msg[0]) {
        printf("%s\n", error_msg);
    }
}

#endif /* end of include guard: KITTY_EXTRA */
