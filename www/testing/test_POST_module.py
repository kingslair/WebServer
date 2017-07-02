#test_POST_request.py
#script to test the POST HTTP
#request to the HTTP Server

import pycurl
import re
import os
import time
import threading
from get_headers import *
from generate_report import *


host = 'localhost'
port = '4000'

def test_POST_request():
  st = time.time()
  c = pycurl.Curl()
  c.setopt(pycurl.URL, "localhost:4000/index2.html")
  c.setopt(pycurl.HTTPPOST, [('image.gif', (c.FORM_FILE, \
  './test_data/image.gif'))])
  c.perform()
  c.close()

  elasped_time = time.time() - st
  #make a GET request to the resource created
  p = pycurl.Curl()
  p.setopt(pycurl.URL, "localhost:4000/image.gif")
  p.setopt(c.HEADERFUNCTION, header_function)
  p.setopt (c.NOBODY, True)
  p.perform()
  p.close() 
  
  #check for the data consistency
  if os.stat('./test_data/image.gif').st_size == int(headers['content-length']):
    generate_report(threading.currentThread().getName(), "POST REQUEST", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "POST REQUEST", elasped_time, "FALSE")


