#ifndef _FS_H
#define _FS_H

#define SIGMAFS_MAGIC 0x00ff1233
#define SIGMAFS_SUPER_SIZE 1024
#define SIGMAFS_SUPER_OFFSET 1024
#define SIGMAFS_INODE_SIZE 512
#define SIGMAFS_VERSION_MAJOR 1
#define SIGMAFS_VERSION_MINOR 1
#define SIGMAFS_ROOT_INODE_ID 0

#define FS_VERSION(major, minor) (((uint32_t)(major) << 16) | (uint32_t)(minor))
#define FS_GET_MAJOR(version) ((uint16_t)((version) >> 16))
#define FS_GET_MINOR(version) ((uint16_t)((version) & 0xFFFF))

#define INODE_DIRECT_BLOCKS 8

#include <sigmafs-tools/dev.h>
#include <sigmafs-tools/imode.h>
#include <stdint.h>
#include <stdlib.h>

struct filesystem;
struct superblock;
struct inode;

struct superblock {
	uint32_t magic;
	uint32_t version;

	uint32_t block_size;
	uint32_t inode_size;

	uint32_t dev_block_size;

	uint32_t total_blocks;
	uint32_t total_inodes;
	uint32_t free_blocks;
	uint32_t free_inodes;

	uint32_t block_bitmap_block;
	uint32_t inode_bitmap_block;
	uint32_t inode_table_block;
	uint32_t data_blocks_start;

	char volume_name[16];
};

struct filesystem {
	struct dev		*dev;
	struct superblock	superblock;
};

/* superblock.c */
int superblock_write(struct dev *dev, struct superblock superblock);
int superblock_read(struct dev *dev, struct superblock *superblock);

struct inode {
	uint16_t i_mode;
	uint16_t i_uid;
	uint16_t i_gid;
	uint16_t i_hl_cnt;
	uint64_t f_size;
	uint32_t atime;
	uint32_t mtime;
	uint32_t ctime;

	uint32_t direct_blocks[INODE_DIRECT_BLOCKS];
	uint32_t inderect_block;
	uint32_t double_inderect_block;

	uint32_t i_flags;
	uint32_t i_generation;
};

/* bitmap.c */
int bitarr_write_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t res);
int bitarr_read_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t *res);

/* inode.c */
int inode_alloc(struct filesystem *fs);
int inode_free(struct filesystem *fs, uint32_t inode_id);
int inode_read(struct filesystem *fs, struct inode *inode, uint32_t inode_id);
int inode_write(struct filesystem *fs, struct inode inode, uint32_t inode_id);

/* format.c */
int make_fs(struct dev *dev, struct filesystem *output);

/* */

#endif /*!_FS_H*/
