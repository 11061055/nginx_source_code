
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_CONNECT_H_INCLUDED_
#define _NGX_EVENT_CONNECT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_PEER_KEEPALIVE           1
#define NGX_PEER_NEXT                2
#define NGX_PEER_FAILED              4


typedef struct ngx_peer_connection_s  ngx_peer_connection_t;

typedef ngx_int_t (*ngx_event_get_peer_pt)(ngx_peer_connection_t *pc, // 当使用长连接，使用该方法获取一个连接
    void *data);
typedef void (*ngx_event_free_peer_pt)(ngx_peer_connection_t *pc, void *data, // 当使用长连接，使用该方法退还一个连接
    ngx_uint_t state);
#if (NGX_SSL)

typedef ngx_int_t (*ngx_event_set_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_save_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
#endif

// 主动发起连接时需要该结构
struct ngx_peer_connection_s {
    ngx_connection_t                *connection;

    struct sockaddr                 *sockaddr; // 远端服务地址
    socklen_t                        socklen;
    ngx_str_t                       *name; // 远端服务名称

    ngx_uint_t                       tries; // 连接失败后的重试次数
    ngx_msec_t                       start_time;

    ngx_event_get_peer_pt            get;
    ngx_event_free_peer_pt           free;
    void                            *data; // 上面 get free 函数 所需要的参数

#if (NGX_SSL)
    ngx_event_set_peer_session_pt    set_session;
    ngx_event_save_peer_session_pt   save_session;
#endif

    ngx_addr_t                      *local;

    int                              type;
    int                              rcvbuf;

    ngx_log_t                       *log;

    unsigned                         cached:1; // 为1时表示上面的connection已经缓存了

                                     /* ngx_connection_log_error_e */
    unsigned                         log_error:2;
};


ngx_int_t ngx_event_connect_peer(ngx_peer_connection_t *pc);
ngx_int_t ngx_event_get_peer(ngx_peer_connection_t *pc, void *data);


#endif /* _NGX_EVENT_CONNECT_H_INCLUDED_ */
