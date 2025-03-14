//
// Created by Kaze Fx on 2023/11/24.999
//

#ifndef BOOTMEM_H
#define BOOTMEM_H

#include "list/CList.h"
#include "rbtree/CRbtree.h"
#include "types.h"

// page size, 4K in default
# define PAGE_SIZE 4
# define MAX_PAGES 1024
# define MINIMUM_SIZE 1

typedef struct page_s {
    // start of page
    void *page_start;
    // end of page
    void *page_end;
    // page_size Byte
    size_t page_byte_size;
    // page frame number
    uint pfn;
    // page merged
    struct page_s *first_page;
    // page is used
    bool used: 1;
    // if merged, is the end page
    bool end: 1;
    // used by whom
    bm_pid_t pid;
    // pid_used_page_list, NULL in default
    list_t pages;
    // mem_zone_list
    list_t mem_zone;
    // malloced_mem_tree
    rbtree_t addr_mem;
} page_t;

// page_list_node
typedef struct page_node_s {
    node_t list;
    page_t page;
} page_node_t;

// page_link_node
typedef struct page_link_node_s {
    node_t list;
    page_t *p_page;
} page_link_node_t;

// memory zone
typedef struct mem_zone_s {
    void *start;
    size_t size;
    bool used: 1;
} mem_zone_t;

// mem zone list node
typedef struct mem_zone_node_s {
    node_t list;
    mem_zone_t mem_zone;
} mem_zone_node_t;

// tree node
typedef struct pid_page_tree_node_s {
    rbtnode_t node;
    list_t page_list;
} pid_page_tree_node_t;

// addr tree node
typedef struct addr_mem_tree_node_s {
    rbtnode_t node;
    mem_zone_node_t *zone;
} addr_mem_tree_node_t;

// global manager
typedef struct mem_management_s {
    page_t *page_array;
    rbtree_t pid_page_tree;
    // true if used else false
    bool *bitmap;
} mem_management_t;

// search method
typedef enum {
    // first fit
    FF = 0,
    // next fit
    NF,
    // best fit
    BF,
    // worst fit
    WF
} strategy_t;

bool initPage(uint page_size, uint max_pages);

void setPID(uint _pid);

void setStrategy(strategy_t s);

#include "undef.h"

#endif //BOOTMEM_H
