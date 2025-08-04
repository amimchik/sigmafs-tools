#include <sigmafs-tools/shell.h>

static void l_cd(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_cd(state, (uint8_t *)name, strlen(name));
	if (error)
		printf("eroro: %d\n", error);
}

static void l_touch(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_touch(state, (uint8_t *)name, strlen(name));
	if (error)
		printf("eroro: %d\n", error);
}

static void l_rm(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_rm(state, (uint8_t *)name, strlen(name));
	if (error)
		printf("eroro: %d\n", error);
}

static void l_rmdir(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_rmdir(state, (uint8_t *)name, strlen(name));
	if (error)
		printf("eroro: %d\n", error);
}

static void l_ls(struct fs_interface_state *state)
{
	struct directory_entry *entries;
	uint32_t len = 0;
	int error = fs_interface_ls(state, &entries, &len);
	if (error) {
		printf("error ls: %d\n", error);
		return;
	}
	for (uint32_t i = 0; i < len; i++) {
		printf("%u\t", entries[i].inode_id);
		for (uint32_t z = 0; z < entries[i].name_len; z++)
			putchar(entries[i].name[z]);
		putchar('\n');
	}
	free(entries);
}

static void l_mkdir(struct fs_interface_state *state, const char *name)
{
	int error = fs_interface_mkdir(state, (uint8_t *)name, strlen(name));
	if (error < 0) {
		printf("Error mkdir: %d\n", error);
	}
}

void shell_run(struct fs_interface_state *fs)
{
	char input_buf[10][100];
	char line[1000];
	uint8_t running = 1;
	while (running) {
		if (!fgets(line, sizeof(line), stdin) && feof(stdin))
			break;
		line[strcspn(line, "\n")] = 0;
		char *part = strtok(line, " ");
		uint32_t argc = 0;
		for (; part; argc++) {
			if (argc >= 10) {
				fprintf(stderr, "buf lim exeeded\n");
				return;
			}
			strcpy(input_buf[argc], part);
			part = strtok(NULL, " ");
		}
		if (argc == 0)
			continue;
		if (!strcmp(input_buf[0], "ls"))
			l_ls(fs);
		else if (argc == 1)
			fprintf(stderr, "to few args\n");
		else if (!strcmp(input_buf[0], "cd"))
			l_cd(fs, input_buf[1]);
		else if (!strcmp(input_buf[0], "mkdir"))
			l_mkdir(fs, input_buf[1]);
		else if (!strcmp(input_buf[0], "touch"))
			l_touch(fs, input_buf[1]);
		else if (!strcmp(input_buf[0], "rmdir"))
			l_rmdir(fs, input_buf[1]);
		else if (!strcmp(input_buf[0], "rm"))
			l_rm(fs, input_buf[1]);
		else
			fprintf(stderr, "no such command\n");
	}
	return;
}
