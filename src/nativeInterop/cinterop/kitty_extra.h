#ifndef KITTY_EXTRA
#define KITTY_EXTRA

#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdbool.h>

#define ESC "\x1B"

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


// #define BASE64_FILENAME "dGVzdGluZy5wbmc="                  // "testing.png" (without newline)
// #define BASE64_FILENAME "YXNzZXRzL3Rlc3RpbmcucG5n"       // assert/testing.png
#define BASE64_FILENAME "Li9hc3NldHMvdGVzdGluZy5wbmc="   // ./assert/testing.png

static bool check_local_filesystem()
{
    static const char* local_query = ESC "_Ga=q,i=31,f=100,t=f;" BASE64_FILENAME ESC "\\";
    line line = { 0, { 0 }};

    line = kitty_send_term(local_query);
    kdata k = kitty_parse_response(line);

    if (k.data.r > 0) {
        return strstr(k.data.buf, ";OK" ESC);
    }
    return false;
}

#endif /* end of include guard: KITTY_EXTRA */
