
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text; // 以字符串存储IP地址

    int                 type; // 套接字类型，SOCK_STREAM表示是TCP套接字 通过选项SO_TYPE可获取

    int                 backlog; // backlog, ngx_set_inherited_sockets 中会设置
    int                 rcvbuf; // 接收缓冲区大小， ngx_set_inherited_sockets 中会设置，选项SO_RCVBUF可获取
    int                 sndbuf; // 发送缓冲区大小，ngx_set_inherited_sockets 中会设置，选项SO_SNDBUF可获取
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler; // TCP连接建立成功后的处理方法

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout; // TCP_DEFER_ACCEPT使得只有在握手完成并且收到数据后才向应用进程通知，当这个间隔超过post_accept_timeout后则丢弃连接

    ngx_listening_t    *previous; // 前一个ngx_listening_t，多个ngx_listening_t之间由链表构成
    ngx_connection_t   *connection;

    ngx_uint_t          worker;

    unsigned            open:1; // 为1时不会被ngx_cycle_t关闭
    unsigned            remain:1; // 为1时当使用新的ngx_cycle_t来初始化原有的ngx_cycle_t时，原有的套接字不会被关闭，利于平滑升级
    unsigned            ignore:1; // 跳过设置当前，0表示正常初始化，1表示跳过，如ngx_set_inherited_sockets从其它地方继承时设置为1

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */ // 在平滑升级的时候会用到
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)
    unsigned            reuseport:1; //如ngx_set_inherited_sockets从其它地方继承时设置
    unsigned            add_reuseport:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1; // 如ngx_set_inherited_sockets从其它地方继承时设置
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter; // 如ngx_set_inherited_sockets从其它地方继承时设置
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib; // 如ngx_set_inherited_sockets从其它地方继承时设置
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen; // 如ngx_set_inherited_sockets从其它地方继承时设置
#endif

};


typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_HTTP_V2_BUFFERED   0x02


struct ngx_connection_s { // 在ngx_event_accept中中初始化
    void               *data; // 不同处理框架中含义不一样，在HTTP中，指向ngx_http_request_t结构，在事件相关模块data指向下一个connection，，在ngx_event_process_init中初始化
    ngx_event_t        *read; // 关联的读事件，在ngx_event_process_init中初始化
    ngx_event_t        *write; // 关联的写事件，在ngx_event_process_init中初始化

    ngx_socket_t        fd;

    ngx_recv_pt         recv; // 接收数据的方法,ngx_event_accept中设置
    ngx_send_pt         send; // 发送数据的方法,ngx_event_accept中设置
    ngx_recv_chain_pt   recv_chain; // 以chain为参数接收数据,ngx_event_accept中设置
    ngx_send_chain_pt   send_chain; // 以chain为参数发送数据,ngx_event_accept中设置

    ngx_listening_t    *listening;// ngx_event_accept中设置

    off_t               sent; // 已发送的字节数

    ngx_log_t          *log; // ngx_event_accept中设置

    ngx_pool_t         *pool; // 连接池，在accept时建立，在释放连接时关闭。大小由listening对象中的pool_size决定

    int                 type;

    struct sockaddr    *sockaddr; // 连接客户端的结构
    socklen_t           socklen;
    ngx_str_t           addr_text;

    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr; // 本机监听的，即listen中对应的结构体
    socklen_t           local_socklen;

    ngx_buf_t          *buffer; // 缓存来自客户端的数据，大小由 client_header_buffer_size 决定

    ngx_queue_t         queue;

    ngx_atomic_uint_t   number; // 连接使用次数，每次收到客户端的请求，或者向后端服务器发起连接，都加1

    ngx_uint_t          requests; // 处理的请求次数

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            unexpected_eof:1;
    unsigned            timedout:1; // 连接已超时
    unsigned            error:1; // 连接出错
    unsigned            destroyed:1; // 连接已经销毁，ngx_connection_t 结构体还存在，但是套接字内存池等已不可用

    unsigned            idle:1; // 空闲 如keepalive中间状态
    unsigned            reusable:1; // 可重用
    unsigned            close:1; // 关闭
    unsigned            shared:1;

    unsigned            sendfile:1; // sendfile发送中
    unsigned            sndlowat:1; // 只有缓存中的数据打到一定阈值才发送，与ngx_handle_write_event中的lowat相对应
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */ // 禁用Nagle算法，小包也发送
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */ // 等到数据包到一定大小再转发

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS)
    ngx_thread_task_t  *sendfile_task;
#endif
};


#define ngx_set_connection_log(c, l)                                         \
                                                                             \
    c->log->file = l->file;                                                  \
    c->log->next = l->next;                                                  \
    c->log->writer = l->writer;                                              \
    c->log->wdata = l->wdata;                                                \
    if (!(c->log->log_level & NGX_LOG_DEBUG_CONNECTION)) {                   \
        c->log->log_level = l->log_level;                                    \
    }


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_clone_listening(ngx_conf_t *cf, ngx_listening_t *ls);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
void ngx_close_idle_connections(ngx_cycle_t *cycle);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
