#python_server.py 
#execution of test modules and cases start here

import pycurl
import re
import socket
import time
import threading
from test_parser import *
from test_GET_HEAD_module import *
from test_OPTIONS_module import *
from test_DELETE_module import *
from test_PUT_module import *
from test_POST_module import *
from test_error_codes import *


def generate():
  print "------------------------------------------------------------------------------------------------------------------------------"
  print '%10s' %'Thread', '%30s' %'Test Case', '%20s' %'Time Taken', '%30s' %'Status'
  print "------------------------------------------------------------------------------------------------------------------------------"

#Test Module Called by each indiviual thread
def test_module():

  #module test_parser
  test_server_HTTP_505()
  test_server_HTTP_501()
  test_server_HTTP_400()
  
  #module test_GET_HEAD_module
  test_GET_request()
  test_HEAD_request()
  
  #module test_OPTIONS_module
  test_OPTIONS_request()

  #module test_PUT_request
  test_PUT_request()
  
  #module test_POST_request
  test_POST_request()

  #module test_DELETE_request
  test_DELETE_request()

  #module test_error_codes
  test_404()
  test_403()
  test_307()
  test_308()

def create_test():
  #creating two threads for simentanous execution
  t1 = threading.Thread(target=test_module)
  t2 = threading.Thread(target=test_module) 
  #start the threads
  t1.start()
  t2.start()

if __name__ == "__main__":
 generate()
 create_test()
 while threading.active_count()-1:
  pass
 global passed_test
 global failed_test
 print "-------------------------------------------------------------------------------------------------------------------------------"
 print 'Passed Cases :%30d' % get_pass_count()
 print 'Failed Cases :%30d' % get_failed_count() 
 print 'Total Percentage of Test Cases Passed :%5d %%' % get_percentage()
 print 'Total Elasped Time of Test : %20f units' % get_time() 
 print "-------------------------------------------------------------------------------------------------------------------------------"
