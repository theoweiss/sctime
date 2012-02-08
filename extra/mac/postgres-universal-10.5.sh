# Configure the source tree
PREFIX="$HOME"/bin/postgresql-9.1
SDK=/Developer/SDKs/MacOSX10.5.sdk
COMMON_CFLAGS="-isysroot $SDK -mmacosx-version-min=10.5"
COMMON_LDFLAGS="-Wl,-syslibroot,$SDK -mmacosx-version-min=10.5"

die() {
	echo $@
	exit 1
}

echo "Configuring the Postgres source tree for i386"
CFLAGS="-arch i386 $COMMON_CFLAGS" LDFLAGS="$COMMON_LDFLAGS" ./configure --prefix=$PREFIX --with-openssl --without-perl --without-python --without-tcl --without-bonjour --without-pam --with-krb5 --without-libxml || die "Failed to configure Postgres for i386"
mv src/include/pg_config.h src/include/pg_config_i386.h
make distclean

echo "Configuring the Postgres source tree for x86_64"
CFLAGS="-arch x86_64 $COMMON_CFLAGS" LDFLAGS="$COMMON_LDFLAGS" ./configure --prefix=$PREFIX --with-openssl --without-perl --without-python --without-tcl --without-bonjour --without-pam --with-krb5 --without-libxml || die "Failed to configure Postgres for x86_64"
mv src/include/pg_config.h src/include/pg_config_x86_64.h
make distclean

echo "Configuring the Postgres source tree for ppc"
CFLAGS="-arch ppc $COMMON_CFLAGS" LDFLAGS="$COMMON_LDFLAGS" ./configure --prefix=$PREFIX --with-openssl --without-perl --without-python --without-tcl --without-bonjour --without-pam --with-krb5 --without-libxml || die "Failed to configure Postgres for ppc"
mv src/include/pg_config.h src/include/pg_config_ppc.h
make distclean

echo "Configuring the Postgres source tree for Universal"
CFLAGS="-arch ppc -arch i386 -arch x86_64 $COMMON_CFLAGS" LDFLAGS="$COMMON_LDFLAGS" ./configure --prefix=$PREFIX --with-openssl --without-perl --without-python --without-tcl --without-bonjour --without-pam --with-krb5 --without-libxml || die "Failed to configure Postgres for Universal"

# Create a replacement pg_config.h that will pull in the appropriate architecture-specific one:
echo "#ifdef __BIG_ENDIAN__" > src/include/pg_config.h
echo "#include \"pg_config_ppc.h\"" >> src/include/pg_config.h
echo "#elif __x86_64__" >> src/include/pg_config.h
echo "#include \"pg_config_x86_64.h\"" >> src/include/pg_config.h
echo "#else" >> src/include/pg_config.h
echo "#include \"pg_config_i386.h\"" >> src/include/pg_config.h
echo "#endif" >> src/include/pg_config.h

# Fixup the makefiles
echo "Post-processing Makefiles for Universal Binary build"
find . -name Makefile -print -exec perl -p -i.backup -e 's/\Q$(LD) $(LDREL) $(LDOUT)\E (\S+) (.+)/\$(LD) -arch ppc \$(LDREL) \$(LDOUT) $1.ppc $2; \$(LD) -arch i386 \$(LDREL) \$(LDOUT) $1.i386 $2; \$(LD) -arch x86_64 \$(LDREL) \$(LDOUT) $1.x86_64 $2; lipo -create -output $1 $1.ppc $1.i386 $1.x86_64/' {} \; || die "Failed to post-process the Postgres Makefiles for Universal build"

echo "Building Postgres"
make -j 4 || die "Failed to build Postgres"
make install || die "Failed to install Postgres"
cp src/include/pg_config_i386.h $PREFIX/include/
cp src/include/pg_config_x86_64.h $PREFIX/include/
cp src/include/pg_config_ppc.h $PREFIX/include/
