#include <sigmafs-tools/fs.h>

static int get_superblock_pos(size_t block_size)
{
	return (SIGMAFS_SUPER_OFFSET + block_size - 1) / block_size;
}

static inline void write_u16_le(uint8_t *buf, uint16_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
}

static inline void write_u32_le(uint8_t *buf, uint32_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
}

static inline void write_u64_le(uint8_t *buf, uint64_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
	buf[4] = (uint8_t)(val >> 32);
	buf[5] = (uint8_t)(val >> 40);
	buf[6] = (uint8_t)(val >> 48);
	buf[7] = (uint8_t)(val >> 56);
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
	write_u64_le(buf + i, superblock.total_blocks);
	i += 8;
	write_u64_le(buf + i, superblock.total_inodes);
	i += 8;
	write_u64_le(buf + i, superblock.free_blocks);
	i += 8;
	write_u64_le(buf + i, superblock.free_inodes);
	i += 8;
	write_u64_le(buf + i, superblock.block_bitmap_block);
	i += 8;
	write_u64_le(buf + i, superblock.inode_bitmap_block);
	i += 8;
	write_u64_le(buf + i, superblock.inode_table_block);
	i += 8;
	write_u64_le(buf + i, superblock.data_blocks_start);
	i += 8;
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

static inline uint16_t read_u16_le(const uint8_t *buf)
{
	return (uint16_t)buf[0] |
	       ((uint16_t)buf[1] << 8);
}

static inline uint32_t read_u32_le(const uint8_t *buf)
{
	return (uint32_t)buf[0] |
	       ((uint32_t)buf[1] << 8) |
	       ((uint32_t)buf[2] << 16) |
	       ((uint32_t)buf[3] << 24);
}

static inline uint64_t read_u64_le(const uint8_t *buf)
{
	return (uint64_t)buf[0] |
	       ((uint64_t)buf[1] << 8) |
	       ((uint64_t)buf[2] << 16) |
	       ((uint64_t)buf[3] << 24) |
	       ((uint64_t)buf[4] << 32) |
	       ((uint64_t)buf[5] << 40) |
	       ((uint64_t)buf[6] << 48) |
	       ((uint64_t)buf[7] << 56);
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
	superblock->total_blocks = read_u32_le(buf + i);
	i += 8;
	superblock->total_inodes = read_u32_le(buf + i);
	i += 8;
	superblock->free_blocks = read_u32_le(buf + i);
	i += 8;
	superblock->free_inodes = read_u32_le(buf + i);
	i += 8;
	superblock->block_bitmap_block = read_u32_le(buf + i);
	i += 8;
	superblock->inode_bitmap_block = read_u32_le(buf + i);
	i += 8;
	superblock->inode_table_block = read_u32_le(buf + i);
	i += 8;
	superblock->data_blocks_start = read_u32_le(buf + i);
	i += 8;
	for (size_t s_i = 0; s_i < sizeof(superblock->volume_name); s_i++, i++)
		superblock->volume_name[s_i] = buf[i];
	return 0;
}
