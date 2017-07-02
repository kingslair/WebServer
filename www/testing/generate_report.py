passed_test = 0
failed_test = 0
total_test = 26
et_time = 0

def generate_report(thread_count, data, e_time, status):
  global passed_test
  global failed_test
  global et_time
  et_time += e_time
  if status == "TRUE":
    passed_test += 1
    print '%10s' %thread_count, '%30s' %data, '%20f' %e_time, '%30s' %'PASSED'
  else: 
    failed_test += 1
    print '%10s' %thread_count, '%30s' %data, '%20f' %e_time, '%30s' %'FAILED'
 

def get_percentage():
  if passed_test - failed_test == total_test:
    return 100
  elif passed_test - failed_test < total_test:
     return ((passed_test - failed_test)*100)/26 

def get_pass_count():
  return passed_test

def get_failed_count():
  return failed_test

def get_time():
  return et_time
