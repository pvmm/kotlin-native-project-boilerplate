import kitty_util.*

import kotlinx.cinterop.alloc
//import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
//import kotlinx.cinterop.readValue

fun main() {
    var width : UShort?
    var height : UShort?
    var rows: UShort?
    var cols: UShort?
    var cell_w: UInt?
    var cell_h: UInt?
    var local_exec : Boolean?

    memScoped {
        kitty_setup_termios()

        val size = alloc<winsize>()

        if (get_window_size(size.ptr)) {
            width = size.ws_xpixel
            height = size.ws_ypixel
            cols = size.ws_col
            rows = size.ws_row
            cell_w = size.ws_xpixel / size.ws_col
            cell_h = size.ws_ypixel / size.ws_row
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
        println("* Window size is $cols x $rows cells.")
        println("* Cell size is $cell_w x $cell_h.")
        println("* Graphics support is OK.")

        return;
    }
}
