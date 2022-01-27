#include <stdint.h>
#define MAX_BLK_SIZE                256
#define MIN_BLK_SIZE                16
#define MAX_LVLS                    5
#define BLK_STRUCT_SIZE             sizeof(block)

struct entry {
    struct entry  *next;
    uint32_t       size;
    uint8_t        level;
};
typedef struct entry block;

int mm_available;
void *memory_start;
void _init_allocator();

uint32_t _level_from_size(uint32_t size);
uint32_t _blocks_at_level(uint32_t level);
uint32_t _block_size_at_level(uint32_t level);

block   *free_lists[MAX_LVLS];
uint32_t block_sizes[MAX_LVLS];

/* API */
void *alloc(uint32_t size);
void free_blk(void *blk);
