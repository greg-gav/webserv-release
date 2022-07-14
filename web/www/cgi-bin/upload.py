#!/usr/bin/python3
import cgi, os
import cgitb; cgitb.enable()

form = cgi.FieldStorage()
# Get filename here.
fileitem = form['filename']
# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   fn = os.path.basename(fileitem.filename.replace("\\", "/" ))
   dirpath = os.environ.get('UPLOAD_DIR')
   if not dirpath:
      dirpath = "./uploads/"
   try:
      open(dirpath + fn, 'wb').write(fileitem.file.read())
      message = 'The file "' + fn + '" was uploaded successfully'
   except IOError:
      message = 'No file was uploaded'
else:
   message = 'No file was uploaded'
print ("""\
Content-Type: text/html; charset=utf-8\r\n\r\n
<html>
<body>
   <p>%s</p>
</body>
</html>
""" % (message,))
