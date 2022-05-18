import kitty_util.*

import kotlinx.cinterop.alloc
//import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
//import kotlinx.cinterop.readValue

fun main() {
    var width : UShort?
    var height : UShort?
    /* var local_exec : Boolean? */

    memScoped {
        kitty_setup_termios();

        val size = alloc<winsize>()

        if (get_window_size(size.ptr)) {
            width = size.ws_xpixel;
            height = size.ws_ypixel;
        } else {
            kitty_restore_termios();
            println("Kitty required: window size is unknown.")
            return
        }

        if (!check_graphics_support()) {
            kitty_restore_termios();
            println("Kitty required: graphics support not supported.")
            return
        }

        check_local_execution().let {
            kitty_restore_termios();
            if (it) {
                println("* Local execution detected.")
            } else {
                println("* Non-local execution detected.")
            }
            it;
        }

        kitty_restore_termios();
        println("* Window size is $width x $height.")
        println("* Graphics support is OK.")
        print_error_msg();

        return;
    }
}
