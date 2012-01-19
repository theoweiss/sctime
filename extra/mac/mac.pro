TARGET = patchqmake
SOURCES = patchqmake.c
DEPENDS = patchsource
# do *not* build an app_bundle
CONFIG -= app_bundle
QMAKE_POST_LINK = sh sctime-mac-dist

# original source is dead
#downloadsource.commands = wget -O patchqmake.c \"http://www.valleyhold.org/~gordons/Qt/patchqmake.c\" && \
#	md5 patchqmake.c | cut -d= -f2 | grep \"^ 1c1d00585ce7c663aa64c404430d7cd4\$\$\" >/dev/null && \
#	patch -p0 patchqmake.c < patchqmake.c-mac.patch || \
#	rm patchqmake.c
downloadsource.commands = rm -f patchqmake.c && \
	cp patchqmake.c.orig patchqmake.c && \
	patch patchqmake.c < patchqmake.c-mac.patch || \
	rm patchqmake.c
downloadsource.target = patchqmake.c
QMAKE_CLEAN += patchqmake.c
QMAKE_EXTRA_TARGETS += downloadsource
