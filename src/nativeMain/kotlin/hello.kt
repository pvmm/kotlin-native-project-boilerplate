import kitty_util.*

import kotlinx.cinterop.alloc
//import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
//import kotlinx.cinterop.readValue

fun check_graphics_support2()
{

}

fun main() {
    var width : UShort?
    var height : UShort?
    var local_term : Boolean?

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

        local_term = check_local_filesystem()

        /* kitty_restore_termios(); */
        kitty_restore_termios();
        println("* Window size is $width x $height.")
        println("* Graphics support is OK.")
        println("* Is filesystem local? " + local_term)

        return;
    }
}
