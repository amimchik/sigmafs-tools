#include <sigmafs-tools/fs.h>
#include <errno.h>

/**
 * bitarr_write_bit() -- writes bit to device
 * @dev:		device
 * @block:		block to work with
 * @offset:		bit offset
 * @res:		value to write
 */
int bitarr_write_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t res)
{
	if (!dev)
		return -EINVAL;
	uint32_t byte_offset = offset / 8;
	uint32_t bit = offset % 8;
	uint32_t block_num = block + (byte_offset / dev->block_size);
	uint32_t block_offset = byte_offset % dev->block_size;
	uint8_t *buf = malloc(dev->block_size);
	if (!buf)
		return -ENOMEM;
	if (dev_read_block(dev, block_num, buf) < 0) {
		free(buf);
		return -EIO;
	}
	if (res)
		buf[block_offset] |= (1 << bit);
	else
		buf[block_offset] &= ~(1 << bit);
	if (dev_write_block(dev, block_num, buf) < 0) {
		free(buf);
		return -EIO;
	}
	free(buf);
	return 0;
}

/**
 * bitarr_read_bit() -- read bit to device
 * @dev:		device
 * @block:		block to work with
 * @offset:		bit offset
 * @res:		result
 */
int bitarr_read_bit(struct dev *dev, uint32_t block, uint32_t offset, uint8_t *res)
{
	if (!dev || !res) 
		return -EINVAL; 
	uint32_t byte_offset = offset / 8; 
	uint32_t bit = offset % 8; 
	uint32_t block_num = block + (byte_offset / dev->block_size); 
	uint32_t block_offset = byte_offset % dev->block_size; 
	uint8_t *buf = malloc(dev->block_size); 
	if (!buf) 
		return -ENOMEM; 
	if (dev_read_block(dev, block_num, buf) < 0) { 
		free(buf);
		return -EIO; 
	} 
	*res = (buf[block_offset] & (1 << bit)) ? 1 : 0;
	free(buf); 
	return 0; 
}
