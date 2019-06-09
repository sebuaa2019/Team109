#!/usr/bin/env python3
#接收数据
 
import socket
 
HOST = '127.0.0.1'  # 标准的回环地址 (localhost)
PORT = 65432        # 监听的端口 (非系统级的端口: 大于 1023)
 
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    contents = ['forwards','backwards','moveleft','moveright','turnleft','trueright','stop','mapping','save','kill']
    for content in contents:
        print(content)
        s.sendall(content.encode())


