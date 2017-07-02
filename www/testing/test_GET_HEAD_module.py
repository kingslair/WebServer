#test_GET_HEAD_module
#script to test the GET and HEAD
#HTTP Requests

import pycurl
import socket
import time
import re
import os
import threading
from get_headers import *
from StringIO import StringIO
from generate_report import *

host = 'localhost'
port = '4000'
max_size = 50000

#function to create a HTTP Socket
def create_socket():
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)         
  return s

#Test HTTP GET Request
def test_GET_request():
  st = time.time()
  buffer = StringIO()
  p = pycurl.Curl()
  p.setopt(pycurl.URL, "localhost:4000/index2.html")
  p.setopt(p.HEADERFUNCTION, header_function)
  p.setopt(p.WRITEFUNCTION, buffer.write)
  p.setopt(p.NOBODY, True) 
  p.perform()
  
  elasped_time = time.time() - st
  #checking the data consistency
  if os.stat('./test_data/index2.html').st_size == int(headers['content-length']): 
    generate_report(threading.currentThread().getName(), "GET REQUEST", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "GET REQUEST", elasped_time, "FALSE")
  p.close()

#Test HTTP HEAD Request
def test_HEAD_request():
  st = time.time()
  c = pycurl.Curl()
  c.setopt(c.URL, 'localhost:4000/index2.html')
  # Set header function.
  c.setopt(c.HEADERFUNCTION, header_function)
  c.setopt (c.NOBODY, True)
  c.perform()
  c.close()
 
  elasped_time = time.time() - st
  #check the data consistency
  if os.stat('./test_data/index2.html').st_size == int(headers['content-length']):
    generate_report(threading.currentThread().getName(), "HEAD REQUEST", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "HEAD REQUEST", elapsed_time, "FALSE")
