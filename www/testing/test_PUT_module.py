#test_PUT_request.py
#script to the test the PUT method
#to the HTTP Server

import pycurl
import re
import os
import time
import threading
from get_headers import *
from generate_report import *

host = 'localhost'
port = '4000'

def test_PUT_request():
  st =time.time()
  c = pycurl.Curl()
  c.setopt(pycurl.URL, "localhost:4000/"+ threading.currentThread().getName() +"/working_4.jpeg")
  c.setopt(pycurl.HTTPPOST, [('working_4.jpeg', (c.FORM_FILE, \
  './test_data/working_4.jpeg'))])
  c.setopt(pycurl.CUSTOMREQUEST, "PUT")
  c.setopt(c.HEADERFUNCTION, header_function)
  c.perform()
  c.close()

  elasped_time = time.time() - st
  #make a GET request to the Server to fetch
  #the created resource
  p = pycurl.Curl()
  p.setopt(pycurl.URL, "localhost:4000/" + threading.currentThread().getName() +"/working_4.jpeg")
  p.setopt(c.HEADERFUNCTION, header_function)
  p.setopt (c.NOBODY, True)
  p.perform()
  p.close() 
  
  #check for data consistency
  if os.stat('./test_data/working_4.jpeg').st_size == int(headers['content-length']): 
    generate_report(threading.currentThread().getName(), "PUT REQUEST", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "PUT REQUEST", elasped_time, "FALSE")


