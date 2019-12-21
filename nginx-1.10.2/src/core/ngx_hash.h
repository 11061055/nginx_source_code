
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

// nginx 的 hash 桶 分配在连续的内存空间
// 各个hash桶中存的数据也是分配在一片连续内存空间
// 各个hash桶以NULL字符相隔

typedef struct {
    void             *value;    // 指向value
    u_short           len;      // key 的长度
    u_char            name[1];  // 指向key的第一个字节
} ngx_hash_elt_t; // 哈希元素


typedef struct {
    ngx_hash_elt_t  **buckets; // 哈希桶,二维数组,每个元素都是ngx_hash_elt_t,所有的哈希元素都分配在地址连续的内存空间上
    ngx_uint_t        size;
} ngx_hash_t;  // hash 桶已用个数为 size, hash->buckets[key % hash->size]用于定位bucket中某个ngx_hash_elt_t列表 尾部的 ngx_hash_elt_t 中value为null，以区别不同bucket


typedef struct {
    ngx_hash_t        hash;
    void             *value;
} ngx_hash_wildcard_t; // 带有通配符的散列表


typedef struct {
    ngx_str_t         key;
    ngx_uint_t        key_hash;
    void             *value;
} ngx_hash_key_t; // KV 中的 K 且 key_hash是根据K计算得到的数，用于快速比较，最终要把ngx_hash_key_t存入ngx_hash_elt_t当中


typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);


typedef struct {
    ngx_hash_t            hash;      // 精确散列表
    ngx_hash_wildcard_t  *wc_head;   // 前置通配符散列表
    ngx_hash_wildcard_t  *wc_tail;   // 后置通配符散列表
} ngx_hash_combined_t;


typedef struct {
    ngx_hash_t       *hash; // 保存hash桶
    ngx_hash_key_pt   key;  // hash函数

    ngx_uint_t        max_size; // 桶最大个数，实际个数在ngx_hash_t 的 size 中
    ngx_uint_t        bucket_size; // 每个bucket中的空间，即一个bucket中所有ngx_hash_elt_t的空间大小

    char             *name; // 哈希表名称
    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;
} ngx_hash_init_t;


#define NGX_HASH_SMALL            1
#define NGX_HASH_LARGE            2

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1
#define NGX_HASH_READONLY_KEY     2


typedef struct {
    ngx_uint_t        hsize;  // 桶个数

    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;

    ngx_array_t       keys;        // 所有全匹配的keys
    ngx_array_t      *keys_hash;    // 二维数组,第一个维度为bucket编号,每个元素keys_hash[i]都是一个数组

    ngx_array_t       dns_wc_head;
    ngx_array_t      *dns_wc_head_hash;

    ngx_array_t       dns_wc_tail;
    ngx_array_t      *dns_wc_tail_hash;
} ngx_hash_keys_arrays_t;


typedef struct {
    ngx_uint_t        hash;
    ngx_str_t         key;
    ngx_str_t         value;
    u_char           *lowcase_key;
} ngx_table_elt_t; // KV对


void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);

ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)
ngx_uint_t ngx_hash_key(u_char *data, size_t len);
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);


ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);


#endif /* _NGX_HASH_H_INCLUDED_ */
