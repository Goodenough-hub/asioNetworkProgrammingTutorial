#pragma once

// ��extern�Ƿ�ֹ�ض���

extern int client_end_point(); // �ͻ��ˡ��������˵�
extern int server_end_point(); // �������ˡ��������˵�
extern int create_tcp_socket(); // �ͻ��ˡ�������socket
extern int create_acceptor_socket(); // �������ˡ�������socket
extern int bind_acceptor_socket(); // �������ˡ����󶨱���ip+port
extern int connect_to_end(); // �ͻ��ˡ������Ӳ���
extern int dns_connect_to_end(); // �ͻ��ˡ���������������ʵ�ִӿͻ��˵�������������
extern int accept_new_connection(); // �������ˡ�������һ������

// const_buffer
extern void use_const_buffer();
extern void use_buffer_str();
extern void use_buffer_array(); // ������char���͵�����
