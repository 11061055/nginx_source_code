
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILE_H_INCLUDED_
#define _NGX_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_file_s {
    ngx_fd_t                   fd; // 文件句柄
    ngx_str_t                  name; // 文件名称
    ngx_file_info_t            info; // struct stat

    off_t                      offset; // 当前要操作的文件指针偏移
    off_t                      sys_offset; // 当前该打开文件的实际指针偏移

    ngx_log_t                 *log;

#if (NGX_THREADS)
    ngx_int_t                (*thread_handler)(ngx_thread_task_t *task,
                                               ngx_file_t *file);
    void                      *thread_ctx;
    ngx_thread_task_t         *thread_task;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_event_aio_t           *aio;
#endif

    unsigned                   valid_info:1;
    unsigned                   directio:1;
};


#define NGX_MAX_PATH_LEVEL  3


typedef time_t (*ngx_path_manager_pt) (void *data);
typedef void (*ngx_path_loader_pt) (void *data);


typedef struct {
    ngx_str_t                  name; // 文件名称
    size_t                     len; // level中3个数据之和
    size_t                     level[3]; // 每层目录名的长度，最多三层

    ngx_path_manager_pt        manager; //文件缓冲管理回调
    ngx_path_loader_pt         loader; //文件缓冲loader回调
    void                      *data; // 回调上下文

    u_char                    *conf_file; //所关联的配置文件名称 ngx_conf_t->conf_file->file.name.data;
    ngx_uint_t                 line; // 所在配置文件的行数
} ngx_path_t;


typedef struct {
    ngx_str_t                  name;
    size_t                     level[3];
} ngx_path_init_t;


typedef struct {
    ngx_file_t                 file;
    off_t                      offset;
    ngx_path_t                *path;
    ngx_pool_t                *pool;
    char                      *warn;

    ngx_uint_t                 access;

    unsigned                   log_level:8;
    unsigned                   persistent:1;
    unsigned                   clean:1;
    unsigned                   thread_write:1;
} ngx_temp_file_t;


typedef struct {
    ngx_uint_t                 access; // 文件权限
    ngx_uint_t                 path_access; // 路径权限
    time_t                     time;
    ngx_fd_t                   fd;

    unsigned                   create_path:1; // 是否创建路径
    unsigned                   delete_file:1; // 重命名失败是否删除文件

    ngx_log_t                 *log;
} ngx_ext_rename_file_t;


typedef struct {
    off_t                      size; // 要拷贝的文件大小
    size_t                     buf_size;

    ngx_uint_t                 access;
    time_t                     time;

    ngx_log_t                 *log;
} ngx_copy_file_t;


typedef struct ngx_tree_ctx_s  ngx_tree_ctx_t;

typedef ngx_int_t (*ngx_tree_init_handler_pt) (void *ctx, void *prev);
typedef ngx_int_t (*ngx_tree_handler_pt) (ngx_tree_ctx_t *ctx, ngx_str_t *name);

// 这里是nginx遍历目录的相关数据结构
struct ngx_tree_ctx_s {
    off_t                      size; // 遍历到的文件的大小
    off_t                      fs_size; // 文件所占磁盘块数目乘以512的值与size中的最大值，即fs_size = ngx_max(size,st_blocks*512)
    ngx_uint_t                 access; // 文件权限
    time_t                     mtime; // 文件修改时间

    ngx_tree_init_handler_pt   init_handler;
    ngx_tree_handler_pt        file_handler; // 处理普通文件的回调函数
    ngx_tree_handler_pt        pre_tree_handler; // 进入一个目录前的回调函数
    ngx_tree_handler_pt        post_tree_handler;// 离开一个目录后的回调函数
    ngx_tree_handler_pt        spec_handler;// 处理特殊文件的回调函数，比如socket、FIFO等

    void                      *data; // data 是待分配的 数据结构
    size_t                     alloc; // 如果需要分配一些数据结构，在这里指定分配数据结构的大小，并由init_handler进行初始化，参见ngx_walk_tree

    ngx_log_t                 *log;
};


ngx_int_t ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix,
    ngx_str_t *name);

ssize_t ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain);
ngx_int_t ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path,
    ngx_pool_t *pool, ngx_uint_t persistent, ngx_uint_t clean,
    ngx_uint_t access);
void ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len);
ngx_int_t ngx_create_path(ngx_file_t *file, ngx_path_t *path);
ngx_err_t ngx_create_full_path(u_char *dir, ngx_uint_t access);
ngx_int_t ngx_add_path(ngx_conf_t *cf, ngx_path_t **slot);
ngx_int_t ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user);
ngx_int_t ngx_ext_rename_file(ngx_str_t *src, ngx_str_t *to,
    ngx_ext_rename_file_t *ext);
ngx_int_t ngx_copy_file(u_char *from, u_char *to, ngx_copy_file_t *cf);
ngx_int_t ngx_walk_tree(ngx_tree_ctx_t *ctx, ngx_str_t *tree);

ngx_atomic_uint_t ngx_next_temp_number(ngx_uint_t collision);

char *ngx_conf_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_merge_path_value(ngx_conf_t *cf, ngx_path_t **path,
    ngx_path_t *prev, ngx_path_init_t *init);
char *ngx_conf_set_access_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_atomic_t      *ngx_temp_number;
extern ngx_atomic_int_t   ngx_random_number;


#endif /* _NGX_FILE_H_INCLUDED_ */
