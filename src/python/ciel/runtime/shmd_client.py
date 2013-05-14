# Copyright (c) 2013 Antanas Uršulis <au231@cam.ac.uk>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import logging
import ciel
import ctypes

_lib = ctypes.cdll.LoadLibrary("libshmdc.so")

def init_lib(bs_path):
    ciel.log('initialising client library with %s' % bs_path, 'SHMDC', logging.DEBUG)
    _lib.ipc_init_client(bs_path)

def send_load_request(ref_name):
    ref_name = ref_name.encode('ascii', 'ignore')
    ciel.log('sending request to load %s' % ref_name, 'SHMDC', logging.DEBUG)
    shm_name = ctypes.create_string_buffer(4096) # XXX: this is actually PATH_MAX; hardcode for now
    return_code = _lib.ipc_send_load_request(ref_name, shm_name)
    ciel.log('got back %d, %s' % (return_code, shm_name.value), 'SHMDC', logging.DEBUG)
    return (return_code, shm_name.value)

def send_write_request(ref_name):
    ref_name = ref_name.encode('ascii', 'ignore')
    ciel.log('sending request to write %s' % ref_name, 'SHMDC', logging.DEBUG)
    shm_name = ctypes.create_string_buffer(4096) # XXX: this is actually PATH_MAX; hardcode for now
    return_code = _lib.ipc_send_write_request(ref_name, shm_name)
    ciel.log('got back %d, %s' % (return_code, shm_name.value), 'SHMDC', logging.DEBUG)
    return (return_code, shm_name.value)

def send_commit_request(old_name, new_name):
    old_name = old_name.encode('ascii', 'ignore')
    new_name = new_name.encode('ascii', 'ignore')
    ciel.log('sending request to commit %s as %s' % (old_name, new_name), 'SHMDC', logging.DEBUG)
    return_code = _lib.ipc_send_commit_request(old_name, new_name)
    return return_code == 0
