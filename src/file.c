#include <sigmafs-tools/fs.h>
#include <sigmafs-tools/le.h>
#include <stdlib.h>
#include <string.h>

static inline int64_t read_inderect(struct filesystem *fs, uint32_t block, uint32_t index)
{
	if (!fs || !fs->dev)
		return -1;
	uint8_t *buf = malloc(fs->superblock.block_size);
	if (!buf)
		return -1;
	if (data_block_read(fs, block, buf))
		return -1;
	uint32_t result = read_u32_le(buf + index * 4);
	free(buf);
	return result;
}

static inline int write_inderect(struct filesystem *fs, uint32_t block, uint32_t index, uint32_t value)
{
	if (!fs || !fs->dev)
		return -1;
	uint8_t *buf = malloc(fs->superblock.block_size);
	if (!buf)
		return -1;
	if (data_block_read(fs, block, buf))
		return -1;
	write_u32_le(buf + index * 4, value);
	if (data_block_write(fs, block, buf))
		return -1;
	free(buf);
	return 0;
}

static int64_t get_block_id(struct filesystem *fs, struct inode inode, uint32_t block_num)
{
	uint32_t ptrs_per_block = fs->superblock.block_size / 4;
	if (block_num < INODE_DIRECT_BLOCKS) {
		return inode.direct_blocks[block_num];
	} else if (block_num - INODE_DIRECT_BLOCKS < ptrs_per_block) {
		int64_t result = read_inderect(fs, inode.inderect_block, block_num - INODE_DIRECT_BLOCKS);
		if (result < 0)
			return -1;
		return (uint32_t)result;
	} else if (block_num - INODE_DIRECT_BLOCKS - ptrs_per_block < ptrs_per_block * ptrs_per_block) {
		uint32_t block_num_subed = block_num - INODE_DIRECT_BLOCKS - ptrs_per_block;
		uint32_t inderect_blk_index = block_num_subed / ptrs_per_block;
		uint32_t direct_blk_index = block_num_subed % ptrs_per_block;
		int64_t inderect_blk_id = read_inderect(fs, inode.double_inderect_block, inderect_blk_index);
		if (inderect_blk_id < 0)
			return -1;
		int64_t direct_blk_id = read_inderect(fs, inderect_blk_id, direct_blk_index);
		if (inderect_blk_id < 0)
			return -1;
		return direct_blk_id;
	} else {
		return -2;
	}
}

static int64_t alloc_back(struct filesystem *fs, struct inode *inode, uint32_t block_num)
{
#ifdef DEBUG_FS
	printf("CALLED allocback with %d\n", block_num);
#endif
	uint32_t ptrs_per_block = fs->superblock.block_size / 4;
	if (block_num < INODE_DIRECT_BLOCKS) {
		int allocated = data_block_alloc(fs);
		if (allocated < 0)
			return -1;
		inode->direct_blocks[block_num] = (uint32_t)allocated;
		return allocated;
	} else if (block_num - INODE_DIRECT_BLOCKS < ptrs_per_block) {
		uint32_t block_num_subed = block_num - INODE_DIRECT_BLOCKS;
		if (block_num_subed == 0) {
			int allocated = data_block_alloc(fs);
			if (allocated < 0)
				return -1;
			inode->inderect_block = allocated;
		}
		int allocated = data_block_alloc(fs);
		if (allocated < 0)
			return -1;
		if (write_inderect(fs, inode->inderect_block, block_num_subed, allocated))
			return -1;
		return (uint32_t)allocated;
	} else if (block_num - INODE_DIRECT_BLOCKS - ptrs_per_block < ptrs_per_block * ptrs_per_block) {
		uint32_t block_num_subed = block_num - INODE_DIRECT_BLOCKS - ptrs_per_block;
		uint32_t inderect_blk_index = block_num_subed / ptrs_per_block;
		uint32_t direct_blk_index = block_num_subed % ptrs_per_block;
		if (block_num_subed == 0) {
			int allocated = data_block_alloc(fs);
			if (allocated < 0)
				return -1;
			inode->double_inderect_block = allocated;
		}
		if (direct_blk_index == 0) {
			int allocated = data_block_alloc(fs);
			if (allocated < 0)
				return -1;
			write_inderect(fs, inode->double_inderect_block, inderect_blk_index, allocated);
		}
		int allocated = data_block_alloc(fs);
		if (allocated < 0)
			return -1;
		int64_t inderect_blk_id = read_inderect(fs, inode->double_inderect_block, inderect_blk_index);
		if (inderect_blk_id < 0)
			return -1;
		if (write_inderect(fs, (uint32_t)inderect_blk_id, direct_blk_index, allocated))
			return -1;
		return allocated;
	} else {
		return -2;
	}
}

