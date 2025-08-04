#include <sigmafs-tools/fs.h>
#include <sigmafs-tools/shell.h>
#include <stdio.h>
#include <string.h>


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
	shell_run(&interface);
	return 0;
}
