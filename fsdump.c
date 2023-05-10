#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#include <math.h>

#include "ext2_fs.h"

/* Exit Codes */
// EXIT_SUCCESS     0
#define EXIT_ARG    1
#define EXIT_ERR    2

#define BASE_OFFSET	1024			// beginning of super block
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1) * block_size)

/* Defined i_mode values */
#define EXT2_S_IRWXU	0x01C0	    // user access rights mask
#define EXT2_S_IRWXG	0x0038	    // group access rights mask
#define EXT2_S_IRWXO	0x0007	    // others access rights mask

/* Globals */
struct ext2_super_block sb;			// super block struct
struct ext2_group_desc* gds;		// group descriptors

static int fsfd = -1;      			// file system file descriptor
static unsigned int block_size = 0; // block size
static unsigned int n_groups = 0;	// number of groups
int* inode_bitmap;

/* Utility Functions */
void format_time(uint32_t timestamp, char* buf) {
	time_t epoch = timestamp;
	struct tm ts = *gmtime(&epoch);
	strftime(buf, 80, "%m/%d/%y %H:%M:%S", &ts);
}

void free_all() {
	if (gds != NULL) {
		free(gds);
	}
	if(inode_bitmap != NULL){
		free(inode_bitmap);
	}
	close(fsfd);
}

/* Error Reporting */
void print_error(const char *msg, int exit_code) {
	if (errno) {
		fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    } else {
		fprintf(stderr, "%s\n", msg);
    }

	free_all();
	exit(exit_code);
}

/* Analysis Functions */

// superblock summary
void scan_sb() {

	// Superblock located at byte offset 1024 from beginning
	if (pread(fsfd, &sb, sizeof(struct ext2_super_block), 1024) < 0) {
		print_error("Error reading superblock", EXIT_ERR);
	}

	if (sb.s_magic != EXT2_SUPER_MAGIC) {
		print_error("Error: not an ext2 file system", EXIT_ERR);
	}

	block_size = EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size,

	printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
			sb.s_blocks_count,
			sb.s_inodes_count,
			block_size,
			sb.s_inode_size,
			sb.s_blocks_per_group,
			sb.s_inodes_per_group,
			sb.s_first_ino);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("Error: invalid number of arguments\nUsage: lab3a IMG_FILE", EXIT_ARG);
    }

    // Open file system image for read
    const char *fs_img = argv[1];
    fsfd = open(fs_img, O_RDONLY);
    if (fsfd < 0) {
        print_error("Error opening file system", EXIT_ERR);
    }

	scan_sb();
	

	free_all();
    exit(EXIT_SUCCESS);
}