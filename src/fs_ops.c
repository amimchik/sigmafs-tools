#include <sigmafs-tools/fs.h>
#include <string.h>

int set_dir_content(struct filesystem *fs, uint32_t inode_id, struct directory_entry *entries, uint32_t entries_c)
{
	if (!fs || !fs->dev || !entries)
		return 1;
	uint8_t *buf;
	uint64_t buf_len;
	if (serialize_dir_content(entries_c, entries, &buf, &buf_len))
		return 2;
	if (write_file(fs, inode_id, buf, buf_len))
		return 3;
	free(buf);
	return 0;
}

int mkdir(struct filesystem *fs, uint32_t parent_inode_id, uint8_t *name, uint16_t name_len)
{
	if (!fs || !fs->dev || !name)
		return -1;
	struct directory_entry *content;
	uint32_t content_len;
	if (get_dir_content(fs, parent_inode_id, &content, &content_len))
		return -2;
	for (uint32_t i = 0; i < content_len; i++) {
		if (name_len == content[i].name_len && !memcmp(content[i].name, name, name_len)) {
			free(content);
			return -3;
		}
	}
	int inode_id = inode_alloc(fs);
	if (inode_id < 0) {
		free(content);
		return -5;
	}
	struct directory_entry *tmp = realloc(content, content_len++);
	if (!tmp) {
		free(content);
		return -4;
	}
	content = tmp;
	content[content_len - 1].name_len = name_len;
	memcpy(content[content_len - 1].name, name, name_len);
	content[content_len - 1].inode_id = inode_id;
	struct inode inode;
	inode.i_mode = imode_create(IMODE_IFDIR, 0755);
	inode.i_hl_cnt = 2;
	inode.f_size = 0;
	if (inode_write(fs, inode, inode_id)) {
		free(content);
		return -7;
	}
	if (set_dir_content(fs, parent_inode_id, content, content_len)) {
		free(content);
		return -8;
	}
	free(content);
	content = malloc(sizeof(struct directory_entry) * 2);
	if (!content)
		return -9;
	content_len = 2;
	content[0].name_len = 1;
	content[0].name[0] = '.';
	content[0].inode_id = inode_id;
	content[1].name_len = 2;
	content[1].name[0] = '.';
	content[1].name[1] = '.';
	content[1].inode_id = parent_inode_id;
	int error = set_dir_content(fs, inode_id, content, content_len);
	if (error) {
		printf("sigareta: %d\n", error);
		free(content);
		return -11;
	}
	free(content);
	if (inode_read(fs, &inode, parent_inode_id))
		return -12;
	inode.i_hl_cnt++;
	if (inode_write(fs, inode, parent_inode_id))
		return -13;
	return inode_id;
}

int mkfile(struct filesystem *fs, uint32_t parent_inode_id, uint8_t *name, uint16_t name_len)
{
	if (!fs || !fs->dev || !name)
		return -1;
	struct directory_entry *content;
	uint32_t content_len;
	if (get_dir_content(fs, parent_inode_id, &content, &content_len))
		return -2;
	for (uint32_t i = 0; i < content_len; i++) {
		if (name_len == content[i].name_len && !memcmp(content[i].name, name, name_len)) {
			free(content);
			return -3;
		}
	}
	struct directory_entry *tmp = realloc(content, content_len++);
	if (!tmp) {
		free(content);
		return -4;
	}
	content = tmp;
	content[content_len - 1].name_len = name_len;
	memcpy(content[content_len - 1].name, name, name_len);
	int inode_id = inode_alloc(fs);
	if (inode_id < 0) {
		free(content);
		return -5;
	}
	struct inode inode;
	inode.i_mode = imode_create(IMODE_IFREG, 0755);
	inode.i_hl_cnt = 2;
	if (inode_write(fs, inode, inode_id)) {
		free(content);
		return -7;
	}
	if (set_dir_content(fs, parent_inode_id, content, content_len)) {
		free(content);
		return -8;
	}
	free(content);
	return inode_id;
}

