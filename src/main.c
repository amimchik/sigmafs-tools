#include <sigmafs-tools/fs.h>
#include <stdio.h>
#include <string.h>

int main()
{
	struct dev dev;
	if (dev_create_file(&dev, "disk.img", 512, 2))
		printf("error creating\n");
	struct superblock superblock;
	superblock.magic = 123123;
	superblock.version = 1423;
	strcpy(superblock.volume_name, "test");
	if (superblock_write(&dev, superblock))
		printf("error\n");
	return 0;
}
