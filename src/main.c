#include <sigmafs-tools/fs.h>
#include <errno.h>
#include <sigmafs-tools/shell.h>
#include <stdio.h>
#include <string.h>

static int ls_r(struct fs_interface_state *interface, int indentation)
{
	struct directory_entry *entries = NULL;
	uint32_t entries_count = 0;
	int ret = fs_interface_ls(interface, &entries, &entries_count);
	if (ret)
		goto end;
	for (int i = 0; i < entries_count; i++) {
		struct inode inode;
		ret = inode_read(interface->fs, &inode, entries[i].inode_id);
		if (ret)
			goto end;
		for (int z = 0; z < indentation; z++)
			printf("\t");
		for (int z = 0; z < entries[i].name_len; z++)
			putc(entries[i].name[z], stdout);
		printf("\t%hu", entries[i].inode_id);
		if (!(entries[i].name_len == 1 && entries[i].name[0] == '.') &&
			!(entries[i].name_len == 2 && entries[i].name[0] == '.' && entries[i].name[1] == '.') &&
			imode_is_dir(inode.i_mode)) {
			ret = fs_interface_cd(interface, entries[i].name, entries[i].name_len);
			if (ret)
				goto end;
			printf(":\n");
			ret = ls_r(interface, indentation + 1);
			if (ret)
				goto end;
			ret = fs_interface_cd(interface, "..", 2);
			if (ret)
				goto end;
		} else {
			printf("\n");
		}
	}
end:
	free(entries);
	return ret;
}

#define CEIL_DIV(a, b) ((a) + (b) - 1) / (b)

static int dump_inode_alloc_table(struct filesystem *fs)
{
	if (!fs)
		return -EINVAL;
	uint32_t inode_bmap_block = fs->superblock.inode_bitmap_block;
	uint32_t len = CEIL_DIV(fs->superblock.total_inodes, fs->superblock.block_size * 8);
	for (int i = 0; i < 200; i++) {
		uint8_t bit = 0;
		int ret = bitarr_read_bit(fs->dev, inode_bmap_block, i, &bit);
		printf("%s ", bit ? "@" : " ");
	}
	return 0;
}

int main()
{
	struct dev dev;
	int error = dev_create_ramdisk(&dev, 512, 200);
	if (error)
		printf("error creating: %d\n", error);
	struct filesystem fs;
	strcpy(fs.superblock.volume_name, "test");
	error = make_fs(&dev, &fs, fs.superblock);
	if (error) {
		printf("error formatting: %d\n", error);
		return 1;
	}
	/*struct fs_interface_state i;
	error = create_fs_interface(&fs, &i);
	if (error) {
		printf("Error interface: %d\n", error);
		return 1;
	}
	fs_interface_mkdir(&i, "test", strlen("test"));
	fs_interface_cd(&i, "test", strlen("test"));
	fs_interface_mkdir(&i, "another", strlen("another"));
	fs_interface_cd(&i, "/", strlen("/"));
	ls_r(&i, 0);*/
	for (int i = 0; i < 8; i++) {
		int allocated = inode_alloc(&fs);
		printf("%d\n", allocated);
	}
	/* dump_inode_alloc_table(&fs); */
	return 0;
}
