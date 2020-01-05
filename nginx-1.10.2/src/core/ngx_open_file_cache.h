
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_OPEN_FILE_CACHE_H_INCLUDED_
#define _NGX_OPEN_FILE_CACHE_H_INCLUDED_


#define NGX_OPEN_FILE_DIRECTIO_OFF  NGX_MAX_OFF_T_VALUE
ngx_open_file_cache_t
/*
Nginx中的Cache只是cache文件句柄，因为静态文件的发送，一般来说，Nginx都是尽量使用sendfile()来进行发送的。因此只需要cache文件句柄就足够了。
inotify 监控 Linux 文件系统事件，但是nginx并没有采用。
红黑树是为了方便查找，而队列是为了方便超时管理，队列串接的就是红黑树的叶子节点。
*/

/*
typedef struct stat              ngx_file_info_t;
typedef ino_t                    ngx_file_uniq_t;
*/

typedef struct {
    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq; // 唯一标志 一般取自 inode节点号
    time_t                   mtime; // 最后修改时间
    off_t                    size; // 文件大小
    off_t                    fs_size; // 文件所占块大小
    off_t                    directio; // 文件对应的directio大小值， 小于该值采用sendfile来发送， 大于该值采用aio来发送, 这样可以防止进程被阻塞
    size_t                   read_ahead; // 预读大小 如套接字选项 O_READAHEAD

    ngx_err_t                err;
    char                    *failed;

    time_t                   valid; // 文件的有效时间

    ngx_uint_t               min_uses; // open_file_cache指令中inactive参数时间内，文件的最少访问次数

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 test_dir:1;
    unsigned                 test_only:1;
    unsigned                 log:1;
    unsigned                 errors:1;
    unsigned                 events:1;

    unsigned                 is_dir:1; // 目录
    unsigned                 is_file:1; // 普通文件
    unsigned                 is_link:1; // 链接文件
    unsigned                 is_exec:1; // 可执行文件
    unsigned                 is_directio:1; // 是否开启dierectio
} ngx_open_file_info_t; // 用于nginx中描述一个打开的缓存文件


typedef struct ngx_cached_open_file_s  ngx_cached_open_file_t;

struct ngx_cached_open_file_s {
    ngx_rbtree_node_t        node; // 用于表示该ngx_cached_open_file_s结构在nginx静态缓存红黑树的哪一个节点上
    ngx_queue_t              queue; // 用于表示该ngx_cached_open_file_s结构所在的队列

    u_char                  *name;
    time_t                   created; // 用于指明该文件是最先是在什么时候被放入静态缓存的
    time_t                   accessed; // 用于指示该缓存文件最近访问时间

    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq; //  该缓存文件的inode节点号，用作唯一标识
    time_t                   mtime; // 文件的最近更新时间
    off_t                    size;
    ngx_err_t                err;

    uint32_t                 uses;

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 count:24; // 是文件的引用计数，表示现在文件被几个请求使用中
    unsigned                 close:1; // 用于指示是否需要真正被关闭
    unsigned                 use_event:1; // 指示是否使用event

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;

    ngx_event_t             *event; // 该打开的缓存文件所关联的事件
};


typedef struct {
    ngx_rbtree_t             rbtree;
    ngx_rbtree_node_t        sentinel; // 红黑树sentinel节点
    ngx_queue_t              expire_queue; // 缓存文件过期队列

    ngx_uint_t               current;
    ngx_uint_t               max;
    time_t                   inactive;
} ngx_open_file_cache_t;


typedef struct {
    ngx_open_file_cache_t   *cache; // 用于指示当前要cleanup的cache
    ngx_cached_open_file_t  *file; // 用于指示当前要cleanup的文件
    ngx_uint_t               min_uses; // 低于此值则会cleanup
    ngx_log_t               *log;
} ngx_open_file_cache_cleanup_t;


typedef struct { // 指示nginx静态缓存所对应的事件

    /* ngx_connection_t stub to allow use c->fd as event ident */
    void                    *data;
    ngx_event_t             *read; // 读事件
    ngx_event_t             *write; // 写事件
    ngx_fd_t                 fd;

    ngx_cached_open_file_t  *file; // 所关联的静态缓冲文件
    ngx_open_file_cache_t   *cache; // 所关联的缓存
} ngx_open_file_cache_event_t;


ngx_open_file_cache_t *ngx_open_file_cache_init(ngx_pool_t *pool,
    ngx_uint_t max, time_t inactive);
ngx_int_t ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool);


#endif /* _NGX_OPEN_FILE_CACHE_H_INCLUDED_ */
