#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


struct chunk {
	struct chunk		*next;
	uint16_t		loadaddr;
	uint16_t		size;
	uint8_t			*data;
	char			name[64];
	int			under_io;

	uint16_t s_length;
	uint16_t s_offset;
	uint16_t s_loadaddr;
	int s_bang;

	time_t loadtime;

};

struct group {
	struct chunk	*chunk;
	struct group	*next;
};

struct group *script;
int verbose = 0;

static void load_script(char *filename) {
	FILE *f, *cf;
	char buf[256], *ptr, *name;
	int bang, newgroup = 1;
	long int loadaddr, offset, length;
	struct chunk *chunk, **cdest = 0;
	struct group *group = 0, **gdest = &script;

	f = fopen(filename, "r");
	if(!f) err(1, "fopen: %s", filename);

	while(fgets(buf, sizeof(buf), f)) {
		if(strlen(buf) && buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = 0;
		}
		if(strlen(buf) && buf[strlen(buf) - 1] == '\r') {
			buf[strlen(buf) - 1] = 0;
		}
		ptr = buf;
		while(*ptr == ' ' || *ptr == '\t') ptr++;
		if(!*ptr) {
			newgroup = 1;
		} else if(*ptr != ';' && *ptr != '#') {
			if(*ptr == '"') {
				name = ++ptr;
				while(*ptr && *ptr != '"') ptr++;
			} else {
				name = ptr;
				while(*ptr && *ptr != ' ' && *ptr != '\t') {
					ptr++;
				}
			}
			if(*ptr) *ptr++ = 0;
			loadaddr = strtol(ptr, &ptr, 16);
			while(*ptr == ' ' || *ptr == '\t') ptr++;
			if(*ptr == '!') {
				bang = 1;
				ptr++;
			} else {
				bang = 0;
			}
			offset = strtol(ptr, &ptr, 16);
			length = strtol(ptr, &ptr, 16);
			while(*ptr == ' ' || *ptr == '\t') ptr++;
			if(*ptr) {
				errx(
					1,
					"Unexpected characters at end "
					"of script line (%s).",
					ptr);
			}

			if(loadaddr < 0 || loadaddr > 0xffff) {
				errx(1, "Invalid load address ($%lx)", loadaddr);
			}
			if(length < 0 || length > 0xffff) {
				errx(1, "Invalid load length ($%lx)", length);
			}
			if(offset < 0) {
				errx(1, "Invalid load offset ($%lx)", offset);
			}

			if(newgroup) {
				group = malloc(sizeof(struct group));
				cdest = &group->chunk;
				*gdest = group;
				gdest = &group->next;
				newgroup = 0;
			}

			chunk = calloc(1, sizeof(struct chunk));

			chunk->size = 0;
			chunk->loadaddr = 0;
			chunk->s_length = length;
			chunk->s_offset = offset;
			chunk->s_loadaddr = loadaddr;
			chunk->s_bang = bang;
			snprintf(chunk->name, sizeof(chunk->name), "%s", name);
			chunk->data = NULL;

			chunk->loadtime = (time_t)-1;

			*cdest = chunk;
			cdest = &chunk->next;
		}
	}
	*gdest = 0;

	if(!script) errx(1, "Empty script!");

	fclose(f);
}

void loadchunk(struct chunk *chunk)
{
	FILE *cf;

	long int  length = chunk->s_length;
	long int  offset = chunk->s_offset;
	long int  loadaddr = chunk->s_loadaddr;
	int bang = chunk->s_bang;
	char *name = chunk->name;

	if(length == 0) length = 0xffff;

	if (chunk->data)
		free(chunk->data);

	chunk->data = malloc(length);

	cf = fopen(name, "rb");
	if(!cf) err(1, "fopen: %s", name);
	if(fseek(cf, offset, SEEK_SET) < 0) {
		err(1, "fseek: %s, $%lx", name, offset);
	}
	if(!loadaddr) {
		loadaddr = fgetc(cf);
		loadaddr |= fgetc(cf) << 8;
		if(loadaddr < 0 || loadaddr >= 0xffff) {
			errx(
				1,
				"Error obtaining load "
				"address from file: %s",
				name);
		}
	}

	length = fread(chunk->data, 1, length, cf);
	if(!length) err(1, "fread: %s", name);
	fclose(cf);
	chunk->loadaddr = loadaddr;
	chunk->size = length;

	chunk->under_io =
		(!bang) &&
		(chunk->loadaddr < 0xe000) &&
		((chunk->loadaddr + chunk->size) > 0xd000);
}

