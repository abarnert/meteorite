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

#include "meteorite.h"

class MeteoriteCli : public Meteorite {
public:
  MeteoriteCli(bool interactive=false);
private:
  bool interactive_;
protected:
  virtual bool update_gauge(int percent);
  virtual void alert(string msg, string title);  
};
