/*
 * Boot parameters and other support stuff for MKLinux
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2012
 */

#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <asm/bootparam.h>

extern struct boot_params boot_params;

int mklinux_boot = 0;
EXPORT_SYMBOL(mklinux_boot);

static int __init setup_mklinux(char *arg)
{
        mklinux_boot = 1;
        return 0;
}
early_param("mklinux", setup_mklinux);


/* We're going to put our syscall here, since we need to pass in
   two arguments but the reboot syscall only takes one */

SYSCALL_DEFINE0(get_boot_params_addr)
{
	printk("POPCORN: syscall to return phys addr of boot_params structure\n");
	return __pa(&boot_params);
}
