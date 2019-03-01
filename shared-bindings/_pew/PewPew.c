/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Radomir Dopieralski
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
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/gc.h"
#include "py/mpstate.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/util.h"
#include "PewPew.h"
#include "shared-module/_pew/PewPew.h"
#include "supervisor/shared/translate.h"


//| .. currentmodule:: _pew
//|
//| :class:`PewPew` -- LED Matrix driver
//| ====================================
//|
//| Usage::
//|
//|

//| .. class:: PewPew(buffer, rows, cols)
//|
//|     Initializes matrix scanning routines.
//|
//|
STATIC mp_obj_t pewpew_make_new(const mp_obj_type_t *type, size_t n_args,
        const mp_obj_t *pos_args, mp_map_t *kw_args) {
    mp_arg_check_num(n_args, kw_args, 4, 4, true);
    enum { ARG_buffer, ARG_rows, ARG_cols, ARG_buttons };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer, MP_ARG_OBJ | MP_ARG_REQUIRED },
        { MP_QSTR_rows, MP_ARG_OBJ | MP_ARG_REQUIRED },
        { MP_QSTR_cols, MP_ARG_OBJ | MP_ARG_REQUIRED },
        { MP_QSTR_buttons, MP_ARG_OBJ | MP_ARG_REQUIRED },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
                     allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_READ);

    size_t rows_size = 0;
    mp_obj_t *rows;
    mp_obj_get_array(args[ARG_rows].u_obj, &rows_size, &rows);

    size_t cols_size = 0;
    mp_obj_t *cols;
    mp_obj_get_array(args[ARG_cols].u_obj, &cols_size, &cols);

    if (bufinfo.len != rows_size * cols_size) {
        mp_raise_ValueError(translate(""));
    }

    for (size_t i = 0; i < rows_size; ++i) {
        if (!MP_OBJ_IS_TYPE(rows[i], &digitalio_digitalinout_type)) {
            mp_raise_TypeError(translate(""));
        }
        digitalio_digitalinout_obj_t *pin = MP_OBJ_TO_PTR(rows[i]);
        raise_error_if_deinited(
            common_hal_digitalio_digitalinout_deinited(pin));
    }

    for (size_t i = 0; i < cols_size; ++i) {
        if (!MP_OBJ_IS_TYPE(cols[i], &digitalio_digitalinout_type)) {
            mp_raise_TypeError(translate(""));
        }
        digitalio_digitalinout_obj_t *pin = MP_OBJ_TO_PTR(cols[i]);
        raise_error_if_deinited(
            common_hal_digitalio_digitalinout_deinited(pin));
    }

    if (!MP_OBJ_IS_TYPE(args[ARG_buttons].u_obj,
                        &digitalio_digitalinout_type)) {
        mp_raise_TypeError(translate(""));
    }
    digitalio_digitalinout_obj_t *buttons = MP_OBJ_TO_PTR(
            args[ARG_buttons].u_obj);
    raise_error_if_deinited(
        common_hal_digitalio_digitalinout_deinited(buttons));

    pew_obj_t *pew = MP_STATE_VM(pew_singleton);
    if (!pew) {
        pew = m_new_obj(pew_obj_t);
        pew->base.type = &pewpew_type;
        pew = gc_make_long_lived(pew);
        MP_STATE_VM(pew_singleton) = pew;
    }

    pew->buffer = bufinfo.buf;
    pew->rows = rows;
    pew->rows_size = rows_size;
    pew->cols = cols;
    pew->cols_size = cols_size;
    pew->buttons = buttons;
    pew->pressed = 0;
    pew_init();

    return MP_OBJ_FROM_PTR(pew);
}


STATIC const mp_rom_map_elem_t pewpew_locals_dict_table[] = {
};
STATIC MP_DEFINE_CONST_DICT(pewpew_locals_dict, pewpew_locals_dict_table);
const mp_obj_type_t pewpew_type = {
    { &mp_type_type },
    .name = MP_QSTR_PewPew,
    .make_new = pewpew_make_new,
    .locals_dict = (mp_obj_dict_t*)&pewpew_locals_dict,
};

