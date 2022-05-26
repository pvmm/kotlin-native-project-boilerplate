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
            if (it) {
                println2("* Local execution detected.")
            } else {
                println2("* Non-local execution detected.")
            }
            it;
        }

        store_image(1, "./assets/cow_large.png").let {
            if (it) {
                display_image(1).let {
                    if (it) println2("\n* Image displayed.")
                }
            } else {
                println2("* Image couldn't be stored.")
            }
        }

        kitty_restore_termios();
        println("* Window size is $width x $height.")
        println("* Graphics support is OK.")

        return;
    }
}
