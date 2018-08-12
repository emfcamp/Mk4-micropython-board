#include <stdint.h>

// options to control how MicroPython is built

#define MICROPY_PY_SYS_PLATFORM     "TI-RTOS"

// board-specific settings, including network control
#include "mpconfigboard.h"

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)

#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_QSTR_EXTRA_POOL     mp_qstr_frozen_const_pool
#define MICROPY_ALLOC_PATH_MAX      (256)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (16)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_GC_ALLOC_THRESHOLD  (0)
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_REPL_AUTO_INDENT    (1)
#define MICROPY_REPL_EMACS_KEYS     (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT      (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (0)
#define MICROPY_PY_BUILTINS_FROZENSET (0)
#define MICROPY_PY_BUILTINS_REVERSED (0)
#define MICROPY_PY_BUILTINS_SET     (0)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (0)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY_SYS_STDFILES     (1)
#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (0)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_MODULE_FROZEN_MPY   (1)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_PY_UTIME            (1)
#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_PY_URANDOM          (1)
#define MICROPY_PY_URANDOM_EXTRA_FUNCS (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (0)
#define MICROPY_ENABLE_SCHEDULER    (1)
#define MICROPY_SCHEDULER_DEPTH     (8)

#define MICROPY_MODULE_WEAK_LINKS   (1)

#if !MICROPY_PORT_MINIMAL_MAIN
#define MICROPY_PY_FRAMEBUF         (1)
#define MICROPY_PY_UJSON            (1)
#define MICROPY_PY_UBINASCII        (1)

#define MICROPY_VFS                    (1)
#define MICROPY_FATFS_ENABLE_LFN       (1)
#define MICROPY_FATFS_RPATH            (2)
#define MICROPY_FATFS_MAX_SS           (4096)
#define MICROPY_FATFS_LFN_CODE_PAGE    (437) /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_VFS_FAT                (1)
#define MICROPY_READER_VFS             (1)
#define MICROPY_FATFS_MULTI_PARTITION  (1)
#define MICROPY_FATFS_USE_LABEL        (1)

// use vfs's functions for import stat and builtin open
#define mp_import_stat mp_vfs_import_stat
#define mp_builtin_open mp_vfs_open
#define mp_builtin_open_obj mp_vfs_open_obj

#endif

#define MICROPY_PY_MACHINE          (1)

// others set in each board's mpconfigboard.h

// type definitions for the specific machine

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

// This port is intended to be 32-bit, but unfortunately, int32_t for
// different targets may be defined in different ways - either as int
// or as long. This requires different printf formatting specifiers
// to print such value. So, we avoid int32_t and use int directly.
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size

typedef long mp_off_t;

#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

// extra built in modules to add to the list of known ones
extern const struct _mp_obj_module_t machine_module;
extern const struct _mp_obj_module_t mp_module_uos;
extern const struct _mp_obj_module_t mp_module_utime;
extern const struct _mp_obj_module_t mp_module_socket;
extern const struct _mp_obj_module_t mp_module_network;
extern const struct _mp_obj_module_t mp_module_ugfx;

#if MICROPY_PY_SOCKET
#define SOCKET_BUILTIN_MODULE    { MP_ROM_QSTR(MP_QSTR_usocket), MP_ROM_PTR(&mp_module_socket) },
#define SOCKET_BUILTIN_MODULE_WEAK_LINKS    { MP_ROM_QSTR(MP_QSTR_socket), MP_ROM_PTR(&mp_module_socket) },
#else
#define SOCKET_BUILTIN_MODULE
#define SOCKET_BUILTIN_MODULE_WEAK_LINKS
#endif

#if MICROPY_PY_NETWORK
#define NETWORK_BUILTIN_MODULE              { MP_ROM_QSTR(MP_QSTR_network), MP_ROM_PTR(&mp_module_network) },
#else
#define NETWORK_BUILTIN_MODULE
#endif

#if MICROPY_HW_HAS_UGFX
#define UGFX_BUILTIN_MODULE    { MP_ROM_QSTR(MP_QSTR_ugfx), MP_ROM_PTR(&mp_module_ugfx) },
#else
#define UGFX_BUILTIN_MODULE
#endif

#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_ROM_QSTR(MP_QSTR_umachine), MP_ROM_PTR(&machine_module) }, \
    { MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&machine_module) },  \
    { MP_ROM_QSTR(MP_QSTR_json), MP_ROM_PTR(&mp_module_ujson) }, \
    { MP_ROM_QSTR(MP_QSTR_binascii), MP_ROM_PTR(&mp_module_ubinascii) }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uos), (mp_obj_t)&mp_module_uos },     \
    { MP_OBJ_NEW_QSTR(MP_QSTR_utime), (mp_obj_t)&mp_module_utime }, \
    SOCKET_BUILTIN_MODULE \
    NETWORK_BUILTIN_MODULE \
    UGFX_BUILTIN_MODULE \

#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
    { MP_ROM_QSTR(MP_QSTR_struct),      MP_ROM_PTR(&mp_module_ustruct) },   \
    { MP_ROM_QSTR(MP_QSTR_os),          MP_ROM_PTR(&mp_module_uos) },       \
    { MP_ROM_QSTR(MP_QSTR_time),        MP_ROM_PTR(&mp_module_utime) },     \
    { MP_ROM_QSTR(MP_QSTR_random),      MP_ROM_PTR(&mp_module_urandom) },   \
    SOCKET_BUILTIN_MODULE_WEAK_LINKS

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#ifndef MICROPY_HW_BOARD_NAME
#define MICROPY_HW_BOARD_NAME "minimal"
#define MICROPY_HW_MCU_NAME "ti-cortexm"
#endif

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8]; \
    mp_obj_t pinirq_callback[10]; \
    mp_obj_t pyb_config_main; \

