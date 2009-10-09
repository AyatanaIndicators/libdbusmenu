
check: tests

tests: mago

mago: dbusmenu.xml mago_tests/dbusmenu.py
	PYTHONPATH=$(builddir) mago -f dbusmenu.xml -t $(builddir)/mago.results

dbusmenu.xml: dbusmenu.xml.in
	sed -e "s|\@srcdir\@|$(srcdir)|" $< > mago_tests/$@

EXTRA_DIST = \
	dbusmenu.xml.in \
	dbusmenu.py.in \
	dbusMenuTest \
	mago_tests/data/blank_label_2levels.json \
	mago_tests/data/blank_label.json \
	mago_tests/data/blank_submenus.json \
	mago_tests/data/dynamic.json \
	mago_tests/data/long_label.json \
	mago_tests/data/no_id.json \
	mago_tests/data/no_label.json \
	mago_tests/data/sameid_submenus_diff_sizes.json \
	mago_tests/data/sameid_submenus.json \
	mago_tests/data/sameid_top_and_submenus.json \
	mago_tests/data/sameid_topmenu.json \
	mago_tests/data/several_submenus.json \
	mago_tests/data/several_submenus_recursive.json \
	mago_tests/data/several_submenus_utf8.json \
	mago_tests/data/static.json \
	mago_tests/data/test-gtk-label.json

CLEANFILES = \
	mago_tests/dbusmenu.xml \
	mago_tests/dbusmenu.pyc

distclean-local:
	-rm -rf $(builddir)/mago.results

