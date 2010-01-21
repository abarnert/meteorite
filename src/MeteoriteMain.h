/***************************************************************
 * Name:      MeteoriteMain.h
 * Purpose:   Defines Application Frame
 * Author:    Erdem U. Altinyurt (spamjunkeater@gmail.com)
 * Created:   2009-11-30
 * Copyright: Erdem U. Altinyurt (meteorite.sf.net)
 * License:   GPL v2
 **************************************************************/

#ifndef METEORITEMAIN_H
#define METEORITEMAIN_H

#include "MeteoriteApp.h"
#include "MeteoriteGUI.h"
#include "meteorite.h"
#include <wx/dnd.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/url.h>
#include <wx/msgdlg.h>
#include "../resources/Meteorite-Logo.hpp"
#include "../resources/Meteorite.xpm"

class MeteoriteDialog: public MeteoriteGUI, Meteorite, wxThreadHelper, wxFileDropTarget
{
    public:
        MeteoriteDialog(wxDialog *dlg);
        ~MeteoriteDialog();
    private:
        virtual void OnClose(wxCloseEvent& event);
        virtual void OnQuit(wxCommandEvent& event);
        virtual void OnAbout(wxCommandEvent& event);
        virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
		void *Entry();
		void OnResize();
		void OnMouseRight( wxMouseEvent& event );
		void VersionCheck();
		wxArrayString FilesToRepair;
		wxMutex m_mutex;
		DECLARE_EVENT_TABLE();
};

#endif // METEORITEMAIN_H

