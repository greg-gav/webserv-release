# Import modules for CGI handling
import cgi, cgitb

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')


print ('Content-Type: text/html; charset=utf-8\r\n\r\n')

print ('<html>')
print ('<head>')
print ('<title>Data Form</title>')
print ('</head>')
print ('<body>')
print ('<h2>Plese review provided credentials:</h2>')
print ("<h2>Your name is %s %s. Thanks for your cooperation.</h2>" % (first_name, last_name))
print ('</body>')
print ('</html>')
