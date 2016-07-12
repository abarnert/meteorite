/***********************************(GPL)********************************
*   Meteorite is an MKV/Matroska Video Repair Engine.                   *
*   Copyright (C) 2016  Andrew Barnert                                  *
*   Copyright (C) 2009  Erdem U. Altinyurt                              *
*                                                                       *
*   This program is free software; you can redistribute it and/or       *
*   modify it under the terms of the GNU General Public License         *
*   as published by the Free Software Foundation; either version 2      *
*   of the License.                                                     *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program;                                            *
*   if not, write to the Free Software Foundation, Inc.,                *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*              home : https://github.com/abarnert/meteorite             *
*************************************************************************/

#include "meteoritecli.h"
using namespace std;

#ifdef __MSWIN__
static const char *pathseps = "\\/";
#else
static const char *pathseps = "/";
#endif

static string prefixify(string path, string prefix) {
  size_t slash = path.find_last_of(pathseps);
  if (slash == string::npos) return prefix + path;
  return path.substr(0, slash+1) + prefix + path.substr(slash+1);
}

int main(int argc, char *argv[]) {
  // TODO: Real getopt_long-style interface
  if (argc == 1) {
    cerr << "Usage: " << argv[0] << " FILENAME [FILENAME]*\n";
    return 2;
  }
  MeteoriteCli cli;
  for (int arg = 1; arg != argc; ++arg) {
    string src = argv[arg];
    string dst = prefixify(src, "Meteorite.");
    cerr << src << " -> " << dst << "\n";
    cerr << argv[arg] << ": Starting\n";
    if (cli.Repair(src, dst)) {
      cerr << "************************************************************\n"
	   << "SUCCESS: " << src << " -> " << dst << "\n"
	   << "************************************************************\n"
	   << "\n";
    } else {
      cerr << "************************************************************\n"
	   << "FAILURE: " << src << " -> " << dst << "\n"
	   << "************************************************************\n"
	   << "\n";
    }
  }
}

MeteoriteCli::MeteoriteCli(bool interactive /*=false*/)
  : interactive_(interactive) {}

bool MeteoriteCli::update_gauge(int percent) {
  if (interactive_) {
    // TODO: Fancy progress spinner
    cout << percent << "% done\n";
  } else {
    cerr << percent << "% done\n";
  }
  return true;
}

void MeteoriteCli::alert(string msg, string title) {
  cout << "************************************************************\n"
       << title << "\n\n"
       << msg << "\n"
       << "************************************************************\n";
}
