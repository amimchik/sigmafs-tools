#include <sigmafs-tools/fs.h>

int create_fs_interface(struct filesystem *fs, struct fs_interface_state *out)
{
	if (!fs || !fs->dev || !out)
		return 1;
	out->fs = fs;
	out->wd_id = 0;
	return 0;
}

int fs_interface_cd(struct fs_interface_state *state, uint8_t *dir_name, uint16_t dir_name_len)
{
	if (dir_name_len == 1 && dir_name[0] == '/') {
		state->wd_id = SIGMAFS_ROOT_INODE_ID;
		return 0;
	}
	int entry_id = get_entry_inode(state->fs, state->wd_id, dir_name, dir_name_len);
	if (entry_id < 0)
		return entry_id;
	struct inode entry_inode;
	int err = inode_read(state->fs, &entry_inode, entry_id);
	if (err)
		return err;
	if (!imode_is_dir(entry_inode.i_mode))
		return 1;
	state->wd_id = entry_id;
	return 0;
}

int fs_interface_ls(struct fs_interface_state *state, struct directory_entry **entries, uint32_t *len)
{
	return get_dir_content(state->fs, state->wd_id, entries, len);
}

int fs_interface_mkdir(struct fs_interface_state *state, uint8_t *name, uint16_t name_len)
{
	return mkdir(state->fs, state->wd_id, name, name_len);
}

int fs_interface_touch(struct fs_interface_state *state, uint8_t *name, uint16_t name_len)
{
	return mkfile(state->fs, state->wd_id, name, name_len);
}

int fs_interface_rm(struct fs_interface_state *state, uint8_t *name, uint16_t name_len)
{
	return rmfile(state->fs, state->wd_id, name, name_len);
}

int fs_interface_rmdir(struct fs_interface_state *state, uint8_t *name, uint16_t name_len)
{
	return rmdir(state->fs, state->wd_id, name, name_len);
}
int fs_interface_cat(struct fs_interface_state *state, uint8_t *name, uint16_t name_len, uint8_t **buf, uint64_t *buf_len)
{
	int f_id = get_entry_inode(state->fs, state->wd_id, name, name_len);
	if (f_id < 0)
		return 1;
	struct inode f_inode;
	int err = inode_read(state->fs, &f_inode, f_id);
	if (err)
		return err;
	if (!imode_is_dir(f_inode.i_mode))
		return 2;
	*buf = malloc(f_inode.f_size);
	if (!buf)
		return 3;
	*buf_len = f_inode.f_size;
	return read_file(state->fs, f_id, *buf);
}

int fs_interface_write(struct fs_interface_state *state, uint8_t *name, uint16_t name_len, uint8_t *buf, uint64_t buf_len)
{
	int f_id = get_entry_inode(state->fs, state->wd_id, name, name_len);
	if (f_id < 0)
		return 1;
	struct inode f_inode;
	int err = inode_read(state->fs, &f_inode, f_id);
	if (err)
		return err;
	if (!imode_is_dir(f_inode.i_mode))
		return 2;
	return write_file(state->fs, f_id, buf, buf_len);
}
