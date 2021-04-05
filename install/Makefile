PACKAGER=./createPackage
PKGDIR=./
PKGFILE=dhufishcms-4.0.0.deb
INTERFACE=dhufishcms-4.0.0/var/lib/cms/install/interface.xml
SERVER=dhufishcms-4.0.0/usr/bin/cms
CGI=dhufishcms-4.0.0/usr/lib/cgi-bin/cms.cgi
DAV=dhufishcms-4.0.0/usr/lib/cgi-bin/dav.cgi

all:$(INTERFACE) $(PKGDIR)$(PKGFILE)

$(SERVER):
	cp ../src/cms $(SERVER)

$(CGI):
	cp ../src/cms.cgi $(CGI)

$(DAV):
	cp ../src/dav.cgi $(DAV)

$(PKGDIR)$(PKGFILE):$(INTERFACE) $(SERVER) $(CGI) $(DAV)
	@echo "Building debian install package..."
	fakeroot dpkg-deb --build dhufishcms-4.0.0

clean:
	@echo "Cleaning debian install package."
	rm -f $(PKGDIR)$(PKGFILE) $(CGI) $(DAV) $(SERVER) $(INTERFACE)

$(INTERFACE):
	$(PACKAGER) -i=system > $(INTERFACE)