static int load_chunks()
{
	int updated = 0;
	struct group *group = script;

	while(group)
	{
		struct chunk *chunk = group->chunk;
		while(chunk)
		{
			time_t newtime;

			struct stat attr;
			stat(chunk->name, &attr);
			newtime = attr.st_mtime;

			float since_load = difftime(newtime, chunk->loadtime);
			//printf("file: '%s', since load: %.2f\n", chunk->name, since_load);

			if (since_load > 0.1f)
			{
				chunk->loadtime = newtime;

				loadchunk(chunk);
				updated = 1;
			}

			chunk = chunk->next;
		}

		group = group->next;
	}

	return updated;
}

static uint16_t get_entry(int entry)
{
	int firstchunk = 1;

	struct group *group = script;

	while(group)
	{
		struct chunk *chunk = group->chunk;
		while(chunk)
		{
			if(firstchunk) {
				if(entry < 0) entry = chunk->loadaddr;
				firstchunk = 0;
			}
			chunk = chunk->next;
		}

		group = group->next;
	}

	return (uint16_t)entry;
}



int openvice(char *addr, int port)
{
	struct sockaddr_in servaddr;
	int s;
	
	s = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(addr);
	servaddr.sin_port=htons(port);

	connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr));

	printf("connect socket: %i\n", s);
	if (!s)
		return -1;

	return s;
}

int main(int argc, char *argv[])
{
	uint16_t entry;

	char *viceaddr = strdup(argv[1]);
	int viceport = atoi(argv[2]);
	char *srcfn = strdup(argv[3]);


	load_script(srcfn);

	while(1)
	{

		int updated = load_chunks();

		if (updated)
		{
			struct group *curgroup = script->next;

			int s = openvice(viceaddr, viceport);
			if (!s)
				return 1;

			FILE *fs = fdopen(s, "w");

			fprintf(fs, "\n");
			fflush(fs);

			usleep(1000);

			entry = curgroup->chunk->loadaddr;
			printf("entry: %04X\n", entry);

			struct chunk *curchunk = curgroup->chunk;
			while(curchunk)
			{
				printf("name '%s', load %04X, size %04X\n", curchunk->name, curchunk->loadaddr, curchunk->size);

				int i;

				for (i = 0; i < curchunk->size;)
				{
					int end = i + 50;
					if (end > curchunk->size)
						end = curchunk->size;

					char text[256*3+256];
					sprintf(text, "> %04X ", curchunk->loadaddr + i);

					char *tp = text + strlen(text);

					for (; i < end; i++)
					{
						tp += sprintf(tp, "%02X ", curchunk->data[i]);
					}

					//printf("sending vice: '%s'\n", text);

					fprintf(fs, "%s\n", text);
					fflush(fs);
					usleep(1000);
				}
				curchunk = curchunk->next;
			}

			//printf("sending vice: 'g %X'\n", entry);
			fprintf(fs, "g %X\n", entry);
			fflush(fs);

	#if 0
		while(!feof(fr))
		{
			char rivi[1024];
			fgets(rivi, 1024, fr);
			printf("got from vice: '%s'\n", rivi);
		}
	#endif
			usleep(100000);
			fclose(fs);
			//fclose(fr);
		}

		usleep(700000);
	}

	return 0;
}
