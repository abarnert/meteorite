/***************************************************************
 * Name:      MeteoriteApp.cpp
 * Purpose:   Code for Application Class
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

#include "MeteoriteApp.h"
#include "MeteoriteMain.h"

IMPLEMENT_APP(MeteoriteApp);

bool MeteoriteApp::OnInit()
{
    wxImage::AddHandler(new wxPNGHandler);
    MeteoriteDialog* dlg = new MeteoriteDialog(0L);

    dlg->Show();
    return true;
}
