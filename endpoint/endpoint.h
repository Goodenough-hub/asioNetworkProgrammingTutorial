#pragma once

// 用extern是防止重定义

extern int client_end_point(); // 客户端——创建端点
extern int server_end_point(); // 服务器端——创建端点
extern int create_tcp_socket(); // 客户端——创建socket
extern int create_acceptor_socket(); // 服务器端——创建socket
extern int bind_acceptor_socket(); // 服务器端——绑定本机ip+port
extern int connect_to_end(); // 客户端——连接操作
extern int dns_connect_to_end(); // 客户端——解析域名，再实现从客户端到服务器的连接
extern int accept_new_connection(); // 服务器端——接受一个连接

// const_buffer
extern void use_const_buffer();
extern void use_buffer_str();
extern void use_buffer_array(); // 数据是char类型的数据
