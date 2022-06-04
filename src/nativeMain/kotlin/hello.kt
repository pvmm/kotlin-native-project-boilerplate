import kitty_util.*

import kotlinx.cinterop.alloc
import kotlinx.cinterop.cValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.ptr
import kotlinx.cinterop.staticCFunction
import kotlinx.cinterop.CPointer
import kotlinx.cinterop.pointed


fun term_handler()
{
    println("terminal changed.");
}


fun main() {
    var local_exec : Boolean?

    memScoped {
        val ctx : CPointer<KittyContext>? = kitty_create_context().let {
            if (it == null) kitty_die("Kitty required: window size is unknown.")
            it
        }

        /* Set terminal handler */
        kitty_set_term_handler(staticCFunction<Unit> { term_handler() });

        if (!kitty_check_graphics_support()) {
            kitty_die("Kitty required: graphics not supported.")
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

        ctx?.pointed?.apply {
            println("* Window size is ${width} x ${height}.")
            println("* Window size is ${cols} x ${rows} cells.")
            println("* Cell size is ${cellWidth} x ${cellHeight}.")
        }

        println("* Graphics support is OK.")
    }

    return;
}
