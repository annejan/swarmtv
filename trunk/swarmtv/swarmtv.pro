TEMPLATE = subdirs
SUBDIRS = libswarmtv \ 
	shellfront 
win32{
SUBDIRS += winservice
}
