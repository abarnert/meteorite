#Meteorite Matroska/MKV Repair Engine

Meteorite is a tool to fix broken MKV files.

This is a fork of an abandoned (I believe) project from SourceForge.
See [Documentation from original website](#documentation-from-original-website)
below for full details.

* Version 0.20 Copyright (C) 2016 Andrew Barnert
* Version 0.12 Copyright (C) 2009 Erdem U. Altinyurt
* Licensed under GPL version 2.0 or later. See docs/GPL.txt for details.

#Build

Meteorite requires [wxWidgets][wx]. Tested with version 3.0 and 3.1,
installed via `brew install wxmac` and Windows installer. There may
be other prereqs; I honestly don't know yet.

  [wx]: https://www.wxwidgets.org/

* `make` should work to build an executable on any platform, which
will run from the source tree.
* `[sudo] make install` may work on some platforms, but I don't
know how far I'd trust it. (It does handle the usual `$DESTDIR`, 
`$PREFIX`, etc. customizations.) I definitely wouldn't use it on Mac.
* `make mac` will build a `.app` bundle, which can then be copied
into your applications directory. (However, at present, you'll
probably want to run it from a terminal anyway.)
* `make win` will build... hopefully something useful; not sure what.
* `contrib/meteorite.spec` can presumably be used for building an
RPM package.

#Use (including known issues)

Currently, there are no command-line argumetns.

On the Mac, this means you will probably want to run the app
from the terminal. If you're running out of the source tree,
that's of course just `./meteorite`. If you've build a `.app`
bundle, it'll be something like
`/Applications/meteorite.app/Contents/MacOS/meteorite`.

When the app is launched, it just presents a small window. Drag
a `.mkv` movie from Finder/Explorer/Nautilus/etc. to that window,
and it will begin creating a repaired version, in the same
directory, with `Meteorite.` prefixed to the name. So, for
example, if you fix `~/Movies/spam.mkv`, you will get
`~/Movies/Meteorite.spam.mkv`.

There's no useful GUI feedback, but there is a lot of debug 
information dumped to the terminal. Usually, the last piece
of information will be a dump of the final structure, which
should end with a line similar to this:

    |-Void: 00000000000000000000: size 5035

After the output has stopped, quit the app, to make sure the
file is flushed and closed, and now it should play. Note that,
at least on Mac, you cannot quit with Cmd-Q; just close the
window.

Sometimes, at completion, the app hangs. Killing it (`^C` from
the terminal, or `kill` (no `-9` needed), or your favorite GUI
tool seems to be safe—at least the file still ends up flushed.
I don't know whether this is related to a warning that
gets logged by `CoreAnimation` on the Mac about ending a thread
with an open transaction, but it certainly looks suspicious.

#Roadmap

* Version 0.20
  * Fork original Sourceforge repo.
  * Get the code compiling again.
  * Fix the startup crash.
  * Hopefully fix any other critical bugs.
* Version 0.30
 * Create a version that works as a command-line tool.
 * Fix hang on completion bug.
* Future
 * Make wx optional for building.
 * Add some feedback to the GUI.
 * Add some low-verbosity modes for the CLI.
 * Add options to both GUI and CLI.
 * Food in pill form.

#Original version information

Copyright (C) 2009  Erdem U. Altinyurt

The original project at [mkvrepair.com][mkvrepair] (and [sourceforge][sf])
seems to have stalled. The latest binary release, version 0.11 beta, crashes
on at least some platforms. The author checked in a fix and said version
0.12 would be coming soon, but the checked-in code won't compile. The last
update on SF was on 2012-06-27; the last update to the website says
"Please help to spread Meteorite for resuming development", and has a link
to donate.

  [mkvrepair]: http://www.mkvrepair.com/
  [sf]: https://sourceforge.net/p/meteorite

#Documentation from original website

Meteorite Project is [DivFix++][divfix] like program but for Matroska/MKV files.

It can repair your corrupted MKV video files to make it compatible with your player.

Also you can preview Matroska files those are already in download.

##High Definition Video

Why I made this program? Because I cannot watch files which are currently on download from p2p networks like emule,torrent.
Also, new videos, specially in HD, high definition videos are in MKV format generally.
You needed to fix them before watch. But unfortunatelly there was no program could fix matroska files.
So I make this tool, just for myself than released source and binary on my birthday...
You can repair your half downloaded MKV / Matroska videos or broken movies with it too.

  [divfix]: http://divfix.org/
