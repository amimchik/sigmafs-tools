#include <sigmafs-tools/dev.h>
#include <errno.h>

/**
 * dev_create_ramdisk() -- creates ramdisk
 * @out:		dev
 * @block_size:		size of a single block
 * @blocks_count:	count of blocks to allocate
 */
int dev_create_ramdisk(struct dev *out, size_t block_size, size_t blocks_count)
{
	if (!out)
		return -EINVAL;
	out->type = DT_RAM;
	out->block_size = block_size;
	out->ram = calloc(blocks_count, block_size);
	if (!out->ram)
		return -ENOMEM;
	out->blocks_count = blocks_count;
	return 0;
}

/**
 * dev_create_file() -- creates file drive
 * @out:		dev
 * @path:		path of file
 * @block_size:		size of a single block
 * @blocks_count:	count of blocks to allocate
 */
int dev_create_file(struct dev *out, const char *path, size_t block_size, size_t blocks_count)
{
	if (!out)
		return -EINVAL;
	out->type = DT_FILE;
	out->block_size = block_size;
	out->blocks_count = blocks_count;
	out->f = fopen(path, "r+b");
	int ret = 0;
	if (!out->f) {
		out->f = fopen(path, "w+b");
			if (!out->f) {
				ret = -EINVAL;
				goto err;
			}
		uint8_t *zeroes = calloc(1, block_size);
		if (!zeroes) {
			ret = -ENOMEM;
			goto err;
		}
		for (size_t i = 0; i < blocks_count; i++) {
			ret = fwrite(zeroes, block_size, 1, out->f);
			if (ret != 1) {
				ret = -EIO;
				free(zeroes);
				goto err;
			}
		}
		free(zeroes);
	}
	ret = fflush(out->f);
	if (ret != 0) {
		ret = -EIO;
		goto err;
	}
	return 0;
err:
	if (out && out->f) {
		fclose(out->f);
		out->f = NULL;
	}
	return ret;
}

/**
 * dev_read_block() -- reads block from device
 * @dev:		dev
 * @block_n:		block index
 * @buf:		buffer
 */
int dev_read_block(struct dev *dev, size_t block_n, uint8_t *buf)
{
	if (!dev || !buf)
		return -EINVAL;
	if (block_n >= dev->blocks_count) {
		fprintf(stderr, "invalid block: %zu\n", block_n);
		return -EINVAL;
	}
	if (dev->type == DT_RAM) {
		memcpy(buf, dev->ram + dev->block_size * block_n, dev->block_size);
	} else if (dev->type == DT_FILE) {
		fseek(dev->f, block_n * dev->block_size, SEEK_SET);
		if (fread(buf, dev->block_size, 1, dev->f) != 1) {
			fprintf(stderr, "error reading\n");
			return -EIO;
		}
	}
	return 0;
}

/**
 * dev_write_block() -- reads block from device
 * @dev:		dev
 * @block_n:		block index
 * @buf:		buffer
 */
int dev_write_block(struct dev *dev, size_t block_n, uint8_t *buf)
{
	if (!dev || !buf)
		return -EINVAL;
	if (block_n >= dev->blocks_count) {
		fprintf(stderr, "invalid block: %zu\n", block_n);
		return -EINVAL;
	}
	if (dev->type == DT_RAM) {
		memcpy(dev->ram + dev->block_size * block_n, buf, dev->block_size);
	} else if (dev->type == DT_FILE) {
		fseek(dev->f, block_n * dev->block_size, SEEK_SET);
		size_t written = fwrite(buf, dev->block_size, 1, dev->f);
		if (written != 1)
			return -EIO;
	}
	return 0;
}
