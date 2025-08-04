#include <sigmafs-tools/fs.h>
#include <stdio.h>
#include <string.h>

void l_cd(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_cd(state, (uint8_t *)name, strlen(name));
	if (error)
		printf("eroro: %d\n", error);
}

void l_ls(struct fs_interface_state *state)
{
	struct directory_entry *entries;
	uint32_t len = 0;
	int error = fs_interface_ls(state, &entries, &len);
	if (error) {
		printf("error ls: %d\n", error);
		return;
	}
	for (uint32_t i = 0; i < len; i++) {
		printf("%u\t", entries[i].inode_id);
		for (uint32_t z = 0; z < entries[i].name_len; z++)
			putchar(entries[i].name[z]);
		putchar('\n');
	}
	free(entries);
}

void l_mkdir(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_mkdir(state, (uint8_t *)name, strlen(name));
	if (error < 0) {
		printf("Error mkdir: %d\n", error);
	}
}

int main()
{
	struct dev dev;
	int error = dev_create_file(&dev, "disk.img", 512, 200);
	if (error)
		printf("error creating: %d\n", error);
	struct filesystem fs;
	strcpy(fs.superblock.volume_name, "test");
	error = make_fs(&dev, &fs, fs.superblock);
	if (error) {
		printf("error formatting: %d\n", error);
		return 1;
	}
	struct fs_interface_state interface;
	error = create_fs_interface(&fs, &interface);
	if (error) {
		printf("Error interface: %d\n", error);
		return 1;
	}
	printf("doing: mkdir testdir\n");
	l_mkdir(&interface, "testdir");
	printf("doing: ls\n");
	l_ls(&interface);
	printf("doing: cd testdir\n");
	l_cd(&interface, "testdir");
	printf("doing: ls\n");
	l_ls(&interface);
	printf("doing: cd ..\n");
	l_cd(&interface, "..");
	printf("doing: ls\n");
	l_ls(&interface);
	return 0;
}
