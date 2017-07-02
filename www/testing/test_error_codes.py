#test_error_codes.py
#test scripts to check the error codes
#of the server

import pycurl
import re
import threading
import time
from StringIO import StringIO
from get_headers import *
from generate_report import *

#Test for 404 Not Found
def test_404():
  st = time.time()
  buffer = StringIO()
  p = pycurl.Curl()
  p.setopt(pycurl.URL, "localhost:4000/random.html")
  p.setopt(p.HEADERFUNCTION, header_function)
  p.setopt(p.WRITEFUNCTION, buffer.write)
  p.perform()

  elasped_time = time.time() - st
  if p.getinfo(p.RESPONSE_CODE) == 404:
    generate_report(threading.currentThread().getName(), "404", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "404", elasped_time, "FALSE")
  p.close()

#Test for 403 Forbidden
def test_403():
  st = time.time()
  buffer = StringIO()
  p = pycurl.Curl()
  #set the URL for the page
  p.setopt(pycurl.URL, "localhost:4000/haha.html")
  p.setopt(p.WRITEFUNCTION, buffer.write)
  p.perform()

  elasped_time = time.time() - st
  if p.getinfo(p.RESPONSE_CODE) == 403:
    generate_report(threading.currentThread().getName(), "403", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "403", elasped_time, "FALSE")
  p.close()

#Test for 308 Permanent Redirect
def test_307():
  st = time.time()
  c = pycurl.Curl()
  c.setopt(c.URL, 'localhost:4000/new_page.html')
  # Set header function.
  c.setopt(c.HEADERFUNCTION, header_function)
  c.setopt (c.NOBODY, True)
  c.perform()

  elasped_time = time.time() - st
  #check if the response from the server is
  #a 308 response
  if c.getinfo(c.RESPONSE_CODE) == 308:
    generate_report(threading.currentThread().getName(), "308", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "308", elasped_time, "FALSE")
  c.close()

#Test for 307 Temporary Redirect
def test_308():
  st = time.time()
  c = pycurl.Curl()
  c.setopt(c.URL, 'localhost:4000/old_page.html')
  # Set header function.
  c.setopt(c.HEADERFUNCTION, header_function)
  c.setopt (c.NOBODY, True)
  c.perform()

  elasped_time = time.time() - st
  #check if the response from the code is
  #a 307 response
  if c.getinfo(c.RESPONSE_CODE) == 307:
    generate_report(threading.currentThread().getName(), "307", elasped_time, "TRUE")
  else:
    generate_report(threading.currentThread().getName(), "307", elasped_time, "FALSE")
  c.close()
