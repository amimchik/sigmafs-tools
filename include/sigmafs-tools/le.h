#ifndef _LE_H
#define _LE_H


/*
 * Functions to work with numbers
 */

static inline void write_u16_le(uint8_t *buf, uint16_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
}

static inline void write_u32_le(uint8_t *buf, uint32_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
}

static inline void write_u64_le(uint8_t *buf, uint64_t val)
{
	buf[0] = (uint8_t)(val);
	buf[1] = (uint8_t)(val >> 8);
	buf[2] = (uint8_t)(val >> 16);
	buf[3] = (uint8_t)(val >> 24);
	buf[4] = (uint8_t)(val >> 32);
	buf[5] = (uint8_t)(val >> 40);
	buf[6] = (uint8_t)(val >> 48);
	buf[7] = (uint8_t)(val >> 56);
}

static inline uint16_t read_u16_le(const uint8_t *buf)
{
	return (uint16_t)buf[0] |
	       ((uint16_t)buf[1] << 8);
}

static inline uint32_t read_u32_le(const uint8_t *buf)
{
	return (uint32_t)buf[0] |
	       ((uint32_t)buf[1] << 8) |
	       ((uint32_t)buf[2] << 16) |
	       ((uint32_t)buf[3] << 24);
}

static inline uint64_t read_u64_le(const uint8_t *buf)
{
	return (uint64_t)buf[0] |
	       ((uint64_t)buf[1] << 8) |
	       ((uint64_t)buf[2] << 16) |
	       ((uint64_t)buf[3] << 24) |
	       ((uint64_t)buf[4] << 32) |
	       ((uint64_t)buf[5] << 40) |
	       ((uint64_t)buf[6] << 48) |
	       ((uint64_t)buf[7] << 56);
}

#endif /*!_LE_H*/
