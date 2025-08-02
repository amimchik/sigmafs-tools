#include <sigmafs-tools/fs.h>
#include <sigmafs-tools/le.h>

static int get_superblock_pos(size_t block_size)
{
	return (SIGMAFS_SUPER_OFFSET + block_size - 1) / block_size;
}

int superblock_write(struct dev *dev, struct superblock superblock)
{
	if (!dev)
		return 1;
	uint8_t buf[SIGMAFS_SUPER_SIZE];
	size_t i = 0;
	write_u32_le(buf, superblock.magic);
	i += 4;
	write_u32_le(buf + i, superblock.version);
	i += 4;
	write_u32_le(buf + i, superblock.block_size);
	i += 4;
	write_u32_le(buf + i, superblock.inode_size);
	i += 4;
	write_u32_le(buf + i, superblock.dev_block_size);
	i += 4;
	write_u32_le(buf + i, superblock.total_blocks);
	i += 4;
	write_u32_le(buf + i, superblock.total_inodes);
	i += 4;
	write_u32_le(buf + i, superblock.free_blocks);
	i += 4;
	write_u32_le(buf + i, superblock.free_inodes);
	i += 4;
	write_u32_le(buf + i, superblock.block_bitmap_block);
	i += 4;
	write_u32_le(buf + i, superblock.inode_bitmap_block);
	i += 4;
	write_u32_le(buf + i, superblock.inode_table_block);
	i += 4;
	write_u32_le(buf + i, superblock.data_blocks_start);
	i += 4;
	for (size_t s_i = 0; s_i < sizeof(superblock.volume_name); i++, s_i++)
		buf[i] = superblock.volume_name[s_i];
	while (i < SIGMAFS_SUPER_SIZE)
		buf[i++] = 0;
	for (size_t blk = 0; blk * dev->block_size < SIGMAFS_SUPER_SIZE; blk++) {
		if (dev_write_block(dev,
		                    get_superblock_pos(dev->block_size) + blk,
		                    buf + blk * dev->block_size))
			return 1;
	}
	return 0;
}

int superblock_read(struct dev *dev, struct superblock *superblock)
{
	if (!dev || !superblock)
		return 1;
	uint8_t buf[SIGMAFS_SUPER_SIZE];
	for (int blk = 0; blk * dev->block_size < SIGMAFS_SUPER_SIZE; blk++) {
		if (dev_read_block(dev,
		                   blk + get_superblock_pos(dev->block_size), buf + blk * dev->block_size))
			return 1;
	}
	size_t i = 0;
	superblock->magic = read_u32_le(buf);
	i += 4;
	superblock->version = read_u32_le(buf + i);
	i += 4;
	superblock->block_size = read_u32_le(buf + i);
	i += 4;
	superblock->inode_size = read_u32_le(buf + i);
	i += 4;
	superblock->dev_block_size = read_u32_le(buf + i);
	i += 4;
	superblock->total_blocks = read_u32_le(buf + i);
	i += 4;
	superblock->total_inodes = read_u32_le(buf + i);
	i += 4;
	superblock->free_blocks = read_u32_le(buf + i);
	i += 4;
	superblock->free_inodes = read_u32_le(buf + i);
	i += 4;
	superblock->block_bitmap_block = read_u32_le(buf + i);
	i += 4;
	superblock->inode_bitmap_block = read_u32_le(buf + i);
	i += 4;
	superblock->inode_table_block = read_u32_le(buf + i);
	i += 4;
	superblock->data_blocks_start = read_u32_le(buf + i);
	i += 4;
	for (size_t s_i = 0; s_i < sizeof(superblock->volume_name); s_i++, i++)
		superblock->volume_name[s_i] = buf[i];
	return 0;
}
