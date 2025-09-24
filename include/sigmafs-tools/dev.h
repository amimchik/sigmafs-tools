#ifndef _DEV_H
#define _DEV_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Type of device */
enum dev_type {
	DT_RAM,
	DT_FILE,
};

/* Device representation */
struct dev {
	int type;
	size_t block_size;
	size_t blocks_count;
	union {
		uint8_t *ram;
		FILE *f;
	};
};

/* dev.c */
int dev_create_ramdisk(struct dev *out, size_t block_size, size_t blocks_count);
int dev_create_file(struct dev *out, const char *path, size_t block_size, size_t blocks_count);

int dev_read_block(struct dev *dev, size_t block_n, uint8_t *buf);
int dev_write_block(struct dev *dev, size_t block_n, uint8_t *buf);

int dev_free(struct dev *dev);

#endif /*!_DEV_H*/
