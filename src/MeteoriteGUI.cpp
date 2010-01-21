///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 17 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "MeteoriteGUI.h"

///////////////////////////////////////////////////////////////////////////

MeteoriteGUI::MeteoriteGUI( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
	m_gauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge->Hide();
	
	bSizerMain->Add( m_gauge, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText = new wxStaticText( this, wxID_ANY, wxT("Drag and Drop Files"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText->Wrap( -1 );
	bSizerMain->Add( m_staticText, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bitmap_logo = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMain->Add( m_bitmap_logo, 0, wxALL, 5 );
	
	this->SetSizer( bSizerMain );
	this->Layout();
	bSizerMain->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MeteoriteGUI::OnClose ) );
	this->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ) );
	m_gauge->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
	m_staticText->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
	m_bitmap_logo->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
}

MeteoriteGUI::~MeteoriteGUI()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MeteoriteGUI::OnClose ) );
	this->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ) );
	m_gauge->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
	m_staticText->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
	m_bitmap_logo->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MeteoriteGUI::OnMouseRight ), NULL, this );
}
