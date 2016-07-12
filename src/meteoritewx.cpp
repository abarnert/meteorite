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

#include "meteoritewx.h"
using namespace std;


MeteoriteWx::MeteoriteWx( wxGauge *WxGauge_ ){
	WxGauge = WxGauge_;
	}
void MeteoriteWx::SetGauge( wxGauge *WxGauge_){
	WxGauge = WxGauge_;
}

bool MeteoriteWx::update_gauge( int percent ){	//return indicate program live, false on kill req
	if(WxGauge){
		if( WxGauge->GetValue() != percent ){		//Checks value is changed or not
			wxMutexGuiEnter();
			WxGauge->SetValue( percent );
			wxYield();
			wxMutexGuiLeave();
			}
		}
	else{
//		wxString value;
//		wxGetEnv( wxT("COLUMNS"), &value);
//		std::cout << "\r" << value.ToAscii() << " "  << target_file.ToAscii() << "\t\%" << percent;
		}

	if( !wxThread::This()->IsMain() )							//Checks if functions is running on thread
		if( wxThread::This()->TestDestroy() ){		//Checks if thread has termination request
//			close_files(true);				//Releases files
//			infile.close();
//			outfile.close();
//			MemoLogWriter(_("Operation stoped by user.\n"));
			return false;
			}
	return true;
	}

// NOTE: I believe this requires wx 2.8+ or 3.0+ (and will do a
// wasteful checking conversion from ASCII to UTF-16 on Windows,
// or to UTF-32 and back to UTF-8 on Mac and Linux, or something
// like that. But it works.
void MeteoriteWx::alert(string msg, string title) {
  wxMessageBox(msg, title, wxOK|wxCENTER);
}
