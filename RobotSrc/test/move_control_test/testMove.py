#!/usr/bin/env python3
#接收数据
 
import socket
 
HOST = '127.0.0.1'  # 标准的回环地址 (localhost)
PORT = 65432        # 监听的端口 (非系统级的端口: 大于 1023)
 
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(1024)
            if not data:
                continue
            else:
                # print('Received', repr(data))
                c = data.decode()
                if c == 'w':
                    print('Received')
                data = conn.recv(1024)
                if not data:
                    c = ' '
                else:
                    c = data[0]

                if c == 'w':
                    now_x = 0.1
                    now_y = 0
                    mow_z = 0
                if c == 'a':
                    now_x = 0
                    now_y = 0.1
                    mow_z = 0
                if c == 's':
                    now_x = -0.1
                    now_y = 0
                    mow_z = 0
                if c == 'd':
                    now_x = 0
                    now_y = -0.1
                    mow_z = 0
                if c == 'l':
                    now_x = 0
                    now_y = 0
                    now_z = 0.25
                if c == 'r':
                    now_x = 0
                    now_y = 0
                    now_z = -0.25
                if c == 't':
                    now_x = 0
                    now_y = 0
                    now_z = 0