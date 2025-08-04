#include <sigmafs-tools/fs.h>
#include <sigmafs-tools/le.h>

int parse_dir_content(uint8_t *content, uint64_t content_len, uint32_t *entries_count, struct directory_entry **entries)
{
	if (!entries || !entries_count)
		return 1;
	uint32_t cap = 10;
	uint32_t working_entry = 0;
	*entries = malloc(sizeof(struct directory_entry) * cap);
	*entries_count = 0;
	if (!*entries)
		return 1;
	*entries_count = 0;
	for (uint64_t i = 0; i < content_len;) {
		struct directory_entry entry;
		uint16_t name_i = 0;
		while (content[i] != '/') {
			if (i >= content_len || name_i >= SIGMAFS_MAX_NAME_SIZE) {
				free(*entries);
				return 1;
			}
			entry.name[name_i++] = content[i++];
		}
		i++;
		entry.name_len = name_i;
		/*printf("%d;", entry.name_len);
		for (uint32_t x = 0; x < entry.name_len; x++)
			putchar(entry.name[x]);
		putchar('\n');*/
		entry.inode_id = read_u32_le(content + i);
		i += 4;
		if (working_entry >= cap) {
			struct directory_entry *old = *entries;
			*entries = realloc(*entries, sizeof(struct directory_entry) * cap * 2);
			if (!*entries) {
				free(old);
				return 1;
			}
			cap *= 2;
		}
		(*entries)[working_entry++] = entry;
		(*entries_count)++;
	}
	return 0;
}

int serialize_dir_content(uint32_t entries_count, struct directory_entry *entries, uint8_t **content, uint64_t *content_len)
{
	if (!entries || !content || !content_len)
		return 1;
	uint64_t cap = 100;
	uint32_t ch_i = 0;
	*content = malloc(cap);
	if (!*content)
		return 1;
	for (uint32_t i = 0; i < entries_count; i++) {
		for (uint16_t name_i = 0; name_i < entries[i].name_len; name_i++) {
			if (ch_i >= cap) {
				uint8_t *old = *content;
				*content = realloc(*content, cap * 2);
				if (!*content) {
					free(old);
					return 1;
				}
				cap *= 2;
			}
			(*content)[ch_i++] = entries[i].name[name_i];
		}
		if (ch_i >= cap) {
			uint8_t *old = *content;
			*content = realloc(*content, cap * 2);
			if (!*content) {
				free(old);
				return 1;
			}
			cap *= 2;
		}
		(*content)[ch_i++] = '/';
		if (ch_i + 4 >= cap) {
			uint8_t *old = *content;
			uint64_t new_cap = cap > 4 ? cap * 2 : cap + 4;
			*content = realloc(*content, new_cap);
			if (!*content) {
				free(old);
				return 1;
			}
			cap = new_cap;
		}
		write_u32_le(*content + ch_i, entries[i].inode_id);
		ch_i += 4;
	}
	*content_len = ch_i;
	return 0;
}
