
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

// slab 分配器 专为对象分配 准备

typedef struct ngx_slab_page_s  ngx_slab_page_t;

//一页
//内存小于ngx_slab_exact_size时，prev低2位为11
//内存等于ngx_slab_exact_size，prev低2位为10
//内存大于ngx_slab_exact_size而小于等于ngx_slab_max_size，prev低2位为01
//内存大于ngx_slab_max_size，prev低2位为00
struct ngx_slab_page_s {
    uintptr_t         slab; // 保存该页的分配信息
    ngx_slab_page_t  *next;
    uintptr_t         prev;
};

// slab内存管理器，每一个slab内存池对应着一块连续的共享内存，头部结构 紧跟数据内存
typedef struct {
    ngx_shmtx_sh_t    lock;

    size_t            min_size; // 最小空间
    size_t            min_shift; // 最小空间对应的移位

    ngx_slab_page_t  *pages; //第一个页描述单元
    ngx_slab_page_t  *last; // 最后一个页描述单元
    ngx_slab_page_t   free; // 空闲页描述单元

    u_char           *start; // 指向数据内存开始处
    u_char           *end; // 指向数据内存结尾处

    ngx_shmtx_t       mutex;

    u_char           *log_ctx;
    u_char            zero;

    unsigned          log_nomem:1;

    void             *data;
    void             *addr; // 指向所属的ngx_shm_zone_t里的ngx_shm_t成员的addr成员，一般用于指示一段共享内存块的起始位置
} ngx_slab_pool_t;


void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);


#endif /* _NGX_SLAB_H_INCLUDED_ */
