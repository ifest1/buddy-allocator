#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buddy_allocator.h"

void _init_memory() {
    block *memory_start = (block *) ((void *) malloc(MAX_BLK_SIZE));
    mm_available = MAX_BLK_SIZE;
    memory_start->size = mm_available;
    free_lists[0] = (block *) memory_start;
}

uint32_t _level_from_size(uint32_t size) {
    uint32_t level;
    for (level = MAX_LVLS; _block_size_at_level(level) < size; level--);
    return level;
}

void _insert_freelist_head(block *blk, uint32_t level) {
    uint32_t size = _block_size_at_level(level);
    blk->next = free_lists[level];
    blk->size = size;
    free_lists[level] = blk;
}

block *_pop_freelist_head(uint32_t level) {
    block *blk;
    blk = free_lists[level];
    free_lists[level] = free_lists[level]->next;
    return blk;
}

void _delete_freelist_blk(block *blk, uint32_t level) {
    block *current = free_lists[level];

    if (current == blk) {
        free_lists[level] = free_lists[level]->next;
        return;
    }

    current = current->next;

    while (current) {
        if (current->next == blk)
            current->next = current->next->next;
        current = current->next;
    }
}

block *_find_freelist_blk(void *blk, uint32_t level) {
    block *current;
    current = free_lists[level];
    while (current) {
        if ((void *)current == blk)
            return current;
        current = current->next;
    }
    return NULL;
}

uint32_t _blocks_at_level(uint32_t level) {
    return (1 << level);
}

uint32_t _block_size_at_level(uint32_t level) {
    return (MAX_BLK_SIZE / _blocks_at_level(level));
}

void *_get_blk_buddy(block *blk) {
    block *buddy = ((block *) (((void *) blk) - sizeof(block) - blk->size));
    uint32_t level = _level_from_size(blk->size);
    buddy = _find_freelist_blk(buddy, level);
    if (buddy) {
        return buddy;
    }
    buddy = ((block *) (((void *) blk) + sizeof(block) + blk->size));
    return _find_freelist_blk(buddy, level);
}

void *alloc(uint32_t size) {
    uint32_t level;
    block *blk;

    printf("%d\n", mm_available);

    if (mm_available < size) {
        printf("Not enough space.\n");
        return NULL;
    }

    for (level = _level_from_size(size); free_lists[level] == NULL; level--);

    for (; _block_size_at_level(level) > size; level++) {
        block *blk = _pop_freelist_head(level);
        uint32_t next_lvl_size = _block_size_at_level(level + 1);
        block *buddy = (block *) (((void *) blk) + sizeof(block) + next_lvl_size);
        _insert_freelist_head(buddy, level + 1);
        _insert_freelist_head(blk, level + 1);
    }

    mm_available -= (size + sizeof(block));
    block *blk_test = _pop_freelist_head(level);
    void *chunk = (((void *) blk_test) + sizeof(block));
    return chunk;
}

void free_blk(void *chunk) {
    block *blk = (block *) (chunk - sizeof(block));
    uint32_t level = _level_from_size(blk->size);

    mm_available += (blk->size + sizeof(block));
    printf("%d\n", mm_available);

    while (level >= 0) {
        block *buddy = _get_blk_buddy(blk);
        if (!(buddy)) {
            return _insert_freelist_head(blk, level);
        }
        _delete_freelist_blk(buddy, level);
        blk->size = _block_size_at_level(--level);
    }
}

void debug_free_lists() {
    block *current;
    for (int i = 0; i < MAX_LVLS; i++) {
        printf("LEVEL: %d (%d) HEAD: (%p)\n", i, _block_size_at_level(i), free_lists[i]);
    }
    printf("\n");
}

int main() {
    _init_memory();
    void *blk1, *blk2, *blk3, *blk4, *blk5, *blk6;
    block *blk_test;
    blk1 = alloc(128);
    debug_free_lists();
    free_blk(blk1);
    debug_free_lists();
    blk2 = alloc(128);
    free_blk(blk2);
    debug_free_lists();
    blk3 = alloc(32);
    free_blk(blk3);
    debug_free_lists();
    blk4 = alloc(32);
    free_blk(blk3);
    debug_free_lists();
}
