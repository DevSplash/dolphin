/////////////////////////////////////////////////////////////////////////////
// Name:        src/osx/button_osx.cpp
// Purpose:     wxButton
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// RCS-ID:      $Id: button_osx.cpp 67280 2011-03-22 14:17:38Z DS $
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/button.h"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/toplevel.h"
    #include "wx/dcclient.h"
    #include "wx/stattext.h"
#endif

#include "wx/stockitem.h"

#include "wx/osx/private.h"

namespace
{

// Returns true only if the id is wxID_HELP and the label is "Help" or empty.
bool IsHelpButtonWithStandardLabel(wxWindowID id, const wxString& label)
{
    if ( id != wxID_HELP )
        return false;

    if ( label.empty() )
        return true;

    const wxString labelText = wxStaticText::GetLabelText(label);
    return labelText == "Help" || labelText == _("Help");
}

} // anonymous namespace

BEGIN_EVENT_TABLE(wxButton, wxControl)
    EVT_ENTER_WINDOW(wxButton::OnEnterWindow)
    EVT_LEAVE_WINDOW(wxButton::OnLeaveWindow)
END_EVENT_TABLE()

bool wxButton::Create(wxWindow *parent,
    wxWindowID id,
    const wxString& labelOrig,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name)
{
    DontCreatePeer();
    
    m_marginX =
    m_marginY = 0;

    // FIXME: this hack is needed because we're called from
    //        wxBitmapButton::Create() with this style and we currently use a
    //        different wxWidgetImpl method (CreateBitmapButton() rather than
    //        CreateButton()) for creating bitmap buttons, but we really ought
    //        to unify the creation of buttons of all kinds and then remove
    //        this check
    if ( style & wxBU_NOTEXT )
    {
        return wxControl::Create(parent, id, pos, size, style,
                                 validator, name);
    }

    wxString label;

    // Ignore the standard label for help buttons if possible, they use "?"
    // label under Mac which looks better.
    if ( !IsHelpButtonWithStandardLabel(id, labelOrig) )
    {
        label = labelOrig.empty() && wxIsStockID(id) ? wxGetStockLabel(id)
                                                     : labelOrig;
    }


    if ( !wxButtonBase::Create(parent, id, pos, size, style, validator, name) )
        return false;

    m_labelOrig =
    m_label = label ;

    SetPeer(wxWidgetImpl::CreateButton( this, parent, id, label, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    return true;
}

void wxButton::SetLabel(const wxString& label)
{
    if ( IsHelpButtonWithStandardLabel(GetId(), label) )
    {
        // ignore the standard label for the help buttons, it's not used
        return;
    }

    if ( HasFlag(wxBU_NOTEXT) )
    {
        // just store the label internally but don't really use it for the
        // button
        m_labelOrig =
        m_label = label;
        return;
    }

    wxButtonBase::SetLabel(label);
}

wxBitmap wxButton::DoGetBitmap(State which) const
{
    return m_bitmaps[which];
}

void wxButton::DoSetBitmap(const wxBitmap& bitmap, State which)
{
    m_bitmaps[which] = bitmap;

    if ( which == State_Normal )
        GetPeer()->SetBitmap(bitmap);
    else if ( which == State_Pressed )
    {
        wxButtonImpl* bi = dynamic_cast<wxButtonImpl*> (GetPeer());
        if ( bi )
            bi->SetPressedBitmap(bitmap);
    }
    InvalidateBestSize();
}

void wxButton::DoSetBitmapPosition(wxDirection dir)
{
    GetPeer()->SetBitmapPosition(dir);
    InvalidateBestSize();
}

#if wxUSE_MARKUP && wxOSX_USE_COCOA

bool wxButton::DoSetLabelMarkup(const wxString& markup)
{
    if ( !wxButtonBase::DoSetLabelMarkup(markup) )
        return false;

    GetPeer()->SetLabelMarkup(markup);

    return true;
}

#endif // wxUSE_MARKUP && wxOSX_USE_COCOA

wxWindow *wxButton::SetDefault()
{
    wxWindow *btnOldDefault = wxButtonBase::SetDefault();

    if ( btnOldDefault )
    {
        btnOldDefault->GetPeer()->SetDefaultButton( false );
    }

    GetPeer()->SetDefaultButton( true );

    return btnOldDefault;
}

void wxButton::Command (wxCommandEvent & WXUNUSED(event))
{
    GetPeer()->PerformClick() ;
    // ProcessCommand(event);
}

void wxButton::OnEnterWindow( wxMouseEvent& WXUNUSED(event))
{
    if ( DoGetBitmap( State_Current ).IsOk() )
        GetPeer()->SetBitmap( DoGetBitmap( State_Current ) );
}

void wxButton::OnLeaveWindow( wxMouseEvent& WXUNUSED(event))
{
    if ( DoGetBitmap( State_Current ).IsOk() )
        GetPeer()->SetBitmap( DoGetBitmap( State_Normal ) );
}

bool wxButton::OSXHandleClicked( double WXUNUSED(timestampsec) )
{
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, m_windowId);
    event.SetEventObject(this);
    ProcessCommand(event);
    return true;
}

//-------------------------------------------------------
// wxDisclosureTriangle
//-------------------------------------------------------

bool wxDisclosureTriangle::Create(wxWindow *parent, wxWindowID id, const wxString& label,
   const wxPoint& pos, const wxSize& size, long style,const wxValidator& validator, const wxString& name )
{    
    DontCreatePeer();
    if ( !wxControl::Create(parent, id, pos, size, style, validator, name) )
        return false;

    SetPeer(wxWidgetImpl::CreateDisclosureTriangle(this, parent, id, label, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );
    // passing the text in the param doesn't seem to work, so let's do it again
    SetLabel( label );

    return true;
}

void wxDisclosureTriangle::SetOpen( bool open )
{
    GetPeer()->SetValue( open ? 1 : 0 );
}

bool wxDisclosureTriangle::IsOpen() const
{
   return GetPeer()->GetValue() == 1;
}

bool wxDisclosureTriangle::OSXHandleClicked( double WXUNUSED(timestampsec) )
{
    // Just emit button event for now
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, m_windowId);
    event.SetEventObject(this);
    ProcessCommand(event);

    return true;
}

wxSize wxDisclosureTriangle::DoGetBestSize() const
{
    wxSize size = wxWindow::DoGetBestSize();

    // under Carbon the base class GetBestSize() implementation doesn't seem to
    // take the label into account at all, correct for it here
#if wxOSX_USE_CARBON
    size.x += GetTextExtent(GetLabel()).x;
#endif // wxOSX_USE_CARBON

    return size;
}

