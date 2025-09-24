#include <sigmafs-tools/fs.h>
#include <stdio.h>
#include <errno.h>

/**
 * data_block_alloc() -- allocates data block
 * @fs:			filesystem
 */
int data_block_alloc(struct filesystem *fs)
{
	if (!fs || !fs->dev)
		return -EINVAL;
	for (uint32_t i = 0; i < fs->superblock.total_blocks; i++) {
		uint8_t bit;
		uint32_t block = i / (8 * fs->superblock.block_size) + fs->superblock.block_bitmap_block;
		uint32_t bit_offset = i % (8 * fs->superblock.block_size);
		int err = bitarr_read_bit(fs->dev, block, bit_offset, &bit);
		if (err)
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
 * data_block_free() -- free data block
 * @fs:			filesystem
 * @block_id:		id of data block
 */
int data_block_free(struct filesystem *fs, uint32_t block_id)
{
	if (!fs || !fs->dev)
		return -EINVAL;
	uint32_t block = block_id / (8 * fs->superblock.block_size) + fs->superblock.block_bitmap_block;
	uint32_t bit_offset = block_id % (8 * fs->superblock.block_size);
	int ret =  bitarr_write_bit(fs->dev, block, bit_offset, 1);
	if (ret < 0)
		return -EIO;
	return 0;
}

/**
 * data_block_write() -- write data block
 * @fs:			filesystem
 * @block_id:		id of data block
 * @buf:		buffer
 */
int data_block_write(struct filesystem *fs, uint32_t block_id, uint8_t *buf)
{
#ifdef DEBUG_FS
	printf("writing to %u, value str is %s\n", block_id, buf);
#endif
	if (!fs || !fs->dev || !buf)
		return -EINVAL;
	int ret = dev_write_block(fs->dev, fs->superblock.data_blocks_start + block_id, buf);
	if (ret < 0)
		return -EIO;
	return 0;
}

/**
 * data_block_write() -- write data block
 * @fs:			filesystem
 * @block_id:		id of data block
 * @buf:		buffer
 */
int data_block_read(struct filesystem *fs, uint32_t block_id, uint8_t *buf)
{
#ifdef DEBUG_FS
	printf("reading from %u\n", block_id);
#endif
	if (!fs || !fs->dev || !buf)
		return -EINVAL;
	int ret = dev_read_block(fs->dev, fs->superblock.data_blocks_start + block_id, buf);
	if (ret < 0)
		return -EIO;
	return 0;
}
