#include "string.h"

#include "uffs/uffs_device.h"
#include "uffs/uffs_flash.h"
#include "uffs/uffs_mtb.h"
#include "uffs/uffs_mem.h"


#define PFX "ndrv:"

struct nor_chip{
	UBOOL inited;
};

#define PAR_START	4
#define PAR_BLOCKS	4

static int nor_read_page(uffs_Device *dev, u32 block, u32 page, u8 *data, int data_len, u8 *ecc, u8 *spare, int spare_len)
{
	return U_FAIL;
}

static int nor_write_page(uffs_Device *dev, u32 block, u32 page, const u8 *data, int data_len, const u8 *spare, int spare_len)
{
	return U_FAIL;
}

static int nor_erase_block(uffs_Device *dev, u32 block)
{
	return U_FAIL;
}

static int nor_init_flash(uffs_Device *dev)
{
	return U_FAIL;
}

static int nor_release_flash(uffs_Device *dev)
{
	return U_FAIL;
}

static uffs_FlashOps g_nor_ops = {
	nor_init_flash,
	nor_release_flash,
	nor_read_page,
	NULL,	// ReadPageWithLayout
	nor_write_page,
	NULL,	// WritePageWithLayout
	NULL,	// IsBadBlock, let UFFS take care of it
	NULL,	// MarkBadBlock(), let UFFS take care of it
	nor_erase_block,
};

#define TOTAL_BLOCKS	63
#define BLOCK_SIZE	64*1024
#define PAGE_SIZE	512
#define PAGES_PER_BLOCK	(BLOCK_SIZE/PAGE_SIZE)
#define PAGE_SPARE_SIZE	16
#define PAGE_DATA_SIZE (PAGE_SIZE-PAGE_SPARE_SIZE)
#define BLOCK_DATA_SIZE	(PAGE_DATA_SIZE * PAGES_PER_BLOCK)

static int static_buffer[UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, PAR_BLOCKS) / sizeof(int)];

static struct nor_chip g_nor_chip = {0};
static struct uffs_StorageAttrSt g_flash_storage = {0};

static uffs_Device demo_device = {0};

static uffs_MountTable demo_mount_table[] = {
	{ &demo_device, PAR_START, PAR_START + PAR_BLOCKS - 1, "/" },
	{ NULL, 0, 0, NULL}
};

static void setup_flash_storage(struct uffs_StorageAttrSt *attr)
{
	memset(attr, 0, sizeof(struct uffs_StorageAttrSt));

	attr->total_blocks = TOTAL_BLOCKS;
	attr->page_data_size = PAGE_DATA_SIZE;
	attr->pages_per_block = PAGES_PER_BLOCK;
	attr->block_status_offs = 4;
	attr->ecc_opt = UFFS_ECC_SOFT;
	attr->layout_opt = UFFS_LAYOUT_UFFS;
}

static URET InitDevice(uffs_Device *dev)
{
	dev->attr = &g_flash_storage;
	dev->attr->_private = (void *)&g_nor_chip;
	dev->ops = &g_nor_ops;

	return U_SUCC;
}

static URET ReleaseDevice(uffs_Device *dev)
{
	return U_SUCC;
}

static int init_filesystem(void)
{
	uffs_MountTable *mtbl = &(demo_mount_table[0]);

	setup_flash_storage(&g_flash_storage);

	uffs_MemSetupStaticAllocator(&demo_device.mem, static_buffer, sizeof(static_buffer));

	while(mtbl->dev) {
		mtbl->dev->Init = InitDevice;
		mtbl->dev->Release = ReleaseDevice;
		uffs_RegisterMountTable(mtbl);
		mtbl++;
	}

	return uffs_InitMountTable() == U_SUCC ? 0 : -1;
}

int test_uffs()
{
	init_filesystem();
	uffs_ReleaseMountTable();
	return 0;
}