/**
 * read_file() -- reads content of file
 * @fs:			filesystem
 * @inode_id:		id of file inode
 * @out_buf:		buffer
 */
int read_file(struct filesystem *fs, uint32_t inode_id, uint8_t *out_buf)
{
	if (!fs || !fs->dev || !out_buf)
		return 1;
	struct inode inode;
	if (inode_read(fs, &inode, inode_id))
		return 1;
	uint8_t *buf = malloc((inode.f_size + fs->superblock.block_size - 1) / fs->superblock.block_size * fs->superblock.block_size);
	if (!buf)
		return 1;
	for (uint32_t i = 0; i < (inode.f_size + fs->superblock.block_size - 1) / fs->superblock.block_size; i++) {
		int64_t id = get_block_id(fs, inode, i);
		if (id < 0)
			return 1;
		data_block_read(fs, id, buf + i * fs->superblock.block_size);
	}
	memcpy(out_buf, buf, inode.f_size);
	free(buf);
	return 0;
}

/**
 * free_file_content() -- free file content
 * @fs:			filesystem
 * @inode_id:		id of file
 */
int free_file_content(struct filesystem *fs, uint32_t inode_id)
{
	if (!fs || !fs->dev)
		return 1;
	struct inode inode;
	if (inode_read(fs, &inode, inode_id))
		return 1;
	for (uint32_t i = 0; i < (inode.f_size + fs->superblock.block_size - 1) / fs->superblock.block_size; i++) {
		int64_t id = get_block_id(fs, inode, i);
		if (id < 0)
			return 1;
		if (data_block_free(fs, id))
			return 2;
	}
	return 0;
}

/**
 * write_file() -- writes data to file
 * @fs:			filesystem
 * @inode_id:		inode id of file
 * @in_buf:		buffer
 * @size:		buffer size
 */
int write_file(struct filesystem *fs, uint32_t inode_id, uint8_t *in_buf, uint64_t size)
{
	if (!fs || !fs->dev || !in_buf)
		return 1;
	struct inode inode;
	if (inode_read(fs, &inode, inode_id))
		return 2;
	if (free_file_content(fs, inode_id))
		return 3;
	inode.f_size = size;
	uint8_t *buf = malloc((size + fs->superblock.block_size - 1) / fs->superblock.block_size * fs->superblock.block_size);
	if (!buf) {
		printf("allocerr, tried %lu\n", (size + fs->superblock.block_size - 1) / fs->superblock.block_size * fs->superblock.block_size);
		return 4;
	}
	memcpy(buf, in_buf, size);
	for (uint32_t i = 0; i < (size + fs->superblock.block_size - 1) / fs->superblock.block_size; i++) {
		int64_t allocated = alloc_back(fs, &inode, i);
#ifdef DEBUG_FS
		printf("allocback returned: %ld\n", allocated);
#endif
		if (allocated < 0)
			return 1;
		if (data_block_write(fs, allocated, buf + i * fs->superblock.block_size))
			return 1;
	}
	if (inode_write(fs, inode, inode_id))
		return 5;
	return 0;
}
