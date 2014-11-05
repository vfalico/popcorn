/*
 * Dummy Remote Processor control driver
 *
 * Copyright (C) 2014 Huawei Technologies
 *
 * Author: Paul Mundt <paul.mundt@huawei.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#define DRV_NAME "dummy-rproc"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/remoteproc.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>
#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include <linux/gfp.h>
#include <linux/smp.h>

extern unsigned long orig_boot_params;

char *cmdline_override="";
module_param(cmdline_override, charp, 0000);
MODULE_PARM_DESC(cmdline_override, "kernel boot paramters to pass to the second cpu");

static int dummy_rproc_start(struct rproc *rproc)
{
	unsigned long kernel_start_address = CONFIG_PHYSICAL_START;
	const struct firmware *initrd;
	void *initrd_dma;
	int apicid, apicid_1, ret = -ENOMEM;
	char *cmdline_str;
	struct boot_params *bp;
	dma_addr_t dma_bp, dma_str, dma_initrd;
	int cpu = 1;

	dev_notice(&rproc->dev, "Powering up remote processor\n");
	dev_notice(&rproc->dev, "boot params: %s\n", cmdline_override);
	dev_notice(&rproc->dev, "multikernel boot: got to multikernel_boot syscall, cpu %d, apicid %d (%x), kernel start address 0x%lx\n",
		   cpu, apic->cpu_present_to_apicid(cpu), BAD_APICID,kernel_start_address);

	apicid_1 = per_cpu(x86_bios_cpu_apicid, cpu);
	apicid = apic->cpu_present_to_apicid(cpu);

	if (apicid != BAD_APICID) {
		printk(KERN_ERR"The CPU is currently running with this kernel instance. First put it offline and then continue. apicid = %d, apicid_1 = %d\n", apicid, apicid_1);
		return -1;
	}

	dev_notice(&rproc->dev, "The CPU is not present in the current present_mask (OK to continue), apicid = %d, apicid_1 = %d\n", apicid, apicid_1);

	apicid = per_cpu(x86_bios_cpu_apicid, cpu);

	bp = dma_alloc_coherent(rproc->dev.parent, sizeof(*bp), &dma_bp, GFP_KERNEL);
	if (!bp)
		return -ENOMEM;

	memcpy(bp, __va((void *)orig_boot_params), sizeof(*bp));

	cmdline_str = dma_alloc_coherent(rproc->dev.parent, strlen(cmdline_override), &dma_str, GFP_KERNEL);
	if (!cmdline_str)
		goto free_bp;

	strcpy(cmdline_str, cmdline_override);
	bp->hdr.cmd_line_ptr = __pa(cmdline_str);

	ret = request_firmware(&initrd, "initrd", &rproc->dev);

	if (ret < 0) {
		dev_err(&rproc->dev, "request_firmware failed: %d\n", ret);
		goto free_str;
	}

	initrd_dma = dma_alloc_coherent(rproc->dev.parent, initrd->size,
					&dma_initrd, GFP_KERNEL);

	if (!initrd_dma) {
		dev_err(&rproc->dev, "failed to allocate cma for fw size %zd\n",
			initrd->size);
		ret = -ENOMEM;
		goto free_fw;
	}

	memcpy(initrd_dma, initrd->data, initrd->size);

	bp->hdr.ramdisk_image = __pa(initrd_dma);
	bp->hdr.ramdisk_size = initrd->size;
	bp->hdr.ramdisk_magic = 0; //motherfuckers

	release_firmware(initrd);

	return mkbsp_boot_cpu(apicid, cpu, kernel_start_address, bp);

free_fw:
	release_firmware(initrd);

free_str:
	dma_free_coherent(rproc->dev.parent, strlen(cmdline_override),
			  cmdline_str, dma_str);

free_bp:
	dma_free_coherent(rproc->dev.parent, sizeof(*bp), bp, dma_bp);

	return ret;
}

static int dummy_rproc_stop(struct rproc *rproc)
{
	dev_notice(&rproc->dev, "Powering off remote processor\n");
	return 0;
}

static void dummy_rproc_kick(struct rproc *rproc, int vqid)
{
	dev_notice(&rproc->dev, "Kicking virtqueue id #%d\n", vqid);
}

static struct rproc_ops dummy_rproc_ops = {
	.start		= dummy_rproc_start,
	.stop		= dummy_rproc_stop,
	.kick		= dummy_rproc_kick,
};

static int dummy_rproc_probe(struct platform_device *pdev)
{
	struct rproc *rproc;
	int ret;

	rproc = rproc_alloc(&pdev->dev, DRV_NAME, &dummy_rproc_ops, NULL, 0);
	if (!rproc)
		return -ENOMEM;

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	platform_set_drvdata(pdev, rproc);

	ret = rproc_add(rproc);
	if (unlikely(ret))
		goto err;

	return 0;

err:
	rproc_put(rproc);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int dummy_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);

	rproc_del(rproc);
	rproc_put(rproc);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver dummy_rproc_driver = {
	.probe	= dummy_rproc_probe,
	.remove	= dummy_rproc_remove,
	.driver = {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static struct platform_device *dummy_rproc_device;

static int __init dummy_rproc_init(void)
{
	int ret = 0;

	/*
	 * Only support one dummy device for testing
	 */
	if (unlikely(dummy_rproc_device))
		return -EEXIST;

	ret = platform_driver_register(&dummy_rproc_driver);
	if (unlikely(ret))
		return ret;


	dummy_rproc_device = platform_device_register_simple(DRV_NAME, 0,
							     NULL, 0);
	if (IS_ERR(dummy_rproc_device)) {
		platform_driver_unregister(&dummy_rproc_driver);
		ret = PTR_ERR(dummy_rproc_device);
	}

	return ret;
}

static void __exit dummy_rproc_exit(void)
{
	platform_device_unregister(dummy_rproc_device);
	platform_driver_unregister(&dummy_rproc_driver);
}

module_init(dummy_rproc_init);
module_exit(dummy_rproc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Dummy Remote Processor control driver");
