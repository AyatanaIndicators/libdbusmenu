
check: tests

tests: mago

mago: dbusmenu.xml
	mago -f $(builddir)/dbusmenu.xml

dbusmenu.xml: dbusmenu.xml.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > $@

EXTRA_DIST = dbusmenu.xml

CLEANFILES = dbusmenu.xml

