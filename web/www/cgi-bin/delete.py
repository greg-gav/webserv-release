#install: python3 -m pip install requests
#usage: python3 delete.py

import requests

x = requests.delete('http://localhost:8080/cgi-bin/down')

print(x.text)