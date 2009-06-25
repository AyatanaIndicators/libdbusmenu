
check: tests

tests: mago

mago: dbusmenu.xml dbusmenu.py
	PYTHONPATH=$(builddir) mago -f $(builddir)/dbusmenu.xml

dbusmenu.xml: dbusmenu.xml.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > $@

dbusmenu.py: dbusmenu.py.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > $@

EXTRA_DIST = \
	dbusmenu.xml \
	dbusmenu.xml.in \
	dbusmenu.py \
	dbusmenu.py.in

CLEANFILES = \
	dbusmenu.xml \
	dbusmenu.py

