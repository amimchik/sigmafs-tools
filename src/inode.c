#include <sigmafs-tools/fs.h>
#include <sigmafs-tools/le.h>
#include <errno.h>
#include <stdint.h>

static uint32_t get_inode_block_size(struct filesystem *fs)
{
	if (!fs)
		return 0;
	return (fs->superblock.inode_size + fs->superblock.block_size - 1) / fs->superblock.block_size;
}

static uint32_t get_inode_block(struct filesystem *fs, uint32_t inode_id)
{
	if (!fs)
		return 0;
	int inode_block_size = get_inode_block_size(fs);
	return fs->superblock.inode_table_block + inode_id * inode_block_size;
}

/**
 * inode_alloc() -- allocates inode
 */
int inode_alloc(struct filesystem *fs)
{
	if (!fs || !fs->dev)
		return -EINVAL;
	for (uint32_t i = 0; i < fs->superblock.total_inodes; i++) {
		uint8_t bit;
		uint32_t bits_per_block = fs->superblock.block_size * 8;
		uint32_t block = i / bits_per_block + fs->superblock.inode_table_block;
		uint32_t bit_offset = i % bits_per_block;
#ifndef DEBUG_FS
		printf("DEBUG: %u, %u, %u, %u\n", bits_per_block, block, i, bit_offset);
#endif
		if (bitarr_read_bit(fs->dev, block, bit_offset, &bit))
			return -EIO;
		if (bit)
			continue;
		if (bitarr_write_bit(fs->dev, block, bit_offset, 1))
			return -EIO;
		return i;
	}
	return -ENOSPC;
}

/**
 * inode_free() -- free inode
 */
int inode_free(struct filesystem *fs, uint32_t inode_id)
{
	if (!fs || !fs->dev)
		return 1;
	uint32_t block = inode_id / (8 * get_inode_block_size(fs) * fs->superblock.block_size) + fs->superblock.inode_table_block;
	uint32_t bit_offset = inode_id % (8 * get_inode_block_size(fs) * fs->superblock.block_size);
	return bitarr_write_bit(fs->dev, block, bit_offset, 1);
}

/**
 * inode_read() -- reads inode
 */
int inode_read(struct filesystem *fs, struct inode *inode, uint32_t inode_id)
{
	if (!fs || !fs->dev || !inode)
		return 1;
	uint32_t addr = get_inode_block(fs, inode_id);
	uint32_t blk_size = get_inode_block_size(fs);
	uint8_t *buf = malloc(blk_size * fs->superblock.block_size);
	if (!buf)
		return 1;
	for (uint32_t i = 0; i < blk_size; i++) {
		if (dev_read_block(fs->dev, addr + i, buf + i * fs->superblock.block_size))
			return 1;
	}
	uint32_t i = 0;
	inode->i_mode = read_u16_le(buf);
	i += 16;
	inode->i_uid = read_u16_le(buf + i);
	i += 16;
	inode->i_gid = read_u16_le(buf + i);
	i += 16;
	inode->i_hl_cnt = read_u16_le(buf + i);
	i += 16;
	inode->f_size = read_u64_le(buf + i);
	i += 64;
	inode->atime = read_u32_le(buf + i);
	i += 32;
	inode->mtime = read_u32_le(buf + i);
	i += 32;
	inode->ctime = read_u32_le(buf + i);
	i += 32;
	for (int x = 0; x < INODE_DIRECT_BLOCKS; x++) {
		inode->direct_blocks[x] = read_u32_le(buf + i);
		i += 32;
	}
	inode->inderect_block = read_u32_le(buf + i);
	i += 32;
	inode->double_inderect_block = read_u32_le(buf + i);
	i += 32;
	inode->i_flags = read_u32_le(buf + i);
	i += 32;
	inode->i_generation = read_u32_le(buf + i);
	free(buf);
	return 0;
}

/**
 * inode_write() -- writes inode
 */
int inode_write(struct filesystem *fs, struct inode inode, uint32_t inode_id)
{
	if (!fs || !fs->dev)
		return 1;
	uint32_t addr = get_inode_block(fs, inode_id);
	uint32_t blk_size = get_inode_block_size(fs);
	uint8_t *buf = malloc(blk_size * fs->superblock.block_size);
	if (!buf)
		return 1;
	uint32_t i = 0;
	write_u16_le(buf, inode.i_mode);
	i += 16;
	write_u16_le(buf + i, inode.i_uid);
	i += 16;
	write_u16_le(buf + i, inode.i_gid);
	i += 16;
	write_u16_le(buf + i, inode.i_hl_cnt);
	i += 16;
	write_u64_le(buf + i, inode.f_size);
	i += 64;
	write_u32_le(buf + i, inode.atime);
	i += 32;
	write_u32_le(buf + i, inode.mtime);
	i += 32;
	write_u32_le(buf + i, inode.ctime);
	i += 32;
	for (int x = 0; x < INODE_DIRECT_BLOCKS; x++) {
		write_u32_le(buf + i, inode.direct_blocks[x]);
		i += 32;
	}
	write_u32_le(buf + i, inode.inderect_block);
	i += 32;
	write_u32_le(buf + i, inode.double_inderect_block);
	i += 32;
	write_u32_le(buf + i, inode.i_flags);
	i += 32;
	write_u32_le(buf + i, inode.i_generation);
	for (uint32_t i = 0; i < blk_size; i++) {
		if (dev_write_block(fs->dev, addr + i, buf + i * fs->superblock.block_size))
			return 1;
	}
	free(buf);
	return 0;
}
