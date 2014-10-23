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

#include <linux/remoteproc.h>
#include <linux/virtio_ids.h>

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
		1,			/* version */
		2,			/* we have 2 entries - mem and rpmsg */
		{ 0, 0 },		/* reserved - must be 0 */
	},
	.offset = {			/* offsets to our resource entries */
		offsetof(struct dummy_rproc_resourcetable, rsc_hdr_mem),
		offsetof(struct dummy_rproc_resourcetable, rsc_hdr_vdev),
	},
	.rsc_hdr_mem = {
		RSC_CARVEOUT,		/* mem resource */
	},
	.rsc_mem = {
		(u32) FW_RSC_ADDR_ANY,	/* we don't care about the dev address */
		0x40000000,		/* here be physicall address */
		0x51f0000,		/* size please */
		0,			/* TODO flags */
		0,			/* reserved - 0 */
		"dummy-rproc-mem",
	},
	.rsc_hdr_vdev = {
		RSC_VDEV,		/* vdev resource */
	},
	.rsc_vdev = {
		VIRTIO_ID_RPMSG,	/* found in virtio_ids.h */
		0xC001DEA1,		/* magic number for IPC */
		0,			/* features - none (??) */
		0,			/* negotiated features - blank */
		0,			/* config len - none (??) */
		0,			/* status - updated by bsp */
		2,			/* we have 2 rings */
		{ 0, 0},		/* reserved */
	},
	.rsc_ring0 = {
		0,			/* we don't (??) care about the dev addr */
		PAGE_SIZE,		/* alignment */
		16,			/* number of buffers */
		0xC001DEA2,		/* magic number for IPC */
		0,			/* reserved - 0 */
	},
	.rsc_ring1 = {
		0,			/* we don't (??) care about the dev addr */
		PAGE_SIZE,		/* alignment */
		16,			/* number of buffers */
		0xC001DEA3,		/* magic number for IPC */
		0,			/* reserved - 0 */
	},
};
