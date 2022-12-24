#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "readbytes.h"

#ifdef _WIN32
#define mkdir(path, mode) mkdir(path)
#endif

/* header: */
/* 0x37 0x6a 0x00 0x00 */
/* 32-bit int: number of files */
/*struct entry
{
	unsigned long offset;
	unsigned long compression_type;
	unsigned long uncompressed_size;
	unsigned long length;
	char *filename;
};*/

enum compression_type
{
	zstd = 2
};

int main(int argc, char **argv)
{
	char *ext, *p;
	char filename[261];
	FILE *archive, *out;
	unsigned char buf[256];
	unsigned int i, j, pos;
	unsigned int compression_type;
	unsigned int uncompressed_size;
	unsigned int count, offset, length;
	unsigned char header[] = {0x37,0x6a,0x00,0x00};

	archive = fopen(argv[1], "rb");

	fread(buf, 1, 8, archive);

	if(memcmp(buf, header, 4))
	{
		puts("not a g archive");
		exit(1);
	}

	count = read_uint32_le(&buf[4]);

	printf("number of files in archive: %u\n", count);

	for(i = 0; i < count; i++)
	{
		memset(buf, 0, 256);
		j = 0;
		while(1)
		{
			buf[j] = fgetc(archive);
			if(!memcmp(&buf[j], "\x00", 1))
				break;
			j++;
		}

		memcpy(filename, buf, j);
		filename[j] = '\0';

		fread(buf, 1, 16, archive);
		offset = read_uint32_le(&buf[0]);
		compression_type = read_uint32_le(&buf[4]);
		uncompressed_size = read_uint32_le(&buf[8]);
		length = read_uint32_le(&buf[12]);

		if(strchr(filename, '/'))
		{
			for(p = filename; *p; p++)
			{
				if(*p == '/')
				{
					*p = '\0';

					if(mkdir(filename, 0755))
					{
						if(errno != EEXIST)
							puts("failed to create directory");
					}

					*p = '/';
				}
			}
		}

		/* Zstandard compressed file */
		if(compression_type == 2)
		{
			ext = strrchr(filename, '\0');
			ext[0] = '.';
			ext[1] = 'z';
			ext[2] = 's';
			ext[3] = 't';
			ext[4] = '\0';
		}
		/* no compression */
		else if(compression_type == 0)
		{/*NOP*/}
		else
		{
			printf("unknown compression type: %u\n", compression_type);
			exit(2);
		}

        printf("%s\n", filename);
        printf("offset: %u compression type: %u length: %u uncompressed size: %u\n", \
			offset, compression_type, length, uncompressed_size);

		pos = ftell(archive);
		fseek(archive, offset, SEEK_SET);
		out = fopen(filename, "wb");
		while(length--)
			fputc(fgetc(archive), out);

		fclose(out);
		fseek(archive, pos, SEEK_SET);
	}

	return 0;
}
