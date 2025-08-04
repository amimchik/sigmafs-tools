#include <sigmafs-tools/fs.h>


int bitarr_write_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t res)
{
	if (!dev)
		return 1;
	uint32_t byte_offset = offset / 8;
	uint32_t bit = offset % 8;
	uint32_t block_num = block + (byte_offset / dev->block_size);
	uint32_t block_offset = byte_offset % dev->block_size;
	uint8_t *buf = malloc(dev->block_size);
	if (!buf)
		return 1;
	if (dev_read_block(dev, block_num, buf)) {
		free(buf);
		return 1;
	}
	if (res)
		buf[block_offset] |= (1 << bit);
	else
		buf[block_offset] &= ~(1 << bit);
	if (dev_write_block(dev, block_num, buf)) {
		free(buf);
		return 1;
	}
	free(buf);
	return 0;
}

int bitarr_read_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t *res)
{
	if (!dev || !res) 
		return 1; 
	uint32_t byte_offset = offset / 8; 
	uint32_t bit = offset % 8; 
	uint32_t block_num = block + (byte_offset / dev->block_size); 
	uint32_t block_offset = byte_offset % dev->block_size; 
	uint8_t *buf = malloc(dev->block_size); 
	if (!buf) 
		return 2; 
	int err = dev_read_block(dev, block_num, buf);
	if (err) { 
		free(buf);
		printf("bitarr: %d\n", err);
		return 3; 
	} 
	*res = (buf[block_offset] & (1 << bit)) ? 1 : 0;
	free(buf); 
	return 0; 
}
