dnl
dnl ICKLE_GNOME_INIT_HOOK (script-if-gnome-enabled, [failflag], [additional-inits])
dnl
dnl if failflag is "fail" then ICKLE_GNOME_INIT_HOOK will abort if gnomeConf.sh
dnl is not found. 
dnl

AC_DEFUN([ICKLE_GNOME_INIT_HOOK],[
	AC_SUBST(GNOME_LIBS)
	AC_SUBST(GNOMEUI_LIBS)
	AC_SUBST(GNOMEGNORBA_LIBS)
	AC_SUBST(GTKXMHTML_LIBS)
	AC_SUBST(ZVT_LIBS)
	AC_SUBST(GNOME_LIBDIR)
	AC_SUBST(GNOME_INCLUDEDIR)

	AC_ARG_WITH(gnome-includes,
	[  --with-gnome-includes   Specify location of GNOME headers],[
	CFLAGS="$CFLAGS -I$withval"
	])
	
	AC_ARG_WITH(gnome-libs,
	[  --with-gnome-libs       Specify location of GNOME libs],[
	LDFLAGS="$LDFLAGS -L$withval"
	gnome_prefix=$withval
	])

	AC_ARG_WITH(gnome,
	[  --with-gnome            Specify prefix for GNOME files],
		if test x$withval = xyes; then
	    		want_gnome=yes
	    		dnl Note that an empty true branch is not
			dnl valid sh syntax.
	    		ifelse([$1], [], :, [$1])
        	else
	    		if test "x$withval" = xno; then
	        		want_gnome=no
	    		else
	        		want_gnome=yes
	    			LDFLAGS="$LDFLAGS -L$withval/lib"
	    			CFLAGS="$CFLAGS -I$withval/include"
	    			gnome_prefix=$withval/lib
	    		fi
  		fi,
		want_gnome=yes)

	if test "x$want_gnome" = xyes; then

	    AC_PATH_PROG(GNOME_CONFIG,gnome-config,no)
	    if test "$GNOME_CONFIG" = "no"; then
	      want_gnome=no
	    else
	      AC_MSG_CHECKING(if $GNOME_CONFIG works)
	      if $GNOME_CONFIG --libs-only-l gnome >/dev/null 2>&1; then
	        AC_MSG_RESULT(yes)
	        GNOME_GNORBA_HOOK([],$2)
	        GNOME_LIBS="`$GNOME_CONFIG --libs-only-l gnome`"
	        GNOMEUI_LIBS="`$GNOME_CONFIG --libs-only-l gnomeui`"
	        GNOMEGNORBA_LIBS="`$GNOME_CONFIG --libs-only-l gnorba gnomeui`"
	        GTKXMHTML_LIBS="`$GNOME_CONFIG --libs-only-l gtkxmhtml`"
		ZVT_LIBS="`$GNOME_CONFIG --libs-only-l zvt`"
	        GNOME_LIBDIR="`$GNOME_CONFIG --libs-only-L gnorba gnomeui`"
	        GNOME_INCLUDEDIR="`$GNOME_CONFIG --cflags gnorba gnomeui`"
		if test -n "$3"; then
		    n="$3"
		    for i in $n; do
			AC_MSG_CHECKING(extra library \"$i\")
			case $i in 
			    applets)
				AC_SUBST(GNOME_APPLETS_LIBS)
				if $GNOME_CONFIG --libs-only-l applets > /dev/null 2>&1; then
				    GNOME_APPLETS_LIBS=`$GNOME_CONFIG --libs-only-l applets`
				    AC_MSG_RESULT($GNOME_APPLETS_LIBS)
				else
				    want_gnome=no
				    AC_MSG_RESULT(not available)
				fi;;
			    capplet)
				AC_SUBST(GNOME_CAPPLET_LIBS)
				GNOME_CAPPLET_LIBS=`$GNOME_CONFIG --libs-only-l capplet`
				AC_MSG_RESULT($GNOME_CAPPLET_LIBS);;
			    *)
				AC_MSG_RESULT(unknown library)
			esac
		    done
		fi
                $1
	      else
	        AC_MSG_RESULT(no)
	        AC_MSG_WARN(Your gnome-config script does not work!)
              fi
            fi

	    if test x$exec_prefix = xNONE; then
	        if test x$prefix = xNONE; then
		    gnome_prefix=$ac_default_prefix/lib
	        else
 		    gnome_prefix=$prefix/lib
	        fi
	    else
	        gnome_prefix=`eval echo \`echo $libdir\``
	    fi
	fi
])

dnl
dnl ICKLE_GNOME_INIT ([additional-inits])
dnl

AC_DEFUN([ICKLE_GNOME_INIT],[
	ICKLE_GNOME_INIT_HOOK([],nofail,$1)
])
