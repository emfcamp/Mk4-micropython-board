#include <stdint.h>
#include <string.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>

#include "py/mpconfig.h"
#include "py/stackctrl.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/compile.h"
#include "py/nlr.h"
#include "py/mperrno.h"

#include "lib/utils/pyexec.h"
#include "lib/mp-readline/readline.h"

static char * stack_top;
static UART_Handle console;

void mp_hal_stdout_tx_str(const char * str)
{
    UART_write(console, str, strlen(str));
}

#if MICROPY_PORT_MINIMAL_MAIN

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

int mp_main(void * heap, uint32_t heapsize, uint32_t stacksize,
            UART_Handle uart)
{
    int stack_dummy;

soft_reset:
    stack_top = (char*)&stack_dummy;
    // make stack limit somewhat smaller than full stack available
    mp_stack_set_limit(stacksize - 512);

    gc_init(heap, (uint8_t *)heap + heapsize);

    mp_init();

    console = uart;

    #if MICROPY_ENABLE_COMPILER
    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
                break;
            }
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }
    }
/*
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
*/
    #else
    pyexec_friendly_repl();
    #endif

    /* Magic String: needed by pyboard.py - do not change/remove */
    mp_hal_stdout_tx_str("PYB: soft reboot\r\n");

    extern void machine_teardown(void);
    machine_teardown();

    mp_deinit();
    goto soft_reset;

    return 0;
}

#else

int mp_main(void * heap, uint32_t heapsize, uint32_t stacksize,
            UART_Handle uart)
{
    int stack_dummy;

soft_reset:
    stack_top = (char *)&stack_dummy;
    mp_stack_set_top(stack_top);

    // make stack limit somewhat smaller than full stack available
    mp_stack_set_limit(stacksize - 512);

    gc_init(heap, (uint8_t *)heap + heapsize);
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));

    mp_obj_list_init(mp_sys_argv, 0);

    console = uart;

    pyexec_frozen_module("_boot.py");

    const char *boot_py = "/boot.py";
    mp_import_stat_t statf = mp_import_stat(boot_py);
    if (statf == MP_IMPORT_STAT_FILE) {
        pyexec_file(boot_py);
    }
    if (GPIO_read(0)) {
        mp_import_stat_t statf = mp_import_stat("/main.py");
        if (statf == MP_IMPORT_STAT_FILE) {
            pyexec_file("/main.py");
        }
    }

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
                break;
            }
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }
    }

    /* Magic String: needed by pyboard.py - do not change/remove */
    mp_hal_stdout_tx_str("PYB: soft reboot\r\n");

    extern void machine_teardown(void);
    machine_teardown();

    mp_deinit();
    goto soft_reset;

    return 0;
}
#endif

int mp_hal_stdin_rx_chr(void)
{
    char c;

    UART_read(console, &c, 1);

    return (int)c;
}

void mp_hal_stdout_tx_strn(const char * str, size_t len)
{
    UART_write(console, str, len);
}

void mp_hal_stdout_tx_strn_cooked(const char * str, size_t len)
{
    mp_hal_stdout_tx_strn(str, len);
}

void nlr_jump_fail(void * val)
{
    (void)val;
    while (1);
}

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) /
                    sizeof(mp_uint_t));
    gc_collect_end();
    //gc_dump_info();
}
