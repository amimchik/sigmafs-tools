#include <sigmafs-tools/fs.h>
#include <stdint.h>


uint64_t inode_alloc(struct filesystem *fs)
{
	if (!fs || !fs->dev)
		return UINT64_MAX;

}

int inode_read(struct filesystem *fs, struct inode *inode, uint64_t inode_id)
{
}

int inode_write(struct filesystem *fs, struct inode inode, uint64_t inode_id)
{
}
