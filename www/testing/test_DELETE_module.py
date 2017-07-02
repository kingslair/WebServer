#test_DELETE_request.py
#script to test the DELETE HTTP Request

import pycurl
import re
import os
import threading
import time
from StringIO import StringIO
from get_headers import *
from generate_report import *

def test_DELETE_request():
  st = time.time()
  buffer = StringIO()
  c = pycurl.Curl()
  c.setopt(c.URL, "localhost:4000/"+ threading.currentThread().getName() +"/working_4.jpeg")
  c.setopt(c.WRITEFUNCTION, buffer.write)
  c.setopt(c.CUSTOMREQUEST, 'DELETE')
  # Set header function.
  c.setopt(c.HEADERFUNCTION, header_function)
  c.perform()
  c.close()

  elapsed_time = time.time() - st
  #checking the data consistency
  if os.stat('./test_data/working_4.jpeg').st_size == int(headers['content-length']):
    generate_report(threading.currentThread().getName(), "DELETE REQUEST", elapsed_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "DELETE REQUEST", elapsed_time,"FALSE")
