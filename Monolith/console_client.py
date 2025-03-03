import string
import sys

print('console client for prefixes')
print('enter request')
for line in sys.stdin:
    print('request:' + line)
    if line == '\n':
	    print("terminating")
	    break
