/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>

#include <xdc/runtime/System.h>

#include "py/mpconfig.h"
#include "py/stackctrl.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/compile.h"
#include "py/nlr.h"
#include "py/mperrno.h"

#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"

#include "mphalport.h"
#include "storage.h"
#include "led.h"

#include "lib/utils/pyexec.h"
#include "lib/mp-readline/readline.h"

#if MICROPY_HW_USB_REPL
#include "CDCD.h"
#endif

#if MICROPY_PY_TILDA
#include <pthread.h>
#include "tilda_thread.h"
#define TILDA_TASK_STACKSIZE       2048
#define TILDA_TASK_PRIORITY        10
#endif

static char * stack_top;

static UART_Handle console;
#if MICROPY_HW_USB_REPL
static CDCD_Handle repl_cdc;
#endif

void mp_hal_stdout_tx_str(const char * str);

void SystemClock_Config(void);

fs_user_mount_t fs_user_mount_flash;

#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(void); \
        mp_handle_pending(); \
        Task_yield();        \
    } while (0);


int mp_hal_stdin_rx_chr(void)
{
    char c;
    uint32_t count;
    while (1) {
        #if MICROPY_HW_USB_REPL
        unsigned char data[1];
        /* Block while the device is NOT connected to the USB */
        c = CDCD_receiveData(repl_cdc, data, 1, 1);
        if (c != 0) {
            return (int)data[0];
        }
        #endif
        if (UART_control(console, UART_CMD_GETRXCOUNT, &count) >= 0 && count > 0) {
            UART_read(console, &c, 1);
            return (int)c;
        }
        MICROPY_EVENT_POLL_HOOK
    }
}

void mp_hal_stdout_tx_strn(const char * str, size_t len)
{
    #if MICROPY_HW_USB_REPL
    CDCD_sendData(repl_cdc, (const unsigned char *)str, len, 1);
    #endif
    UART_write(console, str, len);
}

void mp_hal_stdout_tx_str(const char * str)
{
    mp_hal_stdout_tx_strn(str, strlen(str));
}

void mp_hal_stdout_tx_strn_cooked(const char * str, size_t len)
{
    const char *last = str;
    while (len--) {
        if (*str == '\n') {
            if (str > last) {
                mp_hal_stdout_tx_strn(last, str - last);
            }
            mp_hal_stdout_tx_strn("\r\n", 2);
            ++str;
            last = str;
        } else {
            ++str;
        }
    }
    if (str > last) {
        mp_hal_stdout_tx_strn(last, str - last);
    }
}



void flash_error(int n) {
    for (int i = 0; i < n; i++) {
        led_state(TILDA_LED_RED, 1);
        led_state(TILDA_LED_GREEN, 0);
        mp_hal_delay_ms(250);
        led_state(TILDA_LED_RED, 0);
        led_state(TILDA_LED_GREEN, 1);
        mp_hal_delay_ms(250);
    }
    led_state(TILDA_LED_GREEN, 0);
}

void NORETURN __fatal_error(const char *msg) {
    for (volatile uint delay = 0; delay < 10000000; delay++) {
    }
    led_state(TILDA_LED_RED, 1);
    led_state(TILDA_LED_GREEN, 1);
    led_state(3, 1);
    led_state(4, 1);
    mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
    mp_hal_stdout_tx_strn(msg, strlen(msg));
    for (uint i = 0;;) {
        led_toggle(((i++) & 3) + 1);
        for (volatile uint delay = 0; delay < 10000000; delay++) {
        }
        if (i >= 16) {
            // to conserve power
            Task_exit();
        }
    }
}

void nlr_jump_fail(void *val) {
    printf("FATAL: uncaught exception %p\n", val);
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    __fatal_error("");
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    (void)func;
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("");
}
#endif

