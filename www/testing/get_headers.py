import pycurl
import re
try:
    from io import BytesIO
except ImportError:
    from StringIO import StringIO as BytesIO

headers = {}
def header_function(header_line):
    # Header lines include the first status line (HTTP/1.1)
    # split on multiple lines
    if ':' not in header_line:
        return

    # Break the header line into header name and value.
    name, value = header_line.split(':', 1)

    # Remove whitespace that may be present.
    name = name.strip()
    value = value.strip()

    # Lower the case 
    name = name.lower()

    # Record the header name and value.
    headers[name] = value