int rmdir(struct filesystem *fs, uint32_t parent_inode_id, uint8_t *name, uint16_t name_len)
{
	if (!fs || !name || !fs->dev)
		return 1;
	int found_inode = get_entry_inode(fs, parent_inode_id, name, name_len);
	if (found_inode < 0)
		return found_inode;
	struct directory_entry *entries;
	uint32_t entries_count;
	if (get_dir_content(fs, parent_inode_id, &entries, &entries_count))
		return 2;
	uint32_t found_inode_id = 0;
	uint32_t found_index = 0;
	uint8_t found = 0;
	for (uint32_t i = 0; i < entries_count; i++) {
		if (name_len == entries[i].name_len && !memcmp(name, entries[i].name, name_len)) {
			found_inode = entries[i].inode_id;
			found_index = i;
			found = 1;
			break;
		}
	}
	if (!found) {
		free(entries);
		return 3;
	}
	{
		struct inode found_inode;
		if (inode_read(fs, &found_inode, found_inode_id)) {
			free(entries);
			return 4;
		}
		if (!imode_is_dir(found_inode.i_mode)) {
			free(entries);
			return 5;
		}
		struct directory_entry *found_content;
		uint32_t found_length;
		if (get_dir_content(fs, found_inode_id, &found_content, &found_length)) {
			free(entries);
			return 6;
		}
		if (found_length > 2) {
			free(entries);
			free(found_content);
			return 7;
		}
		free(found_content);
		found_inode.i_hl_cnt = 0;
		if (free_file_content(fs, found_inode_id)) {
			free(entries);
			return 8;
		}
		if (inode_free(fs, found_inode_id)) {
			free(entries);
			return 9;
		}
	}
	for (uint32_t i = found_index; i < entries_count - 1; i++) {
		entries[i] = entries[i + 1];
	}
	entries_count--;
	if (set_dir_content(fs, parent_inode_id, entries, entries_count)) {
		free(entries);
		return 10;
	}
	free(entries);
	struct inode parent_inode;
	if (inode_read(fs, &parent_inode, parent_inode_id))
		return 11;
	parent_inode.i_hl_cnt--;
	if (inode_write(fs, parent_inode, parent_inode_id))
		return 12;
	return 0;
}

int rmfile(struct filesystem *fs, uint32_t parent_inode_id, uint8_t *name, uint16_t name_len)
{
	if (!fs || !name || !fs->dev)
		return 1;
	int found_inode = get_entry_inode(fs, parent_inode_id, name, name_len);
	if (found_inode < 0)
		return found_inode;
	struct directory_entry *entries;
	uint32_t entries_count;
	if (get_dir_content(fs, parent_inode_id, &entries, &entries_count))
		return 2;
	int32_t found_inode_id = 0;
	int32_t found_index = 0;
	uint8_t found = 0;
	for (uint32_t i = 0; i < entries_count; i++) {
		if (name_len == entries[i].name_len && !memcmp(name, entries[i].name, name_len)) {
			found_inode = entries[i].inode_id;
			found_index = i;
			found = 1;
			break;
		}
	}
	if (!found) {
		free(entries);
		return 3;
	}
	{
		struct inode found_inode;
		if (inode_read(fs, &found_inode, found_inode_id)) {
			free(entries);
			return 4;
		}
		if (!imode_is_regular(found_inode.i_mode)) {
			free(entries);
			return 5;
		}
		found_inode.i_hl_cnt--;
		if (!found_inode.i_hl_cnt) {
			if (free_file_content(fs, found_inode_id)) {
				free(entries);
				return 8;
			}
			if (inode_free(fs, found_inode_id)) {
				free(entries);
				return 9;
			}
		}
	}
	for (uint32_t i = found_index; i < entries_count - 1; i++) {
		entries[i] = entries[i + 1];
	}
	entries_count--;
	if (set_dir_content(fs, parent_inode_id, entries, entries_count)) {
		free(entries);
		return 10;
	}
	free(entries);
	return 0;
}

int get_entry_inode(struct filesystem *fs, uint32_t parent_inode_id, uint8_t *name, uint16_t name_len)
{
	if (!fs || !fs->dev || !name)
		return -1;
	struct directory_entry *content;
	uint32_t content_len;
	if (get_dir_content(fs, parent_inode_id, &content, &content_len))
		return -2;
	for (uint32_t i = 0; i < content_len; i++) {
		if (name_len == content[i].name_len && !memcmp(content[i].name, name, name_len))
			return content[i].inode_id;
	}
	return -3;
}

int get_dir_content(struct filesystem *fs, uint32_t parent_inode_id, struct directory_entry **entries, uint32_t *entries_count)
{
	if (!entries || !fs || !fs->dev || !entries_count)
		return 1;
	struct inode inode;
	if (inode_read(fs, &inode, parent_inode_id))
		return 1;
	uint64_t content_ser_len = inode.f_size;
	uint8_t *content_ser = malloc(content_ser_len);
	if (!content_ser)
		return 2;
	if (read_file(fs, parent_inode_id, content_ser))
		return 3;
	if (parse_dir_content(content_ser, content_ser_len, entries_count, entries))
		return 4;
	free(content_ser);
	return 0;
}
