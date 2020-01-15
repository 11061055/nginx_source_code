
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init; // 不同用处有不同的实现 如 ngx_http_upstream_init_zone
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};

// 在main函数中调用ngx_init_cycle初始化
struct ngx_cycle_s {
    void                  ****conf_ctx; // 是一个数组，每个成员是指针，该指针指向另一个存着指针的数组,保存的是每个module的index到各个模块create_conf返回的指针的映射，参见ngx_init_cycle
    ngx_pool_t               *pool;

    ngx_log_t                *log; // main函数中初始化，由 ngx_log_init 返回
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files; // 所有连接的个数
    ngx_connection_t         *free_connections; // 可用连接池
    ngx_uint_t                free_connection_n; // 可用连接池的个数

    ngx_module_t            **modules; // 由main函数中调用ngx_init_cycle调用ngx_cycle_modules设置，保存所有模块
    ngx_uint_t                modules_n;
    ngx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    ngx_queue_t               reusable_connections_queue; // 可重复连接队列，元素类型是 ngx_connection_t

    ngx_array_t               listening; // 每个数组存着ngx_listening_t结构，中有监听端口及相关参数，可能是新建的，可能是从ngx_add_inherited_sockets继承的
    ngx_array_t               paths; // 所有要操作的目录
    ngx_array_t               config_dump;
    ngx_list_t                open_files; // 所有已经打开的文件 参见 ngx_conf_open_file
    ngx_list_t                shared_memory; // 每个元素是 ngx_shm_zone_t

    ngx_uint_t                connection_n; // 连接对象的总数，worker_connections指令设定，ngx_event_connections函数设置
    ngx_uint_t                files_n;

    ngx_connection_t         *connections; // 所有连接对象
    ngx_event_t              *read_events; // 每个连接关联一个读事件，在ngx_event_process_init中初始化
    ngx_event_t              *write_events;

    ngx_cycle_t              *old_cycle; // 旧的 ngx_cycle_t

    ngx_str_t                 conf_file; // -c 参数 nginx 配置文件路径 由main 函数调用 ngx_process_options 设置
    ngx_str_t                 conf_param; // -g 参数 全局配置参数 由main 函数调用 ngx_process_options 设置
    ngx_str_t                 conf_prefix; // main 中 ngx_process_options 初始化 保存 nginx 前缀
    ngx_str_t                 prefix; // main 中 ngx_process_options 初始化 保存 nginx 前缀
    ngx_str_t                 lock_file; // 进程间同步的文件锁名称
    ngx_str_t                 hostname;
};


typedef struct {
    ngx_flag_t                daemon;
    ngx_flag_t                master;

    ngx_msec_t                timer_resolution;

    ngx_int_t                 worker_processes;
    ngx_int_t                 debug_points;

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority; // 优先级

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;
    ngx_uid_t                 user;
    ngx_gid_t                 group;

    ngx_str_t                 working_directory;
    ngx_str_t                 lock_file;

    ngx_str_t                 pid;
    ngx_str_t                 oldpid;

    ngx_array_t               env;
    char                    **environment;
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
