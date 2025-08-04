#include <sigmafs-tools/fs.h>

static inline int get_first_free_block(uint32_t block_size)
{
	return (SIGMAFS_SUPER_SIZE + SIGMAFS_SUPER_OFFSET + block_size - 1) / block_size;
}

static uint32_t get_inode_block_size(struct filesystem *fs)
{
	if (!fs)
		return 0;
	return (fs->superblock.inode_size + fs->superblock.block_size - 1) / fs->superblock.block_size;
}

int make_fs(struct dev *dev, struct filesystem *output, struct superblock initial_super)
{
	if (!dev)
		return 1;
	if (dev->blocks_count * dev->block_size <= (size_t)get_first_free_block(dev->block_size))
		return 2;
	uint32_t first_free_block = get_first_free_block(dev->block_size);
	output->dev = dev;
	output->superblock = initial_super;
	output->superblock.magic = SIGMAFS_MAGIC;
	output->superblock.version = FS_VERSION(SIGMAFS_VERSION_MAJOR, SIGMAFS_VERSION_MINOR);
	output->superblock.block_size = dev->block_size;
	output->superblock.inode_size = SIGMAFS_INODE_SIZE;
	uint32_t free_blocks = dev->blocks_count - first_free_block;
	uint32_t bmap_coef = dev->block_size * 8;
	uint32_t inode_bmap_coef = bmap_coef * get_inode_block_size(output);
	uint32_t inode_reserved_blocks = free_blocks / 2;
	uint32_t inode_bmap_size = (inode_reserved_blocks + inode_bmap_coef) / (inode_bmap_coef + 1);
	inode_reserved_blocks = inode_reserved_blocks / (inode_bmap_size + 1) * (inode_bmap_size + 1);
	uint32_t inode_count = (inode_reserved_blocks - inode_bmap_size) / get_inode_block_size(output);
	uint32_t dblock_reserved_blocks = free_blocks - inode_reserved_blocks;
	uint32_t dblock_bmap_size = (dblock_reserved_blocks + bmap_coef) / (bmap_coef + 1);
	uint32_t dblock_count = dblock_reserved_blocks - dblock_bmap_size;
	output->superblock.dev_block_size = dev->block_size;
	output->superblock.total_blocks = dblock_count;
	output->superblock.total_inodes = inode_count;
	output->superblock.free_blocks = dblock_count;
	output->superblock.free_inodes = inode_count;
	output->superblock.inode_bitmap_block = first_free_block;
	output->superblock.inode_table_block = first_free_block + inode_bmap_size;
	output->superblock.block_bitmap_block = output->superblock.inode_table_block + inode_count * get_inode_block_size(output);
	output->superblock.data_blocks_start = output->superblock.block_bitmap_block + dblock_bmap_size;
	superblock_write(dev, output->superblock);
	{
		uint8_t *buf = malloc(output->superblock.block_size);
		if (!buf)
			return 1;
		for (uint32_t i = 0; i < output->superblock.block_size; i++)
			buf[i] = 0;
		for (uint32_t i = 0; i < output->superblock.inode_table_block - output->superblock.inode_bitmap_block; i++) {
			dev_write_block(output->dev, output->superblock.inode_bitmap_block + i, buf);
		}
		for (uint32_t i = 0; i < output->superblock.data_blocks_start - output->superblock.block_bitmap_block; i++) {
			dev_write_block(output->dev, output->superblock.block_bitmap_block + i, buf);
		}
	}
	struct inode root_dir;
	root_dir.i_mode = imode_create(IMODE_IFDIR, 0755);
	root_dir.f_size = 0;
	inode_write(output, root_dir, 0);
	struct directory_entry root_content[2];
	root_content[0].name_len = 1;
	root_content[0].name[0] = '.';
	root_content[0].inode_id = 0;
	root_content[1].name_len = 2;
	root_content[1].name[0] = '.';
	root_content[1].name[1] = '.';
	root_content[1].inode_id = 0;
	int error = set_dir_content(output, 0, root_content, 2);
	if (error) {
		fprintf(stderr, "ERR: %d\n", error);
		return 4;
	}
	return 0;
}
