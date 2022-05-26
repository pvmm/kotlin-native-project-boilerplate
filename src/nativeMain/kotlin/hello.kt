import kitty_util.*

import kotlinx.cinterop.alloc
//import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
//import kotlinx.cinterop.readValue

fun main() {
    var width : UShort?
    var height : UShort?
    var local_exec : Boolean?

    memScoped {
        kitty_setup_termios()

        val size = alloc<winsize>()

        if (get_window_size(size.ptr)) {
            width = size.ws_xpixel
            height = size.ws_ypixel
        } else {
            die("Kitty required: window size is unknown.")
            return
        }

        if (!check_graphics_support()) {
            die("Kitty required: graphics support not supported.")
            return
        }

        local_exec = check_local_execution().let {
            kitty_restore_termios()
            if (it) {
                println("* Local execution detected.")
            } else {
                println("* Non-local execution detected.")
            }
            kitty_setup_termios()
            it;
        }

        val moo = store_image(1, "./assets/moo.png").let {
            kitty_restore_termios()
            if (it) {
                println("* Image stored successfully.")
            } else {
                println("* Image couldn't be stored.")
            }
            kitty_setup_termios()
        }

        kitty_restore_termios();
        println("* Window size is $width x $height.")
        println("* Graphics support is OK.")
        //print_error_msg();

        return;
    }
}
