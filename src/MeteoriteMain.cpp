/***************************************************************
 * Name:      MeteoriteMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Erdem U. Altinyurt (spamjunkeater@gmail.com)
 * Created:   2009-11-30
 * Copyright: Erdem U. Altinyurt (meteorite.sf.net)
 * License:   GPL v2
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "MeteoriteMain.h"

BEGIN_EVENT_TABLE(MeteoriteDialog,wxDialog )
	EVT_MENU( wxID_ABOUT, MeteoriteDialog::OnAbout )
	EVT_MENU( wxID_EXIT, MeteoriteDialog::OnQuit )
END_EVENT_TABLE()

MeteoriteDialog::MeteoriteDialog(wxDialog *dlg)
    : MeteoriteGUI(dlg), Meteorite( WxGauge )
{
	SetDropTarget( this );
	wxMemoryInputStream png_stream( meteorite_logo, sizeof(meteorite_logo));
	wxBitmap the_Logo( wxImage(png_stream, wxBITMAP_TYPE_PNG)) ;
	m_bitmap_logo->SetBitmap( the_Logo );
	m_bitmap_logo->SetDropTarget( this );
	VersionCheck();

#ifdef __WXMSW__
	this->SetIcon(Meteorite_xpm);
#else
	wxIcon wxc;
	wxc.CopyFromBitmap(the_Logo );
	SetIcon(wxc);
#endif
	SetGauge( m_gauge );
	OnResize();
	Center();
}

MeteoriteDialog::~MeteoriteDialog()
{
	m_bitmap_logo->SetDropTarget( NULL );
	SetDropTarget( NULL );
}

void MeteoriteDialog::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void MeteoriteDialog::OnQuit(wxCommandEvent &event)
{
    Destroy();
}


 void MeteoriteDialog::OnAbout(wxCommandEvent &event){
	wxAboutDialogInfo AllAbout;

	wxMemoryInputStream png_stream( meteorite_logo, sizeof(meteorite_logo));
	wxBitmap the_Logo( wxImage(png_stream, wxBITMAP_TYPE_PNG)); std::cout << Clusterart();
	wxIcon wxc;
	wxc.CopyFromBitmap(the_Logo );
	AllAbout.SetIcon( wxc );
	AllAbout.SetName(_T("Meteorite"));
    AllAbout.SetVersion( _T(METEORITE_VERSION) );
    AllAbout.SetDescription(_("project_Meteorite is a video repair utility for MKV/Matroska files."));
    AllAbout.SetCopyright(_T("(C) 2009 Erdem U. Altinyurt"));

    AllAbout.SetWebSite( _T("http://meteorite.sourceforge.net"));

	AllAbout.SetLicense( _T("Meteorite is a video repair utility for MKV/Matroska files.\n"
             "Copyright (C) 2009  Erdem U. Altunyurt\n"
             "\n"
             "This program is free software; you can redistribute it and/or\n"
             "modify it under the terms of the GNU General Public License\n"
             "as published by the Free Software Foundation; either version 2"
             "of the License."
             "\n"
             "This program is distributed in the hope that it will be useful,\n"
             "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
             "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
             "GNU General Public License for more details.\n"
             "\n"
             "You should have received a copy of the GNU General Public License\n"
             "along with this program; if not, write to the Free Software\n"
             "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
             "\n"
             "home:  meteorite.sourceforge.net\n"
             "email: spamjunkeater@gmail.com\n")
             );
    wxAboutBox(AllAbout);
#ifdef __WXMSW__
    SetIcon( Meteorite_xpm );
#endif
	}

void MeteoriteDialog::OnMouseRight( wxMouseEvent& event ){
	wxMenu menu;
	menu.Append(wxID_ABOUT, _T("About"));
	menu.Append(wxID_EXIT, _T("Exit"));
    PopupMenu( &menu, event.GetPosition() );
	event.Skip(false);
	}

void MeteoriteDialog::OnResize(){
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	bSizerMain->Add( m_staticText, 0, wxALIGN_CENTER|wxALL, 5 );
	bSizerMain->Add( m_bitmap_logo, 0, wxALL, 5 );
	bSizerMain->Add( m_gauge, 0, wxALL|wxEXPAND, 5 );
	this->SetSizer( bSizerMain );
	this->Layout();
	bSizerMain->Fit( this );
	}

void *MeteoriteDialog::Entry(){
	wxMutexGuiEnter();
	OnResize();
	wxMutexGuiLeave();
	for( int i = 0 ; i < FilesToRepair.GetCount() ; i++){
		wxString input = FilesToRepair.Item(i);
		wxString output;
		if( not input.BeforeLast(wxFileName::GetPathSeparator()).IsEmpty() )
			output = input.BeforeLast(wxFileName::GetPathSeparator())+wxFileName::GetPathSeparator()+_T("Meteorite.")+input.AfterLast(wxFileName::GetPathSeparator());
		else
			output = _T("Meteorite.")+input.AfterLast(wxFileName::GetPathSeparator());
		Repair( (string)input.ToAscii()/*, (string)output.ToAscii()*/ );
		}
	FilesToRepair.Clear();
	wxMutexGuiEnter();
	m_gauge->Hide();
	OnResize();
	wxMutexGuiLeave();

	m_mutex.Unlock();
	}

bool MeteoriteDialog::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames){
	if( m_mutex.TryLock() == wxMUTEX_BUSY ){
		wxBell();
		return FALSE;
		}
	int nFiles = filenames.GetCount();
	for ( int n = 0; n < nFiles; n++ )
		if ( wxFile::Exists( filenames.Item(n).c_str() ))
			FilesToRepair.Add( filenames.Item(n) );
	wxThreadHelper::Create();
	m_gauge->Show();
	GetThread()->Run();
	return TRUE;
	}

void MeteoriteDialog::VersionCheck(){
	wxURL url( wxT("http://meteorite.sourceforge.net/version.php") );
	if (url.IsOk()){
		url.GetProtocol().SetTimeout(3);
		wxInputStream *in_stream = url.GetInputStream();
		if( in_stream == NULL || in_stream->GetSize() > 10 ){
			return;	//need for keep safe
			}
		char *bfr = new char[in_stream->GetSize()+1];
		for(unsigned i = 0 ; i < in_stream->GetSize()+1 ; i++ )
			bfr[i]=0;
		in_stream->Read(bfr, in_stream->GetSize());
		if( strcmp( bfr, METEORITE_VERSION ) > 0 ){
			wxString newver = wxString::FromAscii( bfr );
			wxMessageBox( wxString::Format( _("New Meteorite version %s is available!"), newver.c_str() ), wxT("New Version!"), wxICON_INFORMATION);
			Centre();
			Fit();
			wxBell();
			ShowModal();
			}
		}
	}
