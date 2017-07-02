#test_OPTIONS_request.py
#script to test the OPTIONS HTTP
#request to the server

import socket
import pycurl
import re
import os
import threading
import time
from StringIO import StringIO
from get_headers import *
from generate_report import *


def test_OPTIONS_request():
  st = time.time()
  buffer = StringIO()
  c = pycurl.Curl()
  c.setopt(c.URL, 'localhost:4000/index2.html')
  c.setopt(c.WRITEFUNCTION, buffer.write)
  c.setopt(c.CUSTOMREQUEST, 'OPTIONS')
  # Set our header function.
  c.setopt(c.HEADERFUNCTION, header_function)
  c.perform()
  c.close()

  elapsed_time = time.time() - st
  #check if the operation was successfully
  if ('allow' in headers.keys()): 
    generate_report(threading.currentThread().getName(), "OPTIONS REQUEST", elapsed_time, "TRUE")
  else: 
    generate_report(threading.currentThread().getName(), "OPTIONS REQUEST", elapsed_time, "FALSE")


