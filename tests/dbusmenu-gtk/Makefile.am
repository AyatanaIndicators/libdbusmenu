
check: tests

tests: mago

mago: dbusmenu.xml dbusmenu.py
	PYTHONPATH=$(builddir) mago -f $(builddir)/dbusmenu.xml -t $(builddir)/mago.results

dbusmenu.xml: dbusmenu.xml.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > $@

dbusmenu.py: dbusmenu.py.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > $@

EXTRA_DIST = \
	dbusmenu.xml.in \
	dbusmenu.py.in \
	dbusMenuTest \
	data/blank_label_2levels.json \
	data/blank_label.json \
	data/blank_submenus.json \
	data/dynamic.json \
	data/long_label.json \
	data/no_id.json \
	data/no_label.json \
	data/sameid_submenus_diff_sizes.json \
	data/sameid_submenus.json \
	data/sameid_top_and_submenus.json \
	data/sameid_topmenu.json \
	data/several_submenus.json \
	data/several_submenus_recursive.json \
	data/several_submenus_utf8.json \
	data/static.json \
	data/test-gtk-label.json

CLEANFILES = \
	dbusmenu.xml \
	dbusmenu.pyc \
	dbusmenu.py

distclean-local:
	-rm -rf $(builddir)/mago.results

