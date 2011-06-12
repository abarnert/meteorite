WXCONFIG = wx-config
CPP = `$(WXCONFIG) --cxx`
CXXFLAGS= `$(WXCONFIG) --cxxflags` -O2
LDFLAGS = `$(WXCONFIG) --libs`
RC = `$(WXCONFIG) --rescomp`
#RC = x86_64-w64-mingw32-windres --define WX_CPU_AMD64
RCFLAGS = `$(WXCONFIG) --cxxflags`
MSGFMT = msgfmt

SOURCES= src/MeteoriteApp.cpp\
			src/MeteoriteGUI.cpp\
			src/MeteoriteMain.cpp\
			src/meteorite.cpp
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(OBJECTS:.o=.d)
RESOURCES= resources/resource.rc
RESOURCE_OBJ=$(RESOURCES:.rc=.o)
EXECUTABLE=meteorite
EXECUTABLE_WIN=Meteorite.exe

DESTDIR		=
PREFIX		= $(DESTDIR)/usr
BINDIR	    = $(PREFIX)/bin
DATADIR	    = $(PREFIX)/share
LOCALEDIR   = $(DATADIR)/locale

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@

win: $(SOURCES) $(RESOURCES) $(EXECUTABLE_WIN)

$(EXECUTABLE_WIN): $(OBJECTS) $(RESOURCE_OBJ)
	$(CPP) $(OBJECTS) $(RESOURCE_OBJ) $(LDFLAGS) -static-libstdc++ -static-libgcc -o $@

%.o : %.rc
	$(RC) $(RCFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(CXXFLAGS) -c $< -o $@

install:
	install -D -m 755 $(EXECUTABLE) $(BINDIR)/$(EXECUTABLE)
	install -D -m 644 resources/$(EXECUTABLE).png $(DATADIR)/pixmaps/$(EXECUTABLE).png
	install -D -m 644 resources/$(EXECUTABLE).desktop $(DATADIR)/applications/$(EXECUTABLE).desktop

uninstall:
	rm $(BINDIR)/$(EXECUTABLE)
	rm $(DATADIR)/pixmaps/$(EXECUTABLE).png
	rm $(DATADIR)/applications/$(EXECUTABLE).desktop
	rm $(LOCALEDIR)/*/LC_MESSAGES/$(EXECUTABLE).mo

test:
	cat $(LANGUAGEDIRS)

clean:
	rm -f src/*.o
	rm -f resources/resource.o
	rm -f locale/*/$(EXECUTABLE).mo
	rm -f $(EXECUTABLE)
	rm -f $(EXECUTABLE_WIN)
	rm -rf $(EXECUTABLE).app

distclean: clean

mac: all
	mkdir -p $(EXECUTABLE).app/Contents
	mkdir -p $(EXECUTABLE).app/Contents/MacOS
	mkdir -p $(EXECUTABLE).app/Contents/Resources
	mv $(EXECUTABLE) $(EXECUTABLE).app/Contents/MacOS/
	cp resources/$(EXECUTABLE).icns $(EXECUTABLE).app/Contents/Resources/
	cp docs/* $(EXECUTABLE).app/Contents/Resources/
	echo "APPLDivF" > $(EXECUTABLE).app/Contents/PkgInfo
	echo "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
<dict>\n\
 	\t<key>CFBundleDevelopmentRegion</key>\n\
 	\t<string>English</string>\n\
\
  	\t<key>CFBundleExecutable</key>\n\
 	\t<string>$(EXECUTABLE)</string>\n\
\
	\t<key>CFBundleGetInfoString</key>\n\
	\t<string>$(EXECUTABLE) v0.11</string>\n\
\
	\t<key>CFBundleIconFile</key>\n\
	\t<string>$(EXECUTABLE).icns</string>\n\
\
  	\t<key>CFBundleIdentifier</key>\n\
 	\t<string>net.sourceforge.divfixpp</string>\n\
\
  	\t<key>CFBundleShortVersionString</key>\n\
 	\t<string>v0.11</string>\n\
\
  	\t<key>CFBundleInfoDictionaryVersion</key>\n\
 	\t<string>6.0</string>\n\
\
  	\t<key>CFBundleName</key>\n\
 	\t<string>$(EXECUTABLE)</string>\n\
\
  	\t<key>CFBundlePackageType</key>\n\
 	\t<string>APPL</string>\n\
\
  	\t<key>CFBundleSignature</key>\n\
 	\t<string>DivF</string>\n\
\
  	\t<key>CFBundleVersion</key>\n\
 	\t<string>1.0.0</string>\n\
\
	\t<key>DRURLs</key>\n\
	\t<string>http://divfixpp.sourceforge.net</string>\n\
\
	\t<key>NSMainNibFile</key>\n\
	\t<string>$(EXECUTABLE)</string>\n\
\
	\t<key>NSPrincipalClass</key>\n\
	\t<string>NSApplication</string>\n\
\
  	\t<key>NSHumanReadableCopyright</key>\n\
 	\t<string> (c) 2009-2011, Erdem U. Altinyurt</string>\n\
\
</dict>\n\
</plist>\n\n" > $(EXECUTABLE).app/Contents/Info.plist
