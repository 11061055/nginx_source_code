
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1) // 一次可从内存池获取最大的字节, 约4k, 页大小可以通过命令 getconf PAGESIZE 获取

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024) // nginx 内存池 默认大小16K

#define NGX_POOL_ALIGNMENT       16 // 内存池分配在16字节对齐上
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)
// ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))

typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler; // ngx_pool_cleanup_pt定义见上，主要用于释放空间data的
    void                 *data;
    ngx_pool_cleanup_t   *next; // 清除函数也组成链表
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {  // 大块内存管理，由alloc指向
    ngx_pool_large_t     *next;
    void                 *alloc;
};


typedef struct { // 内存池中的一块，end-last就是可分配内存，failed是本块上分配失败次数
    u_char               *last;
    u_char               *end;
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;


/*

src/core/ngx_buf.h

typedef struct ngx_chain_s       ngx_chain_t;

struct ngx_chain_s {
    ngx_buf_t    *buf;
    ngx_chain_t  *next;
};
*/

struct ngx_pool_s { // 内存
    ngx_pool_data_t       d;
    size_t                max; // 一次最大分配的大小，受NGX_MAX_ALLOC_FROM_POOL限制
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    ngx_pool_large_t     *large;  // 大块内存直接malloc分配
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};

// 所以一块内存由如下 组成 [[[last end next failed] max current chain large cleanup log]...........]组成，其中省略处由last end指向，表明本块可用内存
// 所以，内存块之间由ngx_pool_data_t结构组成列表，列表中可用内存由 end-last 指出，ngx_pool_s保存了本块内存的清除函数，和大块内存。
// 头部结构也是本块内存的组成部分。
typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t; // 用于文件cache的保存，一个ngx_pool_s的ngx_pool_cleanup_t的handler可能有若干个文件描述符释放函数


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
