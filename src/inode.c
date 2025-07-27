#include <sigmafs-tools/fs.h>


uint64_t inode_alloc(filesystem *fs)
{
	if (!fs || !fs->dev)
		return UINT64T_MAX;

}

int inode_read(struct filesystem *fs, struct inode *inode, uint64_t inode_id)
{
}

int inode_write(struct filesystem *fs, struct inode inode, uint64_t inode_id)
{
}
