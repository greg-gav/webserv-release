import os

print ('<html>')
print ('<head>')
print ('<title>Hello World - Python CGI</title>')
print ('</head>')
print ('<body>')
print ('<h2>Hello World! This is my first CGI program</h2>')
print ("<h3>Environment:</h3>")
for param in os.environ.keys():
   print ("%20s: %s<br>" % (param, os.environ[param]))
cwd = os.getcwd()
print('SCRIPT WORKING DIRECTORY: ', cwd)
print ('</body>')
print ('</html>')
