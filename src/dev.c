#include <sigmafs-tools/dev.h>

int dev_create_ramdisk(struct dev *out, size_t block_size, size_t blocks_count)
{
	if (!out)
		return 1;
	out->type = DT_RAM;
	out->block_size = block_size;
	out->ram = malloc(block_size * blocks_count);
	if (!out->ram)
		return 1;
	out->blocks_count = blocks_count;
	return 0;
}

int dev_create_file(struct dev *out, const char *path, size_t block_size, size_t blocks_count)
{
	if (!out)
		return 1;
	out->type = DT_FILE;
	out->block_size = block_size;
	out->blocks_count = blocks_count;
	out->f = fopen(path, "r+b");
	if (!out->f) {
		out->f = fopen(path, "w+b");
		uint8_t zero = 0;
		for (size_t i = 0; i < block_size * blocks_count; i++)
			fwrite(&zero, 1, 1, out->f);
	}
	if (!out->f)
		return 1;
	return 0;
}

int dev_read_block(struct dev *dev, size_t block_n, uint8_t *buf)
{
	if (!dev || !buf)
		return 1;
	if (block_n >= dev->blocks_count) {
	    fprintf(stderr, "invalid block: %zu\n", block_n);
	    return 1;
	}
	if (dev->type == DT_RAM) {
		memcpy(buf, dev->ram, dev->block_size);
	} else if (dev->type == DT_FILE) {
		fseek(dev->f, block_n * dev->block_size, SEEK_SET);
		if (fread(buf, dev->block_size, 1, dev->f) != 1) {
			fprintf(stderr, "error reading\n");
			return 2;
		}
	}
	return 0;
}

int dev_write_block(struct dev *dev, size_t block_n, uint8_t *buf)
{
	if (!dev || !buf)
		return 1;
	if (block_n >= dev->blocks_count) {
	    fprintf(stderr, "invalid block: %zu\n", block_n);
	    return 1;
	}
	if (dev->type == DT_RAM) {
		memcpy(dev->ram, buf, dev->block_size);
	} else if (dev->type == DT_FILE) {
		fseek(dev->f, block_n * dev->block_size, SEEK_SET);
		size_t written = fwrite(buf, dev->block_size, 1, dev->f);
		if (written != 1)
			return 2;
	}
	return 0;
}
