#ifndef _IMODE_H
#define _IMODE_H

#define IMODE_IFSOCK		0xC000
#define IMODE_IFLNK		0xA000
#define IMODE_IFREG		0x8000
#define IMODE_IFBLK		0x6000
#define IMODE_IFDIR		0x4000
#define IMODE_IFCHR		0x2000
#define IMODE_IFIFO		0x1000

#define IMODE_IFMT		0xF000

#define IMODE_SUID		0x0800
#define IMODE_SGID		0x0400
#define IMODE_STICKY		0x0200

#define IMODE_UR		0x0100
#define IMODE_UW		0x0080
#define IMODE_UX		0x0040

#define IMODE_GR		0x0020
#define IMODE_GW		0x0010
#define IMODE_GX		0x0008

#define IMODE_OR		0x0004
#define IMODE_OW		0x0002
#define IMODE_OX		0x0001

#define IMODE_PERMS		0x0FFF

static inline uint16_t imode_get_type(uint16_t mode) {
	return mode & IMODE_IFMT;
}

static inline int imode_is_regular(uint16_t mode) {
	return (mode & IMODE_IFMT) == IMODE_IFREG;
}

static inline int imode_is_dir(uint16_t mode) {
	return (mode & IMODE_IFMT) == IMODE_IFDIR;
}

static inline int imode_is_symlink(uint16_t mode) {
	return (mode & IMODE_IFMT) == IMODE_IFLNK;
}

static inline int imode_is_device(uint16_t mode) {
	return (mode & IMODE_IFMT) == IMODE_IFBLK || (mode & IMODE_IFMT) == IMODE_IFCHR;
}

static inline uint16_t imode_get_perms(uint16_t mode) {
	return mode & IMODE_PERMS;
}

static inline uint16_t imode_set_type(uint16_t mode, uint16_t type) {
	return (mode & ~IMODE_IFMT) | (type & IMODE_IFMT);
}

static inline uint16_t imode_set_perms(uint16_t mode, uint16_t perms) {
	return (mode & ~IMODE_PERMS) | (perms & IMODE_PERMS);
}

static inline uint16_t imode_create(uint16_t type, uint16_t perms) {
	return (type & IMODE_IFMT) | (perms & IMODE_PERMS);
}

#endif /*!_IMODE_H*/
