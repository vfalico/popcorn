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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/remoteproc.h>
#include <linux/virtio_ids.h>

#define VMLINUX_FIRMWARE_SIZE		0x6f00000

struct dummy_rproc_resourcetable {
	struct resource_table		main_hdr;
	u32				offset[2];
	/* We'd need some physical mem */
	struct fw_rsc_hdr		rsc_hdr_mem;
	struct fw_rsc_carveout		rsc_mem;
	/* And some rpmsg rings */
	struct fw_rsc_hdr		rsc_hdr_vdev;
	struct fw_rsc_vdev		rsc_vdev;
	struct fw_rsc_vdev_vring	rsc_ring0;
	struct fw_rsc_vdev_vring	rsc_ring1;
};

struct dummy_rproc_resourcetable dummy_remoteproc_resourcetable
	__attribute__((section(".resource_table"), aligned(PAGE_SIZE))) =
{
	.main_hdr = {
		.ver =		1,			/* version */
		.num =		2,			/* we have 2 entries - mem and rpmsg */
		.reserved =	{ 0, 0 },		/* reserved - must be 0 */
	},
	.offset = {					/* offsets to our resource entries */
		offsetof(struct dummy_rproc_resourcetable, rsc_hdr_mem),
		offsetof(struct dummy_rproc_resourcetable, rsc_hdr_vdev),
	},
	.rsc_hdr_mem = {
		.type =		RSC_CARVEOUT,		/* mem resource */
	},
	.rsc_mem = {
		.da =		CONFIG_PHYSICAL_START,	/* we don't care about the dev address */
		.pa =		0,			/* here be physicall address */
		.len =		VMLINUX_FIRMWARE_SIZE,	/* size please */
		.flags =	0,			/* TODO flags */
		.reserved =	0,			/* reserved - 0 */
		.name =		"dummy-rproc-mem",
	},
	.rsc_hdr_vdev = {
		.type =		RSC_VDEV,		/* vdev resource */
	},
	.rsc_vdev = {
		.id =		VIRTIO_ID_RPMSG,	/* found in virtio_ids.h */
		.notifyid =	0,			/* magic number for IPC */
		.dfeatures =	0,			/* features - none (??) */
		.gfeatures =	0,			/* negotiated features - blank */
		.config_len =	0,			/* config len - none (??) */
		.status =	0,			/* status - updated by bsp */
		.num_of_vrings=	2,			/* we have 2 rings */
		.reserved =	{ 0, 0},		/* reserved */
	},
	.rsc_ring0 = {
		.da =		0,			/* we don't (??) care about the da */
		.align =	PAGE_SIZE,		/* alignment */
		.num =		512,			/* number of buffers */
		.notifyid =	0,			/* magic number for IPC */
		.reserved =	0,			/* reserved - 0 */
	},
	.rsc_ring1 = {
		.da =		0,			/* we don't (??) care about the da */
		.align =	PAGE_SIZE,		/* alignment */
		.num =		512,			/* number of buffers */
		.notifyid =	0,			/* magic number for IPC */
		.reserved =	0,			/* reserved - 0 */
	},
};

struct dummy_rproc_resourcetable *lproc = NULL;
u64 lproc_pa = 0;

static int __init dummy_lproc_init(void)
{
	if (!lproc_pa)
		return 0;

	lproc = ioremap_cache(lproc_pa, sizeof(struct rproc));
	if (lproc)
		printk(KERN_INFO "lproc 0x%p version %d vrings %d vring1.notifyid %d\n", lproc, lproc->main_hdr.ver, lproc->main_hdr.num, lproc->rsc_ring1.notifyid);
	else {
		printk(KERN_ERR "ioremap failed for pa 0x%p\n", lproc_pa);
		return -EFAULT;
	}

	return 0;

}
late_initcall(dummy_lproc_init);

static int __init dummy_lproc_parse_addr(char *p)
{
	lproc_pa = memparse(p, &p);
	if (!lproc_pa)
		printk(KERN_ERR "lproc: couldn't parse address for rproc\n");

	return 0;
}
__setup("rproc_rsc_tbl=", dummy_lproc_parse_addr);
