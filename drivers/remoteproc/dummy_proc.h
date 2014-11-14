/*
 * Dummy Remote Processor resource table
 *
 * Copyright (C) 2014 Huawei Technologies
 *
 * Author: Veaceslav Falico <veaceslav.falico@huawei.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef DUMMY_PROC_H
#define DUMMY_PROC_H

#include "remoteproc_internal.h"

#define DRV_NAME "dummy-rproc"

#define VMLINUX_FIRMWARE_SIZE			80000000

int dummy_lproc_set_bsp_callback(void (*fn)(void *), void *data);
int dummy_lproc_boot_remote_cpu(int boot_cpu, unsigned long start_addr, void *boot_params);

#endif /* DUMMY_PROC_H */