STATIC mp_obj_t tilda_main(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_opt, MP_ARG_INT, {.u_int = 0} }
    };

    if (MP_OBJ_IS_STR(pos_args[0])) {
        MP_STATE_PORT(tilda_config_main) = pos_args[0];

        // parse args
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
        MP_STATE_VM(mp_optimise_value) = args[0].u_int;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(tilda_main_obj, 1, tilda_main);

#if MICROPY_HW_ENABLE_STORAGE
static const char fresh_boot_py[] =
"import os, tilda\r\n\r\nos.sync()\r\nroot = os.listdir()\r\n\r\ndef app(a):\r\n    if (a in root) and (\"main.py\" in os.listdir(a)):\r\n        return a + \"/main.py\"\r\n\r\ndef file(file, remove):\r\n    try:\r\n        a = None\r\n        with open(file, 'r') as f:\r\n            a = f.read().strip()\r\n        if remove:\r\n            os.remove(file)\r\n        return app(a)\r\n    except Exception as e:\r\n        print(str(e))\r\n\r\ndef any_home():\r\n    h = [a for a in root if a.startswith(\"home\")]\r\n    return h[0] if len(h) else False\r\n\r\nif \"no_boot\" in root:\r\n    os.remove(\"no_boot\")\r\nelse:\r\n    start = None\r\n    if \"main.py\" in root:\r\n        start = \"main.py\"\r\n    start = file(\"once.txt\", True) or file(\"default_app.txt\", False) or any_home() or \"bootstrap.py\"\r\n    print(\"Booting into %s\" % start)\r\n    tilda.main(start)\r\n";

static const char fresh_bootstrap_py[] =
"\"\"\"Bootstraps the badge by downloading the base software\"\"\"\r\n\r\nimport ugfx, machine, network, json, time, usocket, os, gc\r\nfrom tilda import Buttons\r\n\r\nHOST = \"badgeserver.emfcamp.org\"\r\nwifi = network.WLAN()\r\nwifi.active(True)\r\n\r\n# Helpers\r\ndef msg(text):\r\n    ugfx.clear()\r\n    ugfx.text(5, 5, \"EMF 2018\", ugfx.BLACK)\r\n    ugfx.text(5, 30, \"TiLDA Mk4\", ugfx.BLACK)\r\n    lines = text.split(\"\\n\")\r\n    print(lines[0])\r\n    for i, line in enumerate(lines):\r\n        ugfx.text(5, 65 + i * 20, line, ugfx.BLACK)\r\n\r\ndef wifi_select():\r\n    msg(\"Please select your wifi\\nConfirm with button A\")\r\n    sl = ugfx.List(5, 110, 228, 204)\r\n    aps = {}\r\n    while not Buttons.is_pressed(Buttons.BTN_A):\r\n        for s in (wifi.scan() or []):\r\n            if s[0] not in aps:\r\n                sl.add_item(s[0])\r\n                aps[s[0]] = s\r\n        time.sleep(0.01)\r\n        ugfx.poll()\r\n    ssid = sl.selected_text()\r\n    sl.destroy()\r\n\r\n    msg(\"Wifi: %s\\nPlease enter your password\\nConfirm with button A\" % ssid)\r\n    kb = ugfx.Keyboard(0, 160, 240, 170)\r\n    e = ugfx.Textbox(5, 130, 228, 25, text=\"\")\r\n    while not Buttons.is_pressed(Buttons.BTN_A):\r\n        time.sleep(0.01)\r\n        ugfx.poll()\r\n    pw = e.text()\r\n    e.destroy()\r\n    kb.destroy()\r\n    result = {\"ssid\":ssid,\"pw\":pw}\r\n    with open(\"wifi.json\", \"wt\") as file:\r\n        file.write(json.dumps(result))\r\n        file.flush()\r\n    os.sync()\r\n    return result\r\n\r\ndef wifi_details():\r\n    try:\r\n        with open(\"wifi.json\") as f:\r\n            return json.loads(f.read())\r\n    except Exception as e:\r\n        print(str(e))\r\n        return wifi_select()\r\n\r\ndef connect():\r\n    details = wifi_details()\r\n    if 'pw' in details:\r\n        wifi.connect(details['ssid'], details['pw'])\r\n    else:\r\n        wifi.connect(details['ssid'])\r\n\r\n    wait_until = time.ticks_ms() + 10000\r\n    while not wifi.isconnected():\r\n        if (time.ticks_ms() > wait_until):\r\n            os.remove(\"wifi.json\")\r\n            raise OSError(\"Wifi timeout\");\r\n        time.sleep(0.1)\r\n\r\ndef addrinfo(host, port, retries_left = 20):\r\n    try:\r\n        return usocket.getaddrinfo(host, port)[0][4]\r\n    except OSError as e:\r\n        if (\"-15\" in str(e)) and retries_left:\r\n            # [addrinfo error -15]\r\n            # This tends to happen after startup and goes away after a while\r\n            time.sleep_ms(200)\r\n            return addrinfo(host, port, retries_left - 1)\r\n        else:\r\n            raise e\r\n\r\ndef get(path):\r\n    s = usocket.socket()\r\n    s.connect(addrinfo(HOST, 80))\r\n    body = b\"\"\r\n    status = None\r\n    try:\r\n        s.send('GET /2018/%s HTTP/1.0\\r\\nHost: %s\\r\\n\\r\\n' % (path, HOST))\r\n        state = 1\r\n        hbuf = b\"\"\r\n        clen = 9999999\r\n        headers = {}\r\n        while len(body) < clen:\r\n            buf = s.recv(1024)\r\n            if state == 1: # Status\r\n                nl = buf.find(b\"\\n\")\r\n                if nl > -1:\r\n                    hbuf += buf[:nl - 1]\r\n                    status = int(hbuf.split(b' ')[1])\r\n                    state = 2\r\n                    hbuf = b\"\";\r\n                    buf = buf[nl + 1:]\r\n                else:\r\n                    hbuf += buf\r\n\r\n            if state == 2: # Headers\r\n                hbuf += buf\r\n                nl = hbuf.find(b\"\\n\")\r\n                while nl > -1:\r\n                    if nl < 2:\r\n                        buf = hbuf[2:]\r\n                        hbuf = None\r\n                        state = 3\r\n                        clen = int(headers[\"content-length\"])\r\n                        break\r\n\r\n                    header = hbuf[:nl - 1].decode(\"utf8\").split(':', 3)\r\n                    headers[header[0].strip().lower()] = header[1].strip()\r\n                    hbuf = hbuf[nl + 1:]\r\n                    nl = hbuf.find(b\"\\n\")\r\n\r\n            if state == 3: # Content\r\n                body += buf\r\n\r\n    finally:\r\n        s.close()\r\n    if status != 200:\r\n        raise Exception(\"HTTP %d for %s\" % (status, path))\r\n    return body\r\n\r\n# os.path bits\r\ndef split(path):\r\n    if path == \"\":\r\n        return (\"\", \"\")\r\n    r = path.rsplit(\"/\", 1)\r\n    if len(r) == 1:\r\n        return (\"\", path)\r\n    head = r[0]\r\n    if not head:\r\n        head = \"/\"\r\n    return (head, r[1])\r\n\r\ndef dirname(path):\r\n    return split(path)[0]\r\n\r\ndef exists(path):\r\n    try:\r\n        os.stat(path)[0]\r\n        return True\r\n    except OSError:\r\n        return False\r\n\r\ndef makedirs(path):\r\n    sub_path = split(path)[0]\r\n    if sub_path and (not exists(sub_path)):\r\n        makedirs(sub_path)\r\n    if not exists(path):\r\n        os.mkdir(path)\r\n\r\n# Steps\r\ndef step_wifi():\r\n    while not wifi.isconnected():\r\n        msg(\"Connecting to wifi...\");\r\n        try:\r\n            connect()\r\n        except Exception as e:\r\n            print(str(e))\r\n            msg(\"Couldn't connect\\nPlease check wifi details\")\r\n            time.sleep(1)\r\n\r\ndef step_download():\r\n    msg(\"Connecting to server...\")\r\n    files = list(json.loads(get(\"bootstrap\")).keys())\r\n    for i, file in enumerate(files):\r\n        msg(\"Downloading - %d%%\\n%s\" % (100 * i // len(files), file))\r\n        makedirs(dirname(file))\r\n        with open(file, 'wb') as f:\r\n            f.write(get(\"download?repo=emfcamp/Mk4-Apps&path=%s\" % file))\r\n    os.sync()\r\n\r\ndef step_goodbye():\r\n    msg(\"All done!\\n\\nRestarting badge...\")\r\n    time.sleep(2)\r\n    machine.reset()\r\n\r\nugfx.init()\r\nmachine.Pin(machine.Pin.PWM_LCD_BLIGHT).on()\r\ntry:\r\n    step_wifi()\r\n    step_download()\r\n    step_goodbye()\r\nexcept Exception as e:\r\n    msg(\"Error\\nSomething went wrong :(\\n\\n\" + str(e))\r\n    raise e\r\n"
;

static const char fresh_wifi_json[] =
"{\"ssid\":\"emfcamp\",\"user\":\"emf\",\"pw\":\"emf\"}";


// TODO: Get usb cdc inf file
#if 0
static const char fresh_pybcdc_inf[] =
#include "genhdr/pybcdc_inf.h"
;
#endif
static const char fresh_readme_txt[] =
"This is TiLDA Mk4!\r\n"
"\r\n"
"If you want to test some code, just create a main.py in here.\r\n"
"\r\n"
"For a serial prompt:\r\n"
" - Windows: you need to go to 'Device manager', right click on the unknown device,\r\n"
"   then update the driver software, using the 'pybcdc.inf' file found on this drive.\r\n"
"   Then use a terminal program like Hyperterminal or putty.\r\n"
" - Mac OS X: use the command: screen /dev/tty.usbmodem*\r\n"
" - Linux: use the command: screen /dev/ttyACM0\r\n"
"\r\n"
"Please visit https://badge.emfcamp.org/2018 and http://micropython.org/help/ for further help.\r\n"
;

// avoid inlining to avoid stack usage within main()
MP_NOINLINE STATIC bool init_flash_fs(uint reset_mode) {
    // init the vfs object
    fs_user_mount_t *vfs_fat = &fs_user_mount_flash;
    vfs_fat->flags = 0;
    pyb_flash_init_vfs(vfs_fat);

    // try to mount the flash
    FRESULT res = f_mount(&vfs_fat->fatfs);

    if (reset_mode == 3 || res == FR_NO_FILESYSTEM) {
        // no filesystem, or asked to reset it, so create a fresh one

        // LED on to indicate creation of LFS
        led_state(TILDA_LED_GREEN, 1);

        uint8_t working_buf[_MAX_SS];
        res = f_mkfs(&vfs_fat->fatfs, FM_FAT, 0, working_buf, sizeof(working_buf));
        if (res == FR_OK) {
            // success creating fresh LFS
        } else {
            printf("PYB: can't create flash filesystem\n");
            return false;
        }

        // set label
        f_setlabel(&vfs_fat->fatfs, MICROPY_HW_FLASH_FS_LABEL);

#if 0
        // create .inf driver file
        f_open(&vfs_fat->fatfs, &fp, "/pybcdc.inf", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_pybcdc_inf, sizeof(fresh_pybcdc_inf) - 1 /* don't count null terminator */, &n);
        f_close(&fp);
#endif
        // create readme file
        FIL fp;
        f_open(&vfs_fat->fatfs, &fp, "/README.txt", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_readme_txt, sizeof(fresh_readme_txt) - 1 /* don't count null terminator */, &n);
        f_close(&fp);

        // create wifi.json file
        f_open(&vfs_fat->fatfs, &fp, "/wifi.json", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_wifi_json, sizeof(fresh_wifi_json) - 1 /* don't count null terminator */, &n);
        f_close(&fp);

        // keep LED on for at least 200ms
        mp_hal_delay_ms(200);
        led_state(TILDA_LED_GREEN, 0);
    } else if (res == FR_OK) {
        // mount sucessful
    } else {
    fail:
        printf("PYB: can't mount flash\n");
        return false;
    }

    // mount the flash device (there should be no other devices mounted at this point)
    // we allocate this structure on the heap because vfs->next is a root pointer
    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
    if (vfs == NULL) {
        goto fail;
    }
    vfs->str = "/flash";
    vfs->len = 6;
    vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
    vfs->next = NULL;
    MP_STATE_VM(vfs_mount_table) = vfs;

    // The current directory is used as the boot up directory.
    // It is set to the internal flash filesystem by default.
    MP_STATE_PORT(vfs_cur) = vfs;

    // Make sure we have a /flash/boot.py.  Create it if needed.
    FILINFO fno;
    res = f_stat(&vfs_fat->fatfs, "/boot.py", &fno);
    if (res != FR_OK) {
        // doesn't exist, create fresh file

        // LED on to indicate creation of boot.py
        led_state(TILDA_LED_GREEN, 1);

        FIL fp;
        f_open(&vfs_fat->fatfs, &fp, "/boot.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_boot_py, sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);

        // create bootstrap.py
        f_open(&vfs_fat->fatfs, &fp, "/bootstrap.py", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&fp, fresh_bootstrap_py, sizeof(fresh_bootstrap_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);

        // keep LED on for at least 200ms
        mp_hal_delay_ms(200);
        led_state(TILDA_LED_GREEN, 0);
    }

    return true;
}
#endif



STATIC uint update_reset_mode(uint reset_mode) {
    if ((GPIO_read(MICROPY_HW_MODE_GPIO)==MICROPY_HW_MODE_GPIO_STATE)) {
        // Setup inital led state:
        led_state(TILDA_LED_RED, reset_mode & 1);
        led_state(TILDA_LED_GREEN, reset_mode & 2);

        // The original method used on the pyboard is appropriate if you have 2
        // or more LEDs.
        #if defined(MICROPY_HW_LED2)
        for (uint i = 0; i < 3000; i++) {
            if (!(GPIO_read(MICROPY_HW_MODE_GPIO)==MICROPY_HW_MODE_GPIO_STATE)) {
                break;
            }
            mp_hal_delay_ms(20);
            if (i % 30 == 29) {
                if (++reset_mode > 3) {
                    reset_mode = 1;
                }
                led_state(TILDA_LED_RED, reset_mode & 1);
                led_state(TILDA_LED_GREEN, reset_mode & 2);
            }
        }
        // flash the selected reset mode
        for (uint i = 0; i < 6; i++) {
            led_state(TILDA_LED_RED, 0);
            led_state(TILDA_LED_GREEN, 0);
            mp_hal_delay_ms(50);
            led_state(TILDA_LED_RED, reset_mode & 1);
            led_state(TILDA_LED_GREEN, reset_mode & 2);
            mp_hal_delay_ms(50);
        }
        mp_hal_delay_ms(400);

        #elif defined(MICROPY_HW_LED1)

        // For boards with only a single LED, we'll flash that LED the
        // appropriate number of times, with a pause between each one
        for (uint i = 0; i < 10; i++) {
            led_state(TILDA_LED_RED, 0);
            for (uint j = 0; j < reset_mode; j++) {
                if (!(GPIO_read(MICROPY_HW_MODE_GPIO)==MICROPY_HW_MODE_GPIO_STATE)) {
                    break;
                }
                led_state(TILDA_LED_RED, 1);
                mp_hal_delay_ms(100);
                led_state(TILDA_LED_RED, 0);
                mp_hal_delay_ms(200);
            }
            mp_hal_delay_ms(400);
            if (!(GPIO_read(MICROPY_HW_MODE_GPIO)==MICROPY_HW_MODE_GPIO_STATE)) {
                break;
            }
            if (++reset_mode > 3) {
                reset_mode = 1;
            }
        }
        // Flash the selected reset mode
        for (uint i = 0; i < 2; i++) {
            for (uint j = 0; j < reset_mode; j++) {
                led_state(TILDA_LED_RED, 1);
                mp_hal_delay_ms(100);
                led_state(TILDA_LED_RED, 0);
                mp_hal_delay_ms(200);
            }
            mp_hal_delay_ms(400);
        }
        #else
        #error Need a reset mode update method
        #endif
    }
    return reset_mode;
}

int mp_main(void * heap, uint32_t heapsize, uint32_t stacksize, UART_Handle uart)
{
    int stack_dummy;
    uint32_t reset_mode;

    console = uart;
    led_init();

#if MICROPY_HW_USB_REPL
    CDCMSC_setup();
    repl_cdc = CDCD_open(0);
#endif

    #if MICROPY_HW_ENABLE_STORAGE
    storage_init();
    #endif

    #if MICROPY_PY_TILDA
    pthread_t tildaThreadHandle;
    pthread_attr_t tildaAttrs;
    struct sched_param tildaParam;

    pthread_attr_init(&tildaAttrs);
    tildaParam.sched_priority = TILDA_TASK_PRIORITY;
    pthread_attr_setschedparam(&tildaAttrs, &tildaParam);
    pthread_attr_setstacksize(&tildaAttrs, TILDA_TASK_STACKSIZE);
    pthread_create(&tildaThreadHandle, &tildaAttrs, tildaThread, NULL);
    pthread_attr_destroy(&tildaAttrs);
    #endif

soft_reset:

    #if defined(MICROPY_HW_LED2)
    led_state(TILDA_LED_RED, 0);
    led_state(TILDA_LED_GREEN, 0);
    #else
    led_state(TILDA_LED_RED, 1);
    led_state(TILDA_LED_GREEN, 0);
    #endif
    led_state(3, 0);
    led_state(4, 0);

    extern void machine_setup(void);
    machine_setup();

    #if !MICROPY_HW_USES_BOOTLOADER
    // check if user switch held to select the reset mode
    reset_mode = update_reset_mode(1);
    #endif

    // Python threading init
    #if MICROPY_PY_THREAD
    mp_thread_init();
    #endif

    // Stack limit should be less than real stack size, so we have a chance
    // to recover from limit hit.  (Limit is measured in bytes.)
    // Note: stack control relies on main thread being initialised above
    stack_top = (char *)&stack_dummy;
    mp_stack_set_top(stack_top);

    // make stack limit somewhat smaller than full stack available
    mp_stack_set_limit(stacksize - 512);

    // GC init
    gc_init(heap, (uint8_t *)heap + heapsize);

    #if MICROPY_ENABLE_PYSTACK
    static mp_obj_t pystack[384];
    mp_pystack_init(pystack, &pystack[384]);
    #endif

    // MicroPython init
    mp_init();
    mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_)); // current dir (or base dir of the script)
    mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

    // Initialise low-level sub-systems.  Here we need to very basic things like
    // zeroing out memory and resetting any of the sub-systems.  Following this
    // we can run Python scripts (eg boot.py), but anything that is configurable
    // by boot.py must be set after boot.py is run.

    #if MICROPY_PY_TILDA
    tilda_init0();
    #endif

    // Initialise the local flash filesystem.
    // Create it if needed, mount in on /flash, and set it as current dir.
    bool mounted_flash = false;
    #if MICROPY_HW_ENABLE_STORAGE
    mounted_flash = init_flash_fs(reset_mode);
    #endif

    bool mounted_sdcard = false;
    #if MICROPY_HW_HAS_SDCARD
    // if an SD card is present then mount it on /sd/
    if (sdcard_is_present()) {
        // if there is a file in the flash called "SKIPSD", then we don't mount the SD card
        if (!mounted_flash || f_stat(&fs_user_mount_flash.fatfs, "/SKIPSD", NULL) != FR_OK) {
            mounted_sdcard = init_sdcard_fs();
        }
    }
    #endif

    #if MICROPY_HW_ENABLE_USB
    // if the SD card isn't used as the USB MSC medium then use the internal flash
    if (pyb_usb_storage_medium == PYB_USB_STORAGE_MEDIUM_NONE) {
        pyb_usb_storage_medium = PYB_USB_STORAGE_MEDIUM_FLASH;
    }
    #endif

    // set sys.path based on mounted filesystems (/sd is first so it can override /flash)
    if (mounted_sdcard) {
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd_slash_lib));
    }
    if (mounted_flash) {
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_lib));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_flash_slash_upip));
    }

    // reset config variables; they should be set by boot.py
    MP_STATE_PORT(tilda_config_main) = MP_OBJ_NULL;

    // run boot.py, if it exists
    // TODO perhaps have pyb.reboot([bootpy]) function to soft-reboot and execute custom boot.py
    if (reset_mode == 1 || reset_mode == 3) {
        const char *boot_py = "boot.py";
        mp_import_stat_t stat = mp_import_stat(boot_py);
        if (stat == MP_IMPORT_STAT_FILE) {
            int ret = pyexec_file(boot_py);
            if (ret & PYEXEC_FORCED_EXIT) {
                goto soft_reset_exit;
            }
            if (!ret) {
                flash_error(4);
            }
        }
    }

    // turn boot-up LEDs off
    #if !defined(MICROPY_HW_LED2)
    // If there is only one LED on the board then it's used to signal boot-up
    // and so we turn it off here.  Otherwise LED(1) is used to indicate dirty
    // flash cache and so we shouldn't change its state.
    led_state(TILDA_LED_RED, 0);
    #endif
    led_state(TILDA_LED_GREEN, 0);
    led_state(3, 0);
    led_state(4, 0);



    // At this point everything is fully configured and initialised.

    // Run the main script from the current directory.
    if ((reset_mode == 1 || reset_mode == 3) && pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        const char *main_py;
        if (MP_STATE_PORT(tilda_config_main) == MP_OBJ_NULL) {
            main_py = "main.py";
        } else {
            main_py = mp_obj_str_get_str(MP_STATE_PORT(tilda_config_main));
        }
        mp_import_stat_t stat = mp_import_stat(main_py);
        if (stat == MP_IMPORT_STAT_FILE) {
            int ret = pyexec_file(main_py);
            if (ret & PYEXEC_FORCED_EXIT) {
                goto soft_reset_exit;
            }
            if (!ret) {
                flash_error(3);
            }
        }
    }

    // Main script is finished, so now go into REPL mode.
    // The REPL mode can change, or it can request a soft reset.
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

soft_reset_exit:

    // soft reset

    #if MICROPY_HW_ENABLE_STORAGE
    printf("PYB: sync filesystems\n");
    storage_flush();
    #endif

    printf("PYB: soft reboot\n");

    extern void machine_teardown(void);
    machine_teardown();

    mp_deinit();

    gc_sweep_all();

    goto soft_reset;
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
