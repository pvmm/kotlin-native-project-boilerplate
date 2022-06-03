import kitty_util.*

import kotlinx.cinterop.alloc
import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr


fun main() {
    var local_exec : Boolean?

    memScoped {
        val ctx = alloc<KittyContext> {
            rows = 0u; cols = 0u; width = 0u; height = 0u; cellWidth = 0u; cellHeight = 0u
        }

	if (!kitty_create_context(ctx.ptr)) {
            kitty_die("Kitty required: window size is unknown.")
	} 

        if (!kitty_check_graphics_support()) {
            kitty_die("Kitty required: graphics support not supported.")
        }

        local_exec = kitty_check_local_execution().let {
            if (it) {
                kitty_println("* Local execution detected.")
            } else {
                kitty_println("* Non-local execution detected.")
            }
            it;
        }

        kitty_store_image(1, "./assets/cow_large.png").let {
            if (it) {
                kitty_display_image(1).let {
                    if (it) kitty_println("\n* Image displayed.")
                }
            } else {
                kitty_println("* Image couldn't be stored.")
            }
        }

        kitty_restore_termios()
        println("* Window size is ${ctx.width} x ${ctx.height}.")
        println("* Window size is ${ctx.cols} x ${ctx.rows} cells.")
        println("* Cell size is ${ctx.cellWidth} x ${ctx.cellHeight}.")
        println("* Graphics support is OK.")
    }

    return;
}
