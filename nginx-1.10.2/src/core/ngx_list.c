
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size) // 申请链表，每个节点包含n个元素，每个元素最大大小为size
{
    ngx_list_t  *list;

    list = ngx_palloc(pool, sizeof(ngx_list_t)); // 申请表结构
    if (list == NULL) {
        return NULL;
    }

    if (ngx_list_init(list, pool, n, size) != NGX_OK) { // 申请链表，每个节点包含n个元素，每个元素最大大小为size
        return NULL;
    }

    return list;
}


void *
ngx_list_push(ngx_list_t *l) // 从连表中拿出一个 大小为 size 的 区域
{
    void             *elt;
    ngx_list_part_t  *last;

    last = l->last;

    if (last->nelts == l->nalloc) { // 如果链表尾部节点中的元素个数打到了最大值nalloc

        /* the last part is full, allocate a new list part */

        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t)); // 重新申请一个节点元素结构
        if (last == NULL) {
            return NULL;
        }

        last->elts = ngx_palloc(l->pool, l->nalloc * l->size);// 为该节点申请一片连续内存区域
        if (last->elts == NULL) {
            return NULL;
        }

        last->nelts = 0;
        last->next = NULL;

        l->last->next = last;
        l->last = last;
    }

    elt = (char *) last->elts + l->size * last->nelts; // 腾出一片大小为size的区域
    last->nelts++;

    return elt;
}
