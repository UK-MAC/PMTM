
build:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERSION) SVN_URL=$(SVN_URL) all

openmp:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERSION) SVN_URL=$(SVN_URL) all WITH_OMP=1

serial:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERSION) SVN_URL=$(SVN_URL) all SERIAL=1

test:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERSION) SVN_URL=$(SVN_URL) test

testomp:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERSION) SVN_URL=$(SVN_URL) test WITH_OMP=1

docs:
	mkdir -p docs/usage_manual
	sed "s:thisdir:$(PWD):g" usage_manual.doxy > usage.doxy
	doxygen usage.doxy
	cp -R docs $(INSTDIR)/docs

clean:
	$(MAKE) -C src clean

cleanall:
	$(MAKE) -C src cleanall INSTALL_DIR=$(INSTDIR)

cleandir:
	$(MAKE) -C src cleandir

cleandocs:
	rm -rf docs usage.doxy

