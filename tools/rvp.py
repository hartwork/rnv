# $Id$

import sys, os, fcntl, string, re, xml.parsers.expat

global rvpin,rvpout,pat,errors,parser,text,mixed

class ProtocolError(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)
 
def launch():
	global rvpin,rvpout
	rvpin,rvpout=os.popen2('rvp '+string.join(sys.argv[1:],' '),0)

def send(s):
	rvpin.write(s.encode('UTF-8')+'\0')
	rvpin.flush()
	
def recv():
	s=''
	while 1:
		c=rvpout.read(1)
		if(c=='\0'): break
		s=s+c
	return s

def resp():
	global errors
	r=string.split(recv(),' ',3)
	if(r[0]=='ok'):
		return r[1]
	if(r[0]=='error'):
		errors=1
		if(r[3]!=''):
			sys.stderr.write(str(parser.lineno)+","+str(parser.offset)+": "+r[3])
		return r[1]
	raise ProtocolError,"unexpected response '"+r[0]+"'"

def start_tag_open(cur,name):
	send('start-tag-open '+cur+' '+name)
	return resp()

def attribute(cur,name,val):
	send('attribute '+cur+' '+name+' '+val)
	return resp()
	
def start_tag_close(cur,name):
	send('start-tag-close '+cur+' '+name)
	return resp()

def end_tag(cur,name):
	send('end-tag '+cur+' '+name)
	return resp()

def textonly(cur,text):
	send('text '+cur+' '+text)
	return resp()

# in mixed content, whitespace is simply discarded, and any
# non-whitespace is equal; but this optimization gives only only
# 5% increase in speed at most in practical cases
def mixed(cur,text):
	if(re.search('[^\t\n ]',text)):
		send('mixed '+cur+' .')
		return resp()
	else:
		return cur

def start(g):
	send('start '+g)
	return resp()

def quit():
	send('quit')
	return resp()

# Expat handlers
def flush_text():
	global mixed,pat,text
	
	if(pat==mixed):
		mixed(pat,text)
	else:
		textonly(pat,text)
	text=''

def start_element(name,attrs):
	global mixed,pat

	mixed=1
	flush_text()
	pat=start_tag_open(pat,name)
	mixed=0
	for n,v in attrs.items():
		pat=attribute(pat,n,v)
	pat=start_tag_close(pat,name)

def end_element(name):
	global mixed,pat

	flush_text()
	pat=end_tag(pat,name)
	mixed=1

def characters(data):
	global text
	
	text=text+data

# Main

errors=0
launch()
pat=start('0')

parser=xml.parsers.expat.ParserCreate('UTF-8',':')
parser.StartElementHandler=start_element
parser.EndElementHandler=end_element
parser.CharacterDataHandler=characters

text=''
parser.ParseFile(sys.stdin)

quit()
