#test_parser.py
#script to test the 505, 501 and
#400 Status Codes in the HTTP Server

import socket
import re
import time
import pycurl
import threading
from generate_report import *

host = 'localhost'
port = '4000'
max_size = 50000

#function to create headers for 505
def test_505():
  data = "GET /index2.html HTTP/2.1\r\n\
          Accept:  */*\r\n\
          Accept-Encoding:  gzip, deflate, sdch\r\n\
          Accept-Language:  en-US,en;q=0.8\r\n\
          Connection:  keep-alive\r\n\
          Referer:  http://" + host + ":" + port + "/\r\n\
          Host:  " + host + ":" + port + "\r\n\
          User-Agent:  Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 \
          (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36\r\n\
          Upgrade-Insecure-Request: 1\
          url: /\r\n\
          Cache-Control:  no-cache\r\n\
          Pragma:  no-cache\r\n\
          method: GET\r\n\r\n"
  return data

#function to create headers for 501 
def test_501():
  data = "CONNECT /index2.html HTTP/1.1\r\n\
          Accept:  */*\r\n\
          Accept-Encoding:  gzip, deflate, sdch\r\n\
          Accept-Language:  en-US,en;q=0.8\r\n\
          Connection:  keep-alive\r\n\
          Referer:  http://" + host + ":" + port + "/\r\n\
          Host:  " + host + ":" + port + "\r\n\
          User-Agent:  Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 \
          (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36\r\n\
          Upgrade-Insecure-Request: 1\
          url: /\r\n\
          Cache-Control:  no-cache\r\n\
          Pragma:  no-cache\r\n\
          method: GET\r\n\r\n"
  return data

#function to create headers for 400 Bad Request
def test_400():
  data = "GET /index2.htmlHTTP/1.1\r\n\
          Accept:  */*\r\n\
          Accept-Encoding:  gzip, deflate, sdch\r\n\
          Accept-Language:  en-US,en;q=0.8\r\n\
          Connection:  keep-alive\r\n\
          Referer:  http://" + host + ":" + port + "/\r\n\
          Host:  " + host + ":" + port + "\r\n\
          User-Agent:  Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 \
          (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36\r\n\
          Upgrade-Insecure-Request: 1\
          url: /\r\n\
          Cache-Control:  no-cache\r\n\
          Pragma:  no-cache\r\n\
          method: GET\r\n\r\n"
  return data

#function to create a Socket
def create_socket():
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)         
  return s

#Test Case for 505
def test_server_HTTP_505():
  st = time.time()
  s = create_socket()
  s.connect((host, int(port)))
  s.sendall(test_505())
  time.sleep(1)
  status_code = s.recv(max_size).split('\r\n')[0].split(" ")[1]
  s.close()

  elasped_time = time.time() - st
  if status_code == '505':
    generate_report(threading.currentThread().getName(), "505", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "505", elasped_time, "FALSE")

#Test Case for 501
def test_server_HTTP_501():
  st = time.time()
  s = create_socket()
  s.connect((host, int(port)))
  s.sendall(test_501())
  time.sleep(1)
  status_code = s.recv(max_size).split('\r\n')[0].split(" ")[1]
  s.close()

  elasped_time = time.time() - st
  if status_code == '501':
    generate_report(threading.currentThread().getName(), "501", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "501", elasped_time, "FALSE")

#Test Case For 400
def test_server_HTTP_400():
  st = time.time() 
  s = create_socket()
  s.connect((host, int(port)))
  s.sendall(test_400())
  time.sleep(1)
  status_code = s.recv(max_size).split('\r\n')[0].split(" ")[1]
  s.close()

  elasped_time = time.time() - st
  if status_code == '400':
    generate_report(threading.currentThread().getName(), "400", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "400", elasped_time, "FALSE")

