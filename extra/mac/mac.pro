TARGET = patchqmake
SOURCES = patchqmake.c
# do *not* build an app_bundle
CONFIG -= app_bundle

# run the distribution preparation script after linking patchqmake
QMAKE_POST_LINK = sh sctime-mac-dist

# rebuild this if scTime binary in source directory changes
POST_TARGETDEPS = ../../src/scTime.app/Contents/MacOS/scTime

# generate the source for patchqmake.c by (possibly) downloading and patching
# it
# original source is dead, resort to local copy
#downloadsource.commands = wget -O patchqmake.c \"http://www.valleyhold.org/~gordons/Qt/patchqmake.c\" && \
#	md5 patchqmake.c | cut -d= -f2 | grep \"^ 1c1d00585ce7c663aa64c404430d7cd4\$\$\" >/dev/null && \
#	patch -p0 patchqmake.c < patchqmake.c-mac.patch || \
#	rm patchqmake.c
downloadsource.commands = rm -f patchqmake.c && \
	cp patchqmake.c.orig patchqmake.c && \
	patch patchqmake.c < patchqmake.c-mac.patch || \
	rm patchqmake.c
downloadsource.target = patchqmake.c
QMAKE_EXTRA_TARGETS += downloadsource

# throw away generated patchqmake.c on make clean
QMAKE_CLEAN += patchqmake.c
