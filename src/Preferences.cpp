// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "Preferences.h"

#include "Accelerators.h"
#include "dbus/mpris2.h"
#include "Images.h"
#include "MD5.h"
#include "MediaCtrl.h"
#include "Settings.h"
#include "TagInfo.h"
#include "Transcode.h"
#include "Utils.h"

#include <wx/statline.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/arrimpl.cpp>

#include <id3v1genres.h>

#define guPREFERENCES_LISTBOX_HEIGHT    110

WX_DEFINE_OBJARRAY( guCopyToPatternArray );

// -------------------------------------------------------------------------------- //
// guCopyToPattern
// -------------------------------------------------------------------------------- //
guCopyToPattern::guCopyToPattern()
{
    m_Format = guTRANSCODE_FORMAT_KEEP;
    m_Quality = guTRANSCODE_QUALITY_KEEP;
    m_MoveFiles = false;
}

// -------------------------------------------------------------------------------- //
guCopyToPattern::guCopyToPattern( const wxString &pattern )
{
    int Index;
    int Count;
    // Default:{g}/{a}/{b}/{n} - {a} - {t}:0:4:0
    m_Format = guTRANSCODE_FORMAT_KEEP;
    m_Quality = guTRANSCODE_QUALITY_KEEP;
    m_MoveFiles = false;
    wxArrayString Fields = wxStringTokenize( pattern, wxT( ":" ) );
    if( ( Count = Fields.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            switch( Index )
            {
                case 0 : m_Name    = unescape_configlist_str( Fields[ Index ] ); break;
                case 1 : m_Pattern = unescape_configlist_str( Fields[ Index ] ); break;
                case 2 : m_Format  = wxAtoi( Fields[ Index ] ); break;
                case 3 : m_Quality = wxAtoi( Fields[ Index ] ); break;
                case 4 : m_MoveFiles = wxAtoi( Fields[ Index ] ); break;
                case 5 : m_Path = unescape_configlist_str( Fields[ Index ] ); break;
                default :
                    return;
            }
        }
    }
}


// -------------------------------------------------------------------------------- //
guCopyToPattern::~guCopyToPattern()
{
}

// -------------------------------------------------------------------------------- //
wxString guCopyToPattern::ToString( void )
{
    return wxString::Format( wxT( "%s:%s:%i:%i:%i:%s" ),
        escape_configlist_str( m_Name ).c_str(), escape_configlist_str( m_Pattern ).c_str(),
        m_Format, m_Quality, m_MoveFiles, escape_configlist_str( m_Path ).c_str() );
}

#define PREFERENCES_SCROLL_STEP     20

// -------------------------------------------------------------------------------- //
// guPrefDialog
// -------------------------------------------------------------------------------- //
guPrefDialog::guPrefDialog( wxWindow* parent, guDbLibrary * db, int pagenum )
{
	wxBoxSizer *        MainSizer;

    m_Db = db;
    m_LinkSelected = wxNOT_FOUND;
    m_CmdSelected = wxNOT_FOUND;
    m_CopyToSelected = wxNOT_FOUND;
    m_LibPathsChanged = false;
    m_VisiblePanels = 0;
    m_CopyToOptions = NULL;
    m_LyricSearchEngine = NULL;
    m_LyricSourceSelected = wxNOT_FOUND;
    m_LibOptCopyToChoice = NULL;

    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );


    wxPoint WindowPos;
    WindowPos.x = m_Config->ReadNum( wxT( "PosX" ), -1, wxT( "preferences" ) );
    WindowPos.y = m_Config->ReadNum( wxT( "PosY" ), -1, wxT( "preferences" ) );
    wxSize WindowSize;
    WindowSize.x = m_Config->ReadNum( wxT( "Width" ), 600, wxT( "preferences" ) );
    WindowSize.y = m_Config->ReadNum( wxT( "Height" ), 530, wxT( "preferences" ) );

    //wxDialog( parent, wxID_ANY, _( "Songs Editor" ), wxDefaultPosition, wxSize( 625, 440 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    Create( parent, wxID_ANY, _( "Preferences" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

    //
    m_MainLangChoices.Add( _( "Default" ) );
    m_MainLangChoices.Add( _( "Czech" ) );
    m_MainLangChoices.Add( _( "Dutch" ) );
    m_MainLangChoices.Add( _( "English" ) );
    m_MainLangChoices.Add( _( "French" ) );
    m_MainLangChoices.Add( _( "German" ) );
    m_MainLangChoices.Add( _( "Greek" ) );
    m_MainLangChoices.Add( _( "Hungarian" ) );
    m_MainLangChoices.Add( _( "Icelandic" ) );
    m_MainLangChoices.Add( _( "Italian" ) );
    m_MainLangChoices.Add( _( "Japanese" ) );
    m_MainLangChoices.Add( _( "Norwegian Bokmal" ) );
    m_MainLangChoices.Add( _( "Polish" ) );
    m_MainLangChoices.Add( _( "Portuguese" ) );
    m_MainLangChoices.Add( _( "Portuguese-Brazilian" ) );
    m_MainLangChoices.Add( _( "Russian" ) );
    m_MainLangChoices.Add( _( "Slovak" ) );
    m_MainLangChoices.Add( _( "Spanish" ) );
    m_MainLangChoices.Add( _( "Swedish" ) );
    m_MainLangChoices.Add( _( "Thai" ) );
    m_MainLangChoices.Add( _( "Turkish" ) );
    m_MainLangChoices.Add( _( "Ukrainian" ) );
    m_MainLangChoices.Add( _( "Bulgarian" ) );
    m_MainLangChoices.Add( _( "Catalan" ) );
    m_MainLangChoices.Add( _( "Serbian" ) );

    m_MainLangCodes.Add( wxLANGUAGE_DEFAULT );
    m_MainLangCodes.Add( wxLANGUAGE_CZECH );
    m_MainLangCodes.Add( wxLANGUAGE_DUTCH );
    m_MainLangCodes.Add( wxLANGUAGE_ENGLISH );
    m_MainLangCodes.Add( wxLANGUAGE_FRENCH );
    m_MainLangCodes.Add( wxLANGUAGE_GERMAN );
    m_MainLangCodes.Add( wxLANGUAGE_GREEK );
    m_MainLangCodes.Add( wxLANGUAGE_HUNGARIAN );
    m_MainLangCodes.Add( wxLANGUAGE_ICELANDIC );
    m_MainLangCodes.Add( wxLANGUAGE_ITALIAN );
    m_MainLangCodes.Add( wxLANGUAGE_JAPANESE );
    m_MainLangCodes.Add( wxLANGUAGE_NORWEGIAN_BOKMAL );
    m_MainLangCodes.Add( wxLANGUAGE_POLISH );
    m_MainLangCodes.Add( wxLANGUAGE_PORTUGUESE );
    m_MainLangCodes.Add( wxLANGUAGE_PORTUGUESE_BRAZILIAN );
    m_MainLangCodes.Add( wxLANGUAGE_RUSSIAN );
    m_MainLangCodes.Add( wxLANGUAGE_SLOVAK );
    m_MainLangCodes.Add( wxLANGUAGE_SPANISH );
    m_MainLangCodes.Add( wxLANGUAGE_SWEDISH );
    m_MainLangCodes.Add( wxLANGUAGE_THAI );
    m_MainLangCodes.Add( wxLANGUAGE_TURKISH );
    m_MainLangCodes.Add( wxLANGUAGE_UKRAINIAN );
    m_MainLangCodes.Add( wxLANGUAGE_BULGARIAN );
    m_MainLangCodes.Add( wxLANGUAGE_CATALAN );
    m_MainLangCodes.Add( wxLANGUAGE_SERBIAN );

    m_LFMLangNames.Add( _( "Default" ) );        m_LFMLangIds.Add( wxEmptyString );
    m_LFMLangNames.Add( _( "English" ) );        m_LFMLangIds.Add( wxT( "en" ) );
    m_LFMLangNames.Add( _( "French" ) );         m_LFMLangIds.Add( wxT( "fr" ) );
    m_LFMLangNames.Add( _( "German" ) );         m_LFMLangIds.Add( wxT( "de" ) );
    m_LFMLangNames.Add( _( "Italian" ) );        m_LFMLangIds.Add( wxT( "it" ) );
    m_LFMLangNames.Add( _( "Portuguese" ) );     m_LFMLangIds.Add( wxT( "pt" ) );
    m_LFMLangNames.Add( _( "Spanish" ) );        m_LFMLangIds.Add( wxT( "es" ) );


	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_MainNotebook = new wxListbook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_LEFT );

    m_ImageList = new wxImageList( 32, 32 );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_general ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_library ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_playback ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_crossfader ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_record ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_last_fm ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_lyrics ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_online_services ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_podcasts ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_jamendo ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_magnatune ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_links ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_commands ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_copy_to ) );
    m_ImageList->Add( guImage( guIMAGE_INDEX_pref_accelerators ) );
    m_MainNotebook->AssignImageList( m_ImageList );


	m_GenPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_GenPanel, _("General"), true, 0 );
	m_GenPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_LibPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LibPanel, _("Collections"), false );
	m_MainNotebook->SetPageImage( 1, 1 );
	m_LibPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_PlayPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_PlayPanel, _( "Playback" ), false );
	m_MainNotebook->SetPageImage( 2, 2 );
	m_PlayPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_XFadePanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_XFadePanel, _( "Crossfader" ), false );
	m_MainNotebook->SetPageImage( 3, 3 );
	m_XFadePanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_RecordPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_RecordPanel, _( "Record" ), false );
	m_MainNotebook->SetPageImage( 4, 4 );
	m_RecordPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_LastFMPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LastFMPanel, _( "Audioscrobble" ), false );
	m_MainNotebook->SetPageImage( 5, 5 );
	m_LastFMPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_LyricsPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LyricsPanel, _( "Lyrics" ), false );
	m_MainNotebook->SetPageImage( 6, 6 );
	m_LyricsPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_OnlinePanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_OnlinePanel, _( "Online" ), false );
	m_MainNotebook->SetPageImage( 7, 7 );
	m_OnlinePanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_PodcastPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_PodcastPanel, _("Podcasts"), false );
	m_MainNotebook->SetPageImage( 8, 8 );
	m_PodcastPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_JamendoPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_JamendoPanel, wxT("Jamendo"), false );
	m_MainNotebook->SetPageImage( 9, 9 );
	m_JamendoPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_MagnatunePanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_MagnatunePanel, wxT("Magnatune"), false );
	m_MainNotebook->SetPageImage( 10, 10 );
	m_MagnatunePanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_LinksPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_LinksPanel, _("Links"), false );
	m_MainNotebook->SetPageImage( 11, 11 );
	m_LinksPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_CmdPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_CmdPanel, _( "Commands" ), false );
	m_MainNotebook->SetPageImage( 12, 12 );
	m_CmdPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_CopyPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_CopyPanel, _( "Copy to" ), false );
	m_MainNotebook->SetPageImage( 13, 13 );
	m_CopyPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_AccelPanel = new wxScrolledWindow( m_MainNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
	m_MainNotebook->AddPage( m_AccelPanel, _( "Shortcuts" ), false );
	m_MainNotebook->SetPageImage( 14, 14 );
	m_AccelPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

    if( pagenum == guPREFERENCE_PAGE_LASTUSED )
    {
        pagenum = m_Config->ReadNum( wxT( "LastPage" ), guPREFERENCE_PAGE_GENERAL, wxT( "preferences" ) );
    }

    switch( pagenum )
    {
        case guPREFERENCE_PAGE_GENERAL :
            BuildGeneralPage();
            break;

        case guPREFERENCE_PAGE_LIBRARY :
            BuildLibraryPage();
            break;

        case guPREFERENCE_PAGE_PLAYBACK :
            BuildPlaybackPage();
            break;

        case guPREFERENCE_PAGE_CROSSFADER :
            BuildCrossfaderPage();
            break;

        case guPREFERENCE_PAGE_RECORD :
            BuildRecordPage();
            break;

        case guPREFERENCE_PAGE_AUDIOSCROBBLE :
            BuildAudioScrobblePage();
            break;

        case guPREFERENCE_PAGE_LYRICS :
            BuildLyricsPage();
            break;

        case guPREFERENCE_PAGE_ONLINE :
            BuildOnlinePage();
            break;

        case guPREFERENCE_PAGE_PODCASTS :
            BuildPodcastsPage();
            break;

        case guPREFERENCE_PAGE_JAMENDO :
            BuildJamendoPage();
            break;

        case guPREFERENCE_PAGE_MAGNATUNE :
            BuildMagnatunePage();
            break;

        case guPREFERENCE_PAGE_LINKS :
            BuildLinksPage();
            break;

        case guPREFERENCE_PAGE_COMMANDS :
            BuildCommandsPage();
            break;

        case guPREFERENCE_PAGE_COPYTO :
            BuildCopyToPage();
            break;

        case guPREFERENCE_PAGE_ACCELERATORS :
            BuildAcceleratorsPage();
            break;
    }

    m_MainNotebook->SetSelection( pagenum );

    //
	MainSizer->Add( m_MainNotebook, 1, wxEXPAND | wxALL, 5 );

    wxStdDialogButtonSizer *    ButtonsSizer;
    wxButton *                  ButtonsSizerOK;
    wxButton *                  ButtonsSizerCancel;

	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK, _( " Accept " ) );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL, _( " Cancel " ) );
	ButtonsSizer->AddButton( ButtonsSizerCancel );

	ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
	ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    ButtonsSizerOK->SetDefault();

	m_MainNotebook->Connect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED, wxCommandEventHandler( guPrefDialog::OnPageChanged ), NULL, this );

    //
    //
    //
    m_PathSelected = wxNOT_FOUND;
    m_FilterSelected = wxNOT_FOUND;

}

// -------------------------------------------------------------------------------- //
guPrefDialog::~guPrefDialog()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    // Save the window position and size
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( wxT( "PosX" ), WindowPos.x, wxT( "preferences" ) );
    Config->WriteNum( wxT( "PosY" ), WindowPos.y, wxT( "preferences" ) );
    wxSize WindowSize = GetSize();
    Config->WriteNum( wxT( "Width" ), WindowSize.x, wxT( "preferences" ) );
    Config->WriteNum( wxT( "Height" ), WindowSize.y, wxT( "preferences" ) );
    m_Config->WriteNum( wxT( "LastPage" ), m_MainNotebook->GetSelection(), wxT( "preferences" ) );

    //
	m_MainNotebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxCommandEventHandler( guPrefDialog::OnPageChanged ), NULL, this );


    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        m_TaskIconChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );
        if( m_SoundMenuChkBox )
            m_SoundMenuChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateSoundMenuIntegration ), NULL, this );
        m_InstantSearchChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateInstantSearch ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
    {
//    	m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxSelected ), NULL, this );
//        m_AddPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddPathBtnClick ), NULL, this );
//        m_DelPathButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPathBtnClick ), NULL, this );
//        m_PathsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnPathsListBoxDClicked ), NULL, this );
//
//        m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCoversListBoxSelected ), NULL, this );
//        m_AddCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAddCoverBtnClick ), NULL, this );
//        m_UpCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnUpCoverBtnClick ), NULL, this );
//        m_DownCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDownCoverBtnClick ), NULL, this );
//        m_DelCoverButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelCoverBtnClick ), NULL, this );
//        m_CoversListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnCoverListBoxDClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        m_RndPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
        m_DelPlayChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
        m_PlayLevelEnabled->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
        m_PlayLevelSlider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnPlayLevelValueChanged ), NULL, this );
        m_PlayEndTimeCheckBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );
        m_PlayOutDevChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnPlayOutDevChanged ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
    {
        m_RecordChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecEnableClicked ), NULL, this );
        m_RecDelTracks->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecDelTracksClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
    {
        if( m_LyricSearchEngine )
        {
            delete m_LyricSearchEngine;
        }

        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSourceSelected ), NULL, this );
        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSourceDClicked ), NULL, this );
        m_LyricsSrcListBox->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSourceToggled ), NULL, this );
        m_LyricsAddButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricAddBtnClick ), NULL, this );
        m_LyricsUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricUpBtnClick ), NULL, this );
        m_LyricsDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDownBtnClick ), NULL, this );
        m_LyricsDelButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDelBtnClick ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSaveSelected ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDClicked ), NULL, this );
        m_LyricsSaveListBox->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSaveToggled ), NULL, this );
        m_LyricsSaveAddButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveAddBtnClick ), NULL, this );
        m_LyricsSaveUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveUpBtnClick ), NULL, this );
        m_LyricsSaveDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDownBtnClick ), NULL, this );
        m_LyricsSaveDelButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDelBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
    {
        m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
        m_OnlineAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
        m_OnlineDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
        m_OnlineFiltersListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        m_LastFMUserNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );
        m_LastFMPasswdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );

        m_LibreFMUserNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
        m_LibreFMPasswdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        m_JamSelAllBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectAll ), NULL, this );
        m_JamSelNoneBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectNone ), NULL, this );
        m_JamInvertBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoInvertSelection ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
    {
        m_LinksListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
        m_LinksAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
        m_LinksDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
        m_LinksMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
        m_LinksMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
        m_LinksUrlTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
        m_LinksNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
        m_LinksAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
    {
        m_CmdListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
        m_CmdAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
        m_CmdDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
        m_CmdMoveUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
        m_CmdMoveDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
        m_CmdTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
        m_CmdNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
        m_CmdAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
    {
        m_CopyToListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToListBoxSelected ), NULL, this );
        m_CopyToAddBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToAddBtnClick ), NULL, this );
        m_CopyToDelBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToDelBtnClick ), NULL, this );
        m_CopyToUpBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveUpBtnClick ), NULL, this );
        m_CopyToDownBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveDownBtnClick ), NULL, this );
        m_CopyToPatternTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
        m_CopyToNameTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
        m_CopyToFormatChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToFormatChanged ), NULL, this );
        m_CopyToQualityChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToQualityChanged ), NULL, this );
        m_CopyToMoveFilesChkBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveFilesChanged ), NULL, this );
        m_CopyToAcceptBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToSaveBtnClick ), NULL, this );

        if( m_CopyToOptions )
        {
            delete m_CopyToOptions;
        }
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        m_AccelListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( guPrefDialog::OnAccelSelected ), NULL, this );
        m_AccelListCtrl->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPrefDialog::OnAccelKeyDown ), NULL, this );
    }

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPageChanged( wxCommandEvent &event )
{
    switch( m_MainNotebook->GetSelection() )
    {
        case guPREFERENCE_PAGE_GENERAL          : BuildGeneralPage();       break;
        case guPREFERENCE_PAGE_LIBRARY          : BuildLibraryPage();       break;
        case guPREFERENCE_PAGE_PLAYBACK         : BuildPlaybackPage();      break;
        case guPREFERENCE_PAGE_CROSSFADER       : BuildCrossfaderPage();    break;
        case guPREFERENCE_PAGE_RECORD           : BuildRecordPage();        break;
        case guPREFERENCE_PAGE_AUDIOSCROBBLE    : BuildAudioScrobblePage(); break;
        case guPREFERENCE_PAGE_LYRICS           : BuildLyricsPage();        break;
        case guPREFERENCE_PAGE_ONLINE           : BuildOnlinePage();        break;
        case guPREFERENCE_PAGE_PODCASTS         : BuildPodcastsPage();      break;
        case guPREFERENCE_PAGE_JAMENDO          : BuildJamendoPage();       break;
        case guPREFERENCE_PAGE_MAGNATUNE        : BuildMagnatunePage();     break;
        case guPREFERENCE_PAGE_LINKS            : BuildLinksPage();         break;
        case guPREFERENCE_PAGE_COMMANDS         : BuildCommandsPage();      break;
        case guPREFERENCE_PAGE_COPYTO           : BuildCopyToPage();        break;
        case guPREFERENCE_PAGE_ACCELERATORS     : BuildAcceleratorsPage();  break;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildGeneralPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_GENERAL;

    //
    // General Preferences Panel
    //
	wxBoxSizer * GenMainSizer = new wxBoxSizer( wxVERTICAL );
	wxStaticBoxSizer * StartSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Start ") ), wxVERTICAL );

	wxBoxSizer * LangSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * LangStaticText = new wxStaticText( m_GenPanel, wxID_ANY, _( "Language:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LangStaticText->Wrap( -1 );
	LangSizer->Add( LangStaticText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT|wxBOTTOM, 5 );

	m_MainLangChoice = new wxChoice( m_GenPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MainLangChoices, 0 );
	int LangEntry = m_MainLangCodes.Index( m_Config->ReadNum( wxT( "Language" ), 0, wxT( "general" ) ) );
	if( LangEntry == wxNOT_FOUND )
        LangEntry = 0;
	m_MainLangChoice->SetSelection( LangEntry );
	LangSizer->Add( m_MainLangChoice, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5 );

	wxStaticText * LangNoteStaticText = new wxStaticText( m_GenPanel, wxID_ANY, _( "(Needs restart)" ), wxDefaultPosition, wxDefaultSize, 0 );
	LangNoteStaticText->Wrap( -1 );
	LangSizer->Add( LangNoteStaticText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT|wxBOTTOM, 5 );

	StartSizer->Add( LangSizer, 1, wxEXPAND, 5 );

	m_ShowSplashChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Show splash screen"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowSplashChkBox->SetValue( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "general" ) ) );
	StartSizer->Add( m_ShowSplashChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_MinStartChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Start minimized"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinStartChkBox->SetValue( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "general" ) ) );
	StartSizer->Add( m_MinStartChkBox, 0, wxLEFT | wxRIGHT, 5 );

	wxBoxSizer * StartPlayingSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SavePosCheckBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Restore position for tracks longer than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePosCheckBox->SetValue( m_Config->ReadBool( wxT( "SaveCurrentTrackPos" ), false, wxT( "general" ) ) );

	StartPlayingSizer->Add( m_SavePosCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	m_MinLenSpinCtrl = new wxSpinCtrl( m_GenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999, 10 );
	m_MinLenSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinSavePlayPosLength" ), 10, wxT( "general" ) ) );
	m_MinLenSpinCtrl->SetToolTip( _( "set the minimun length in minutes to save track position" ) );

	StartPlayingSizer->Add( m_MinLenSpinCtrl, 0, wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MinLenStaticText = new wxStaticText( m_GenPanel, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
	MinLenStaticText->Wrap( -1 );
	StartPlayingSizer->Add( MinLenStaticText, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	StartSizer->Add( StartPlayingSizer, 1, wxEXPAND, 5 );

    m_IgnoreLayoutsChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Load default layout" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreLayoutsChkBox->SetValue( m_Config->ReadBool( wxT( "LoadDefaultLayouts" ), false, wxT( "general" ) ) );
	StartSizer->Add( m_IgnoreLayoutsChkBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	GenMainSizer->Add( StartSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * BehaviSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" Behaviour ") ), wxVERTICAL );

	m_TaskIconChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Activate task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TaskIconChkBox->SetValue( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "general" ) ) );
	BehaviSizer->Add( m_TaskIconChkBox, 0, wxLEFT | wxRIGHT, 5 );

    m_SoundMenuChkBox = NULL;
    bool WithIndicateSupport = false;
#ifdef WITH_LIBINDICATE_SUPPORT
    WithIndicateSupport = true;
#endif

    guMPRIS2 * MPRIS2 = guMPRIS2::Get();
    bool IsSoundMenuAvailable = MPRIS2->Indicators_Sound_Available();

    if( WithIndicateSupport || IsSoundMenuAvailable )
    {
        m_SoundMenuChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Integrate into SoundMenu" ), wxDefaultPosition, wxDefaultSize, 0 );
        m_SoundMenuChkBox->SetValue( m_Config->ReadBool( wxT( "SoundMenuIntegration" ), false, wxT( "general" ) ) );
        m_SoundMenuChkBox->Enable( m_TaskIconChkBox->IsChecked() );
        BehaviSizer->Add( m_SoundMenuChkBox, 0, wxLEFT | wxRIGHT, 5 );
    }

	m_EnqueueChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Enqueue as default action"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnqueueChkBox->SetValue( m_Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "general" ) ) );
	BehaviSizer->Add( m_EnqueueChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_DropFilesChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Drop files clear playlist"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DropFilesChkBox->SetValue( m_Config->ReadBool( wxT( "DropFilesClearPlaylist" ), false, wxT( "general" ) ) );
	BehaviSizer->Add( m_DropFilesChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_InstantSearchChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Instant text search" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_InstantSearchChkBox->SetValue( m_Config->ReadBool( wxT( "InstantTextSearchEnabled" ), true, wxT( "general" ) ) );
	BehaviSizer->Add( m_InstantSearchChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_EnterSearchChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "When searching, pressing enter queues the result" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnterSearchChkBox->SetValue( m_Config->ReadBool( wxT( "TextSearchEnterRelax" ), false, wxT( "general" ) ) );
    m_EnterSearchChkBox->Enable( m_InstantSearchChkBox->IsChecked() );
	BehaviSizer->Add( m_EnterSearchChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_ShowCDFrameChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _( "Show CD cover frame in player" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowCDFrameChkBox->SetValue( m_Config->ReadNum( wxT( "CoverFrame" ), 1, wxT( "general" ) ) );
	BehaviSizer->Add( m_ShowCDFrameChkBox, 0, wxLEFT | wxRIGHT, 5 );

	GenMainSizer->Add( BehaviSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * OnCloseSizer = new wxStaticBoxSizer( new wxStaticBox( m_GenPanel, wxID_ANY, _(" On Close ") ), wxVERTICAL );

	m_SavePlayListChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Save playlist on close"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SavePlayListChkBox->SetValue( m_Config->ReadBool( wxT( "SaveOnClose" ), true, wxT( "playlist" ) ) );
	OnCloseSizer->Add( m_SavePlayListChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_CloseTaskBarChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Close to task bar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CloseTaskBarChkBox->SetValue( m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "general" ) ) );
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && ( !m_SoundMenuChkBox || !m_SoundMenuChkBox->IsChecked() ) );
	OnCloseSizer->Add( m_CloseTaskBarChkBox, 0, wxLEFT | wxRIGHT, 5 );

	m_ExitConfirmChkBox = new wxCheckBox( m_GenPanel, wxID_ANY, _("Ask confirmation on exit"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExitConfirmChkBox->SetValue( m_Config->ReadBool( wxT( "ShowCloseConfirm" ), true, wxT( "general" ) ) );
	OnCloseSizer->Add( m_ExitConfirmChkBox, 0, wxLEFT | wxRIGHT, 5 );

	GenMainSizer->Add( OnCloseSizer, 0, wxEXPAND|wxALL, 5 );

	m_GenPanel->SetSizer( GenMainSizer );
	m_GenPanel->Layout();
	GenMainSizer->FitInside( m_GenPanel );

	m_TaskIconChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateTaskBarIcon ), NULL, this );
	if( m_SoundMenuChkBox )
        m_SoundMenuChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateSoundMenuIntegration ), NULL, this );
	m_InstantSearchChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnActivateInstantSearch ), NULL, this );

    m_ShowSplashChkBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLibraryPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LIBRARY;

    //
    // Library Preferences Panel
    //
    m_CollectSelected = wxNOT_FOUND;
    m_PathSelected = wxNOT_FOUND;
    m_CoverSelected = wxNOT_FOUND;

	wxBoxSizer * LibMainFrame = new wxBoxSizer( wxVERTICAL );

	m_LibSplitter = new wxSplitterWindow( m_LibPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_LibSplitter->SetMinimumPaneSize( 100 );

	//wxPanel * LibCollectPanel = new wxPanel( m_LibSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxScrolledWindow * LibCollectPanel = new wxScrolledWindow( m_LibSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * LibCollectMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LibCollectSizer = new wxStaticBoxSizer( new wxStaticBox( LibCollectPanel, wxID_ANY, _( " Collections " ) ), wxHORIZONTAL );

    m_Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_NORMAL );
    m_Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE );
    m_Config->LoadCollections( &m_Collections, guMEDIA_COLLECTION_TYPE_IPOD );

	m_LibCollectListBox = new wxListBox( LibCollectPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_HSCROLL|wxLB_SINGLE );
	int Index;
	int Count = m_Collections.Count();
	for( Index = 0; Index < Count; Index++ )
	{
        m_LibCollectListBox->Append( m_Collections[ Index ].m_Name );
	}
	LibCollectSizer->Add( m_LibCollectListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * LibCollectBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LibCollectAddBtn = new wxBitmapButton( LibCollectPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	LibCollectBtnSizer->Add( m_LibCollectAddBtn, 0, wxTOP, 5 );

	m_LibCollectUpBtn = new wxBitmapButton( LibCollectPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibCollectUpBtn->Enable( false );
	LibCollectBtnSizer->Add( m_LibCollectUpBtn, 0, 0, 5 );

	m_LibCollectDownBtn = new wxBitmapButton( LibCollectPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibCollectDownBtn->Enable( false );
	LibCollectBtnSizer->Add( m_LibCollectDownBtn, 0, 0, 5 );

	m_LibCollectDelBtn = new wxBitmapButton( LibCollectPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibCollectDelBtn->Enable( false );
	LibCollectBtnSizer->Add( m_LibCollectDelBtn, 0, wxBOTTOM, 5 );

	LibCollectSizer->Add( LibCollectBtnSizer, 0, wxEXPAND, 5 );

	LibCollectMainSizer->Add( LibCollectSizer, 1, wxEXPAND|wxTOP|wxBOTTOM|wxLEFT, 5 );

	LibCollectPanel->SetSizer( LibCollectMainSizer );
	LibCollectPanel->Layout();
	LibCollectMainSizer->FitInside( LibCollectPanel );

    LibCollectPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );


	m_LibOptPanel = new wxScrolledWindow( m_LibSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * LibOptMainSizer = new wxBoxSizer( wxVERTICAL );

	m_LibOptSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibOptPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	m_LibOptPathSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibOptPanel, wxID_ANY, _(" Paths ") ), wxHORIZONTAL );

	m_LibPathListBox = new wxListBox( m_LibOptPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE );
	m_LibPathListBox->Enable( false );
	m_LibOptPathSizer->Add( m_LibPathListBox, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer * LibOptPathBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LibOptAddPathBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibOptAddPathBtn->Enable( false );
	LibOptPathBtnSizer->Add( m_LibOptAddPathBtn, 0, wxTOP, 5 );

	m_LibOptDelPathBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibOptDelPathBtn->Enable( false );
	LibOptPathBtnSizer->Add( m_LibOptDelPathBtn, 0, wxBOTTOM, 5 );

	m_LibOptPathSizer->Add( LibOptPathBtnSizer, 0, wxEXPAND, 5 );

	m_LibOptSizer->Add( m_LibOptPathSizer, 1, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	wxStaticBoxSizer* LibOptCoversSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibOptPanel, wxID_ANY, _(" Words to detect covers ") ), wxHORIZONTAL );

	m_LibCoverListBox = new wxListBox( m_LibOptPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	m_LibCoverListBox->Enable( false );
	LibOptCoversSizer->Add( m_LibCoverListBox, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer * LibOptCoverBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LibOptAddCoverBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_LibOptAddCoverBtn->Enable( false );
	LibOptCoverBtnSizer->Add( m_LibOptAddCoverBtn, 0, wxTOP, 5 );

	m_LibOptUpCoverBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibOptUpCoverBtn->Enable( false );
	LibOptCoverBtnSizer->Add( m_LibOptUpCoverBtn, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LibOptDownCoverBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibOptDownCoverBtn->Enable( false );
	LibOptCoverBtnSizer->Add( m_LibOptDownCoverBtn, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LibOptDelCoverBtn = new wxBitmapButton( m_LibOptPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LibOptDelCoverBtn->Enable( false );
	LibOptCoverBtnSizer->Add( m_LibOptDelCoverBtn, 0, wxBOTTOM, 5 );

	LibOptCoversSizer->Add( LibOptCoverBtnSizer, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LibOptSizer->Add( LibOptCoversSizer, 1, wxEXPAND, 5 );

	m_LibOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_LibOptPanel, wxID_ANY, _( " Options " ) ), wxVERTICAL );

	m_LibOptAutoUpdateChkBox = new wxCheckBox( m_LibOptPanel, wxID_ANY, _( "Update when opened " ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibOptAutoUpdateChkBox->Enable( false );
	m_LibOptionsSizer->Add( m_LibOptAutoUpdateChkBox, 0, wxRIGHT|wxLEFT, 5 );

	m_LibOptCreatePlayListChkBox = new wxCheckBox( m_LibOptPanel, wxID_ANY, _( "Create playlists on scan" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibOptCreatePlayListChkBox->Enable( false );
	m_LibOptCreatePlayListChkBox->SetValue(true);
	m_LibOptionsSizer->Add( m_LibOptCreatePlayListChkBox, 0, wxRIGHT|wxLEFT, 5 );

	m_LibOptFollowLinksChkBox = new wxCheckBox( m_LibOptPanel, wxID_ANY, _( "Follow symbolic links on scan" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibOptFollowLinksChkBox->Enable( false );
	m_LibOptionsSizer->Add( m_LibOptFollowLinksChkBox, 0, wxRIGHT|wxLEFT, 5 );

	m_LibOptCheckEmbeddedChkBox = new wxCheckBox( m_LibOptPanel, wxID_ANY, _( "Scan embedded covers in audio files" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibOptCheckEmbeddedChkBox->SetValue(true);
	m_LibOptCheckEmbeddedChkBox->Enable( false );
	m_LibOptionsSizer->Add( m_LibOptCheckEmbeddedChkBox, 0, wxRIGHT|wxLEFT, 5 );

	m_LibOptEmbedTagsChkBox = new wxCheckBox( m_LibOptPanel, wxID_ANY, _( "Embed rating, play count and labels" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_LibOptEmbedTagsChkBox->Enable( false );
	m_LibOptionsSizer->Add( m_LibOptEmbedTagsChkBox, 0, wxRIGHT|wxLEFT, 5 );

	wxBoxSizer * LibOptCopyToSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * LibOptCopyToLabel = new wxStaticText( m_LibOptPanel, wxID_ANY, _( "Default copy action" ), wxDefaultPosition, wxDefaultSize, 0 );
	LibOptCopyToLabel->Wrap( -1 );
	LibOptCopyToSizer->Add( LibOptCopyToLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	wxArrayString CopyToChoices;
	CopyToChoices.Add( _( "None" ) );
	wxArrayString CopyToOptions = m_Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "copyto/options" ) );
	Count = CopyToOptions.Count();
	for( Index = 0; Index < Count; Index++ )
	{
	    CopyToChoices.Add( CopyToOptions[ Index ].BeforeFirst( wxT( ':' ) ) );
	}
	m_LibOptCopyToChoice = new wxChoice( m_LibOptPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, CopyToChoices, 0 );
	m_LibOptCopyToChoice->SetSelection( 0 );
	m_LibOptCopyToChoice->Enable( false );
	LibOptCopyToSizer->Add( m_LibOptCopyToChoice, 1, wxEXPAND, 5 );

	m_LibOptionsSizer->Add( LibOptCopyToSizer, 0, wxEXPAND, 5 );

	m_LibOptSizer->Add( m_LibOptionsSizer, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	LibOptMainSizer->Add( m_LibOptSizer, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_LibOptPanel->SetSizer( LibOptMainSizer );
	m_LibOptPanel->Layout();
	LibOptMainSizer->FitInside( m_LibOptPanel );

    m_LibOptPanel->SetScrollRate( PREFERENCES_SCROLL_STEP, PREFERENCES_SCROLL_STEP );

	m_LibSplitter->SplitVertically( LibCollectPanel, m_LibOptPanel, 170 );
	LibMainFrame->Add( m_LibSplitter, 1, wxEXPAND, 5 );

	m_LibPanel->SetSizer( LibMainFrame );
	m_LibPanel->Layout();
	LibMainFrame->FitInside( m_LibPanel );

    //
	m_LibSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPrefDialog::LibSplitterOnIdle ), NULL, this );
	m_LibCollectListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLibCollectSelected ), NULL, this );
	m_LibCollectListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLibCollectDClicked ), NULL, this );
	m_LibCollectAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibAddCollectClick ), NULL, this );
	m_LibCollectUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibUpCollectClick ), NULL, this );
	m_LibCollectDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibDownCollectClick ), NULL, this );
	m_LibCollectDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibDelCollectClick ), NULL, this );
	m_LibPathListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLibPathSelected ), NULL, this );
	m_LibPathListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLibPathDClicked ), NULL, this );
	m_LibOptAddPathBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibAddPathBtnClick ), NULL, this );
	m_LibOptDelPathBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibDelPathBtnClick ), NULL, this );
	m_LibCoverListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLibCoverSelected ), NULL, this );
	m_LibCoverListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLibCoverDClicked ), NULL, this );
	m_LibOptAddCoverBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibAddCoverBtnClick ), NULL, this );
	m_LibOptUpCoverBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibUpCoverBtnClick ), NULL, this );
	m_LibOptDownCoverBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibDownCoverBtnClick ), NULL, this );
	m_LibOptDelCoverBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibDelCoverBtnClick ), NULL, this );
	m_LibOptAutoUpdateChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibAutoUpdateChanged ), NULL, this );
	m_LibOptCreatePlayListChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibCreatePlayListsChanged ), NULL, this );
	m_LibOptFollowLinksChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibFollowSymLinksChanged ), NULL, this );
	m_LibOptCheckEmbeddedChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibCheckEmbeddedChanged ), NULL, this );
	m_LibOptEmbedTagsChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnLibEmbeddMetadataChanged ), NULL, this );
	m_LibOptCopyToChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnLibDefaultCopyToChanged ), NULL, this );

	m_LibCollectListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildPlaybackPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_PLAYBACK;

    //
    // Playback Panel
    //
	wxBoxSizer * PlayMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PlayGenSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxBoxSizer * RandomPlaySizer = new wxBoxSizer( wxHORIZONTAL );

	m_RndPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Play random" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_RndPlayChkBox->SetValue( m_Config->ReadBool( wxT( "RndPlayOnEmptyPlayList" ), false, wxT( "general" ) ) );
	RandomPlaySizer->Add( m_RndPlayChkBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxString m_RndModeChoiceChoices[] = { _( "track" ), _( "album" ) };
	int m_RndModeChoiceNChoices = sizeof( m_RndModeChoiceChoices ) / sizeof( wxString );
	m_RndModeChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RndModeChoiceNChoices, m_RndModeChoiceChoices, 0 );
    m_RndModeChoice->Enable( m_RndPlayChkBox->IsChecked() );
	m_RndModeChoice->SetSelection( m_Config->ReadNum( wxT( "RndModeOnEmptyPlayList" ), 0, wxT( "general" ) ) );
	//m_RndModeChoice->SetMinSize( wxSize( 150,-1 ) );
	RandomPlaySizer->Add( m_RndModeChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RndTextStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "when playlist is empty" ), wxDefaultPosition, wxDefaultSize, 0 );
	RndTextStaticText->Wrap( -1 );
	RandomPlaySizer->Add( RndTextStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	PlayGenSizer->Add( RandomPlaySizer, 1, wxEXPAND, 5 );

	m_DelPlayChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Delete played tracks from playlist" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_DelPlayChkBox->SetValue( m_Config->ReadBool( wxT( "DelTracksPlayed" ), false, wxT( "playback" ) ) );
	PlayGenSizer->Add( m_DelPlayChkBox, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_NotifyChkBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _( "Show notifications" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_NotifyChkBox->SetValue( m_Config->ReadBool( wxT( "ShowNotifications" ), true, wxT( "general" ) ) );
	PlayGenSizer->Add( m_NotifyChkBox, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer * PlayReplaySizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * PlayReplayLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("ReplayGain mode:"), wxDefaultPosition, wxDefaultSize, 0 );
	PlayReplayLabel->Wrap( -1 );
	PlayReplaySizer->Add( PlayReplayLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxString m_PlayReplayModeChoiceChoices[] = { _( "Disabled" ), _("Track"), _("Album") };
	int m_PlayReplayModeChoiceNChoices = sizeof( m_PlayReplayModeChoiceChoices ) / sizeof( wxString );
	m_PlayReplayModeChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PlayReplayModeChoiceNChoices, m_PlayReplayModeChoiceChoices, 0 );
	int ReplayGainModeVal = m_Config->ReadNum( wxT( "ReplayGainMode"), 0, wxT( "general" ) );
	m_PlayReplayModeChoice->SetSelection( ReplayGainModeVal );
	PlayReplaySizer->Add( m_PlayReplayModeChoice, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * PlayPreAmpLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _( "PreAmp:" ), wxDefaultPosition, wxDefaultSize, 0 );
	PlayPreAmpLabel->Wrap( -1 );
	PlayReplaySizer->Add( PlayPreAmpLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    int ReplayGainPreAmpVal = m_Config->ReadNum( wxT( "ReplayGainPreAmp"), 6, wxT( "general" ) );
	m_PlayPreAmpLevelVal = new wxStaticText( m_PlayPanel, wxID_ANY, wxString::Format( wxT("%idb"), ReplayGainPreAmpVal ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayPreAmpLevelVal->Wrap( -1 );
	m_PlayPreAmpLevelVal->Enable( ReplayGainModeVal );
	PlayReplaySizer->Add( m_PlayPreAmpLevelVal, 0, wxALIGN_CENTER_VERTICAL, 5 );

	m_PlayPreAmpLevelSlider = new wxSlider( m_PlayPanel, wxID_ANY, ReplayGainPreAmpVal, -20, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_PlayPreAmpLevelSlider->Enable( ReplayGainModeVal );
	PlayReplaySizer->Add( m_PlayPreAmpLevelSlider, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxEXPAND, 5 );

	PlayGenSizer->Add( PlayReplaySizer, 1, wxEXPAND, 5 );

	PlayMainSizer->Add( PlayGenSizer, 0, wxEXPAND|wxALL, 5 );


    wxStaticBoxSizer * SmartPlayListSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _( " Random / Smart play modes " ) ), wxVERTICAL );

    wxFlexGridSizer * SmartPlayListFlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
	SmartPlayListFlexGridSizer->SetFlexibleDirection( wxBOTH );
	SmartPlayListFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_MinTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 10, 4 );
    m_MinTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "MinTracksToPlay" ), 4, wxT( "playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_MinTracksSpinCtrl, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MinTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks left to start search"), wxDefaultPosition, wxDefaultSize, 0 );
	MinTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MinTracksStaticText, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_NumTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 10, 3 );
    m_NumTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "NumTracksToAdd" ), 3, wxT( "playback" ) ) );
	SmartPlayListFlexGridSizer->Add( m_NumTracksSpinCtrl, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * AddTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Tracks added each time"), wxDefaultPosition, wxDefaultSize, 0 );
	AddTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( AddTracksStaticText, 0, wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	m_MaxTracksPlayed = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 999, 20 );
    m_MaxTracksPlayed->SetValue( m_Config->ReadNum( wxT( "MaxTracksPlayed" ), 20, wxT( "playback" ) ) );
    m_MaxTracksPlayed->Enable( !m_DelPlayChkBox->IsChecked() );
	SmartPlayListFlexGridSizer->Add( m_MaxTracksPlayed, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * MaxTracksStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _("Max played tracks kept in playlist"), wxDefaultPosition, wxDefaultSize, 0 );
	MaxTracksStaticText->Wrap( -1 );
	SmartPlayListFlexGridSizer->Add( MaxTracksStaticText, 0, wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	SmartPlayListSizer->Add( SmartPlayListFlexGridSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * SmartPlayFilterSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * SmartPlayFilterLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("Don't repeat last"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayFilterLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayFilterLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_SmartPlayArtistsSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 50, 20 );
    m_SmartPlayArtistsSpinCtrl->SetValue( m_Config->ReadNum( wxT( "SmartFilterArtists" ), 20, wxT( "playback" ) ) );
	SmartPlayFilterSizer->Add( m_SmartPlayArtistsSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * SmartPlayArtistLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("artists or"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayArtistLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayArtistLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_SmartPlayTracksSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 55,-1 ), wxSP_ARROW_KEYS, 0, 200, 100 );
    m_SmartPlayTracksSpinCtrl->SetValue( m_Config->ReadNum( wxT( "SmartFilterTracks" ), 100, wxT( "playback" ) ) );
	SmartPlayFilterSizer->Add( m_SmartPlayTracksSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * SmartPlayTracksLabel = new wxStaticText( m_PlayPanel, wxID_ANY, _("tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	SmartPlayTracksLabel->Wrap( -1 );
	SmartPlayFilterSizer->Add( SmartPlayTracksLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	SmartPlayListSizer->Add( SmartPlayFilterSizer, 0, wxEXPAND, 5 );

	PlayMainSizer->Add( SmartPlayListSizer, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer * PlaySilenceSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _(" Silence detector ") ), wxVERTICAL );

	wxBoxSizer * PlayLevelSizer = new wxBoxSizer( wxHORIZONTAL );

	bool IsPlayLevelEnabled = m_Config->ReadBool( wxT( "SilenceDetector" ), false, wxT( "playback" ) );
	m_PlayLevelEnabled = new wxCheckBox( m_PlayPanel, wxID_ANY, _("Skip at"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayLevelEnabled->SetValue( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelEnabled, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    int PlayLevelValue = m_Config->ReadNum( wxT( "SilenceLevel" ), -500, wxT( "playback" ) );
	m_PlayLevelVal = new wxStaticText( m_PlayPanel, wxID_ANY, wxString::Format( wxT("%02idb"), PlayLevelValue ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayLevelVal->Wrap( -1 );
	m_PlayLevelVal->Enable( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelVal, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT, 5 );

	m_PlayLevelSlider = new wxSlider( m_PlayPanel, wxID_ANY, PlayLevelValue, -65, 0, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_PlayLevelSlider->Enable( IsPlayLevelEnabled );
	PlayLevelSizer->Add( m_PlayLevelSlider, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlaySilenceSizer->Add( PlayLevelSizer, 0, wxEXPAND, 5 );

	wxBoxSizer* PlayEndTimeSizer;
	PlayEndTimeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PlayEndTimeCheckBox = new wxCheckBox( m_PlayPanel, wxID_ANY, _("In the last"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PlayEndTimeCheckBox->SetValue( m_Config->ReadBool( wxT( "SilenceAtEnd" ), true, wxT( "playback" ) ) );
	m_PlayEndTimeCheckBox->Enable( IsPlayLevelEnabled );
	PlayEndTimeSizer->Add( m_PlayEndTimeCheckBox, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_PlayEndTimeSpinCtrl = new wxSpinCtrl( m_PlayPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 5, 360,
	    m_Config->ReadNum( wxT( "SilenceEndTime" ), 45, wxT( "playback" ) ) );
	m_PlayEndTimeSpinCtrl->Enable( IsPlayLevelEnabled && m_PlayEndTimeCheckBox->IsChecked() );
	PlayEndTimeSizer->Add( m_PlayEndTimeSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * PlayEndTimeStaticText = new wxStaticText( m_PlayPanel, wxID_ANY, _( "seconds" ), wxDefaultPosition, wxDefaultSize, 0 );
	PlayEndTimeStaticText->Wrap( -1 );
	PlayEndTimeSizer->Add( PlayEndTimeStaticText, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlaySilenceSizer->Add( PlayEndTimeSizer, 0, wxEXPAND, 5 );

	PlayMainSizer->Add( PlaySilenceSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer* PlayOutDeviceSizer = new wxStaticBoxSizer( new wxStaticBox( m_PlayPanel, wxID_ANY, _(" Output device ") ), wxHORIZONTAL );

    wxArrayString OutputDeviceOptions;
    OutputDeviceOptions.Add( _( "Automatic" ) );
    OutputDeviceOptions.Add( _( "GConf Defined" ) );
    OutputDeviceOptions.Add( wxT( "Alsa" ) );
    OutputDeviceOptions.Add( wxT( "PulseAudio" ) );
    OutputDeviceOptions.Add( wxT( "OSS" ) );
    OutputDeviceOptions.Add( wxT( "Other" ) );

	m_PlayOutDevChoice = new wxChoice( m_PlayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, OutputDeviceOptions, 0 );
	int OutDevice = m_Config->ReadNum( wxT( "OutputDevice" ), guOUTPUT_DEVICE_AUTOMATIC, wxT( "playback" ) );
	m_PlayOutDevChoice->SetSelection( OutDevice );
	PlayOutDeviceSizer->Add( m_PlayOutDevChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	m_PlayOutDevName = new wxTextCtrl( m_PlayPanel, wxID_ANY, m_Config->ReadStr( wxT( "OutputDeviceName" ), wxEmptyString, wxT( "playback" ) ), wxDefaultPosition, wxDefaultSize, 0 );
	m_PlayOutDevName->Enable( OutDevice > guOUTPUT_DEVICE_GCONF );
	PlayOutDeviceSizer->Add( m_PlayOutDevName, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	PlayMainSizer->Add( PlayOutDeviceSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_PlayPanel->SetSizer( PlayMainSizer );
	m_PlayPanel->Layout();
	PlayMainSizer->FitInside( m_PlayPanel );


    //
    //
    //
	m_RndPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRndPlayClicked ), NULL, this );
	m_DelPlayChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnDelPlayedTracksChecked ), NULL, this );
	m_PlayReplayModeChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnReplayGainModeChanged ), NULL, this );
    m_PlayPreAmpLevelSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnPlayPreAmpLevelValueChanged ), NULL, this );
    m_PlayPreAmpLevelSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnPlayPreAmpLevelValueChanged ), NULL, this );
	m_PlayLevelEnabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayLevelEnabled ), NULL, this );
    m_PlayLevelSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnPlayLevelValueChanged ), NULL, this );
    m_PlayLevelSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnPlayLevelValueChanged ), NULL, this );
	m_PlayEndTimeCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnPlayEndTimeEnabled ), NULL, this );
	m_PlayOutDevChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnPlayOutDevChanged ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCrossfaderPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_CROSSFADER )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_CROSSFADER;

    //
    // Crossfader Panel
    //
	wxBoxSizer * XFadeMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * XFadesbSizer = new wxStaticBoxSizer( new wxStaticBox( m_XFadePanel, wxID_ANY, _(" Crossfader ") ), wxVERTICAL );

    wxFlexGridSizer * XFadeFlexSizer = new wxFlexGridSizer( 3, 0, 0 );
	XFadeFlexSizer->AddGrowableCol( 2 );
	XFadeFlexSizer->SetFlexibleDirection( wxBOTH );
	XFadeFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * XFadeOutLenLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("Fade-out length:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeOutLenLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeOutLenLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeOutLenVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeOutLenVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeOutLenVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeOutLenSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeOutTime" ), 50, wxT( "crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeOutLenSlider->SetToolTip( _( "Select the length of the fade out. 0 for gapless playback" ) );
	XFadeFlexSizer->Add( m_XFadeOutLenSlider, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeInLenLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("Fade-in length:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeInLenLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeInLenLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInLenVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeInLenVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeInLenVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInLenSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInTime" ), 10, wxT( "crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInLenSlider->SetToolTip( _( "Select the length of the fade in" ) );
	XFadeFlexSizer->Add( m_XFadeInLenSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeInStartLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("Fade-in volume:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeInStartLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeInStartLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInStartVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeInStartVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeInStartVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInStartSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInVolStart" ), 80, wxT( "crossfader" ) ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInStartSlider->SetToolTip( _( "Select the initial volume of the fade in" ) );
	XFadeFlexSizer->Add( m_XFadeInStartSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * XFadeTrigerLabel = new wxStaticText( m_XFadePanel, wxID_ANY, _("Fade-in start:"), wxDefaultPosition, wxDefaultSize, 0 );
	XFadeTrigerLabel->Wrap( -1 );
	XFadeFlexSizer->Add( XFadeTrigerLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeTrigerVal = new wxStaticText( m_XFadePanel, wxID_ANY, wxT( "00.0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_XFadeTrigerVal->Wrap( -1 );
	XFadeFlexSizer->Add( m_XFadeTrigerVal, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_XFadeInTrigerSlider = new wxSlider( m_XFadePanel, wxID_ANY, m_Config->ReadNum( wxT( "FadeInVolTriger" ), 50, wxT( "crossfader" ) ), 10, 90, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    m_XFadeInTrigerSlider->SetToolTip( _( "Select at which volume of the fade out the fade in starts" ) );
	XFadeFlexSizer->Add( m_XFadeInTrigerSlider, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	XFadesbSizer->Add( XFadeFlexSizer, 1, wxEXPAND, 5 );

	XFadeMainSizer->Add( XFadesbSizer, 0, wxEXPAND|wxALL, 5 );

	m_FadeBitmap = new wxStaticBitmap( m_XFadePanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 400,200 ), 0 );
	XFadeMainSizer->Add( m_FadeBitmap, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_XFadePanel->SetSizer( XFadeMainSizer );
	m_XFadePanel->Layout();
	XFadeMainSizer->FitInside( m_XFadePanel );

    //
    //
    //
    m_XFadeOutLenSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeOutLenSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInLenSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInLenSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInStartSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInStartSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInTrigerSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );
    m_XFadeInTrigerSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnCrossFadeChanged ), NULL, this );

    //
    wxScrollEvent ScrollEvent;
    OnCrossFadeChanged( ScrollEvent );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildRecordPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_RECORD;

    //
    // Record Panel
    //
	wxBoxSizer * RecMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * RecordSizer = new wxStaticBoxSizer( new wxStaticBox( m_RecordPanel, wxID_ANY, _(" Record ") ), wxVERTICAL );

	m_RecordChkBox = new wxCheckBox( m_RecordPanel, wxID_ANY, _("Enable recording"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RecordChkBox->SetValue( m_Config->ReadBool( wxT( "Enabled" ), false, wxT( "record" ) ) );
	RecordSizer->Add( m_RecordChkBox, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer * RecSelDirSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText * RecSelDirLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Save to:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecSelDirLabel->Wrap( -1 );
	RecSelDirSizer->Add( RecSelDirLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_RecSelDirPicker = new wxDirPickerCtrl( m_RecordPanel, wxID_ANY, wxEmptyString, _("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST );
    m_RecSelDirPicker->SetPath( m_Config->ReadStr( wxT( "Path" ), guPATH_DEFAULT_RECORDINGS, wxT( "record" ) ) );
    m_RecSelDirPicker->Enable( m_RecordChkBox->IsChecked() );
	RecSelDirSizer->Add( m_RecSelDirPicker, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecordSizer->Add( RecSelDirSizer, 0, wxEXPAND, 5 );

	wxStaticBoxSizer * RecPropSizer = new wxStaticBoxSizer( new wxStaticBox( m_RecordPanel, wxID_ANY, _(" Properties ") ), wxVERTICAL );

    wxFlexGridSizer * RecPropFlexSizer = new wxFlexGridSizer( 2, 0, 0 );
	RecPropFlexSizer->AddGrowableCol( 1 );
	RecPropFlexSizer->SetFlexibleDirection( wxBOTH );
	RecPropFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * RecFormatLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecFormatLabel->Wrap( -1 );
	RecPropFlexSizer->Add( RecFormatLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	wxString m_RecFormatChoiceChoices[] = { wxT("mp3"), wxT("ogg"), wxT("flac") };
	int m_RecFormatChoiceNChoices = sizeof( m_RecFormatChoiceChoices ) / sizeof( wxString );
	m_RecFormatChoice = new wxChoice( m_RecordPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RecFormatChoiceNChoices, m_RecFormatChoiceChoices, 0 );
	m_RecFormatChoice->SetSelection( m_Config->ReadNum( wxT( "Format" ), 0, wxT( "record" ) ) );
    m_RecFormatChoice->Enable( m_RecordChkBox->IsChecked() );
	m_RecFormatChoice->SetMinSize( wxSize( 150,-1 ) );
	RecPropFlexSizer->Add( m_RecFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RecQualityLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("Quality:"), wxDefaultPosition, wxDefaultSize, 0 );
	RecQualityLabel->Wrap( -1 );
	RecPropFlexSizer->Add( RecQualityLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	wxArrayString RecQualityChoiceChoices;
	RecQualityChoiceChoices.Add( _( "Very High" ) );
	RecQualityChoiceChoices.Add( _( "High" ) );
	RecQualityChoiceChoices.Add( _( "Normal" ) );
	RecQualityChoiceChoices.Add( _( "Low" ) );
	RecQualityChoiceChoices.Add( _( "Very Low" ) );

	m_RecQualityChoice = new wxChoice( m_RecordPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, RecQualityChoiceChoices, 0 );
	m_RecQualityChoice->SetSelection( m_Config->ReadNum( wxT( "Quality" ), 2, wxT( "record" ) ) );
    m_RecQualityChoice->Enable( m_RecordChkBox->IsChecked() );
	m_RecQualityChoice->SetMinSize( wxSize( 150,-1 ) );
	RecPropFlexSizer->Add( m_RecQualityChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecPropSizer->Add( RecPropFlexSizer, 1, wxEXPAND, 5 );

	RecordSizer->Add( RecPropSizer, 1, wxEXPAND|wxALL, 5 );

	m_RecSplitChkBox = new wxCheckBox( m_RecordPanel, wxID_ANY, _( "Split tracks" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_RecSplitChkBox->SetValue( m_Config->ReadBool( wxT( "Split" ), false, wxT( "record" ) ) );
    m_RecSplitChkBox->Enable( m_RecordChkBox->IsChecked() );
	RecordSizer->Add( m_RecSplitChkBox, 0, wxALL, 5 );

	wxBoxSizer * RecDelSizer = new wxBoxSizer( wxHORIZONTAL );

	m_RecDelTracks = new wxCheckBox( m_RecordPanel, wxID_ANY, _("Delete tracks shorter than"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RecDelTracks->SetValue( m_Config->ReadBool( wxT( "DeleteTracks" ), false, wxT( "record" ) ) );

	RecDelSizer->Add( m_RecDelTracks, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_RecDelTime = new wxSpinCtrl( m_RecordPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 999,
                                    m_Config->ReadNum( wxT( "DeleteTime" ), 50, wxT( "record" ) ) );
	m_RecDelTime->Enable( m_RecDelTracks->IsChecked() );
	RecDelSizer->Add( m_RecDelTime, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * RecDelSecLabel = new wxStaticText( m_RecordPanel, wxID_ANY, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	RecDelSecLabel->Wrap( -1 );
	RecDelSizer->Add( RecDelSecLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	RecordSizer->Add( RecDelSizer, 0, wxEXPAND, 5 );

	RecMainSizer->Add( RecordSizer, 0, wxEXPAND|wxALL, 5 );

	m_RecordPanel->SetSizer( RecMainSizer );
	m_RecordPanel->Layout();
	RecMainSizer->FitInside( m_RecordPanel );

    //
    //
    //
	m_RecordChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecEnableClicked ), NULL, this );
	m_RecDelTracks->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnRecDelTracksClicked ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildAudioScrobblePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE;

    //
    // LastFM Panel
    //
	wxBoxSizer * ASMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LastFMASSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _(" Last.fm audioscrobble ") ), wxVERTICAL );

	m_LastFMASEnableChkBox = new wxCheckBox( m_LastFMPanel, wxID_ANY, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LastFMASEnableChkBox->SetValue( m_Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "lastfm" ) ) );
	LastFMASSizer->Add( m_LastFMASEnableChkBox, 0, wxALL, 5 );

    wxFlexGridSizer * ASLoginSizer = new wxFlexGridSizer( 2, 0, 0 );
	ASLoginSizer->SetFlexibleDirection( wxBOTH );
	ASLoginSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * UserNameStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Username:"), wxDefaultPosition, wxDefaultSize, 0 );
	UserNameStaticText->Wrap( -1 );
	ASLoginSizer->Add( UserNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LastFMUserNameTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_LastFMUserNameTextCtrl->SetValue( m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "lastfm" ) ) );
	ASLoginSizer->Add( m_LastFMUserNameTextCtrl, 0, wxALL, 5 );

	wxStaticText * PasswdStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	PasswdStaticText->Wrap( -1 );
	ASLoginSizer->Add( PasswdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LastFMPasswdTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PASSWORD );
	m_LastFMPasswdTextCtrl->SetValue( m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "lastfm" ) ).IsEmpty() ? wxEmptyString : wxT( "******" ) );
	// Password is saved in md5 form so we cant load it back
	ASLoginSizer->Add( m_LastFMPasswdTextCtrl, 0, wxALL, 5 );

	if( m_LastFMPasswdTextCtrl->IsEmpty() ||
	    m_LastFMUserNameTextCtrl->IsEmpty() )
        m_LastFMASEnableChkBox->Disable();

	LastFMASSizer->Add( ASLoginSizer, 1, wxEXPAND, 5 );

	ASMainSizer->Add( LastFMASSizer, 0, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * LibreFMASSizer = new wxStaticBoxSizer( new wxStaticBox( m_LastFMPanel, wxID_ANY, _(" Libre.fm audioscrobble ") ), wxVERTICAL );

	m_LibreFMASEnableChkBox = new wxCheckBox( m_LastFMPanel, wxID_ANY, _( "Enabled" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LibreFMASEnableChkBox->SetValue( m_Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "librefm" ) ) );
	LibreFMASSizer->Add( m_LibreFMASEnableChkBox, 0, wxALL, 5 );

    wxFlexGridSizer * LibreFMASLoginSizer = new wxFlexGridSizer( 2, 0, 0 );
	LibreFMASLoginSizer->SetFlexibleDirection( wxBOTH );
	LibreFMASLoginSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * LibreFMUserNameStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _( "Username:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LibreFMUserNameStaticText->Wrap( -1 );
	LibreFMASLoginSizer->Add( LibreFMUserNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LibreFMUserNameTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_LibreFMUserNameTextCtrl->SetValue( m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "librefm" ) ) );
	LibreFMASLoginSizer->Add( m_LibreFMUserNameTextCtrl, 0, wxALL, 5 );

	wxStaticText * LibreFMPasswdStaticText = new wxStaticText( m_LastFMPanel, wxID_ANY, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	LibreFMPasswdStaticText->Wrap( -1 );
	LibreFMASLoginSizer->Add( LibreFMPasswdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

	m_LibreFMPasswdTextCtrl = new wxTextCtrl( m_LastFMPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), wxTE_PASSWORD );
	m_LibreFMPasswdTextCtrl->SetValue( m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "librefm" ) ).IsEmpty() ? wxEmptyString : wxT( "******" ) );
	// Password is saved in md5 form so we cant load it back
	LibreFMASLoginSizer->Add( m_LibreFMPasswdTextCtrl, 0, wxALL, 5 );

	if( m_LibreFMPasswdTextCtrl->IsEmpty() ||
	    m_LibreFMUserNameTextCtrl->IsEmpty() )
        m_LibreFMASEnableChkBox->Disable();

	LibreFMASSizer->Add( LibreFMASLoginSizer, 0, wxEXPAND, 5 );

	ASMainSizer->Add( LibreFMASSizer, 0, wxEXPAND|wxALL, 5 );


	ASMainSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxHyperlinkCtrl * m_UserGroupLink = new wxHyperlinkCtrl( m_LastFMPanel, wxID_ANY,
        _( "Join the Guayadeque Last.fm users group" ), wxT("http://www.last.fm/group/Guayadeque"),
        wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	ASMainSizer->Add( m_UserGroupLink, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LastFMPanel->SetSizer( ASMainSizer );
	m_LastFMPanel->Layout();
	ASMainSizer->FitInside( m_LastFMPanel );

    //
    //
    //
    m_LastFMUserNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );
    m_LastFMPasswdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLastFMASUserNameChanged ), NULL, this );

    m_LibreFMUserNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );
    m_LibreFMPasswdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLibreFMASUserNameChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLyricsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LYRICS;

    //
    // Lyrics
    //
	wxBoxSizer * LyricsMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LyricsSrcSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _(" Sources " ) ), wxHORIZONTAL );

    m_LyricSearchEngine = new guLyricSearchEngine();

    wxArrayString LyricSourcesNames;
    wxArrayInt    LyricSourcesEnabled;
    int Index;
    int Count = m_LyricSearchEngine->SourcesCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSource * LyricSource = m_LyricSearchEngine->GetSource( Index );
        LyricSourcesNames.Add( LyricSource->Name() );
        LyricSourcesEnabled.Add( LyricSource->Enabled() );
    }

	m_LyricsSrcListBox = new wxCheckListBox( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1, guPREFERENCES_LISTBOX_HEIGHT ), LyricSourcesNames, 0 );
	LyricsSrcSizer->Add( m_LyricsSrcListBox, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	for( Index = 0; Index < Count; Index++ )
	{
        m_LyricsSrcListBox->Check( Index, LyricSourcesEnabled[ Index ] );
	}

	wxBoxSizer * LyricsSrcBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LyricsAddButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	//m_LyricsAddButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsAddButton, 0, wxTOP, 5 );

	m_LyricsUpButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsUpButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsUpButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsDownButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsDownButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsDownButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsDelButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsDelButton->Enable( false );
	LyricsSrcBtnSizer->Add( m_LyricsDelButton, 0, wxTOP|wxBOTTOM, 5 );

	LyricsSrcSizer->Add( LyricsSrcBtnSizer, 0, wxEXPAND, 5 );

	LyricsMainSizer->Add( LyricsSrcSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * LyricsSaveSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _( " Targets " ) ), wxHORIZONTAL );

    wxArrayString LyricTargetsNames;
    wxArrayInt    LyricTargetsEnabled;
    Count = m_LyricSearchEngine->TargetsCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSource * LyricTarget = m_LyricSearchEngine->GetTarget( Index );
        LyricTargetsNames.Add( LyricTarget->Name() );
        LyricTargetsEnabled.Add( LyricTarget->Enabled() );
    }
	m_LyricsSaveListBox = new wxCheckListBox( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1, guPREFERENCES_LISTBOX_HEIGHT ), LyricTargetsNames, 0 );
	for( Index = 0; Index < Count; Index++ )
	{
        m_LyricsSaveListBox->Check( Index, LyricTargetsEnabled[ Index ] );
	}
	LyricsSaveSizer->Add( m_LyricsSaveListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * LyricsSaveBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LyricsSaveAddButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	//m_LyricsSaveAddButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveAddButton, 0, wxTOP|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LyricsSaveUpButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveUpButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveUpButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsSaveDownButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveDownButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveDownButton, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );

	m_LyricsSaveDelButton = new wxBitmapButton( m_LyricsPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LyricsSaveDelButton->Enable( false );
	LyricsSaveBtnSizer->Add( m_LyricsSaveDelButton, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 5 );

	LyricsSaveSizer->Add( LyricsSaveBtnSizer, 0, wxEXPAND, 5 );

	LyricsMainSizer->Add( LyricsSaveSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer * LyricsFontSizer = new wxStaticBoxSizer( new wxStaticBox( m_LyricsPanel, wxID_ANY, _(" Font ") ), wxHORIZONTAL );

	wxStaticText * LyricsFontLabel = new wxStaticText( m_LyricsPanel, wxID_ANY, _( "Font:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LyricsFontLabel->Wrap( -1 );
	LyricsFontSizer->Add( LyricsFontLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

	wxFont LyricFont;
	LyricFont.SetNativeFontInfo( m_Config->ReadStr( wxT( "Font" ), wxEmptyString, wxT( "lyrics" ) ) );
	if( !LyricFont.IsOk() )
        LyricFont = GetFont();
	m_LyricFontPicker = new wxFontPickerCtrl( m_LyricsPanel, wxID_ANY, LyricFont, wxDefaultPosition, wxDefaultSize, wxFNTP_DEFAULT_STYLE );
	m_LyricFontPicker->SetMaxPointSize( 100 );
	LyricsFontSizer->Add( m_LyricFontPicker, 2, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * LyricsAlignLabel = new wxStaticText( m_LyricsPanel, wxID_ANY, _( "Align:" ), wxDefaultPosition, wxDefaultSize, 0 );
	LyricsAlignLabel->Wrap( -1 );
	LyricsFontSizer->Add( LyricsAlignLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT|wxLEFT, 5 );

	wxArrayString LyricsAlignChoices;
	LyricsAlignChoices.Add( _( "Left" ) );
	LyricsAlignChoices.Add( _( "Center" ) );
	LyricsAlignChoices.Add( _( "Right" ) );
	m_LyricsAlignChoice = new wxChoice( m_LyricsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), LyricsAlignChoices, 0 );
	m_LyricsAlignChoice->SetSelection( m_Config->ReadNum( wxT( "TextAlign" ), 1, wxT( "lyrics" ) ) );
	LyricsFontSizer->Add( m_LyricsAlignChoice, 1, wxRIGHT, 5 );

	LyricsMainSizer->Add( LyricsFontSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_LyricsPanel->SetSizer( LyricsMainSizer );
	m_LyricsPanel->Layout();
	LyricsMainSizer->FitInside( m_LyricsPanel );

	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSourceSelected ), NULL, this );
	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSourceDClicked ), NULL, this );
	m_LyricsSrcListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSourceToggled ), NULL, this );
	m_LyricsAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricAddBtnClick ), NULL, this );
	m_LyricsUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricUpBtnClick ), NULL, this );
	m_LyricsDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDownBtnClick ), NULL, this );
	m_LyricsDelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricDelBtnClick ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLyricSaveSelected ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDClicked ), NULL, this );
	m_LyricsSaveListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( guPrefDialog::OnLyricSaveToggled ), NULL, this );
	m_LyricsSaveAddButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveAddBtnClick ), NULL, this );
	m_LyricsSaveUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveUpBtnClick ), NULL, this );
	m_LyricsSaveDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDownBtnClick ), NULL, this );
	m_LyricsSaveDelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLyricSaveDelBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildOnlinePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_ONLINE;

    //
    // Online Services Filter
    //
    wxBoxSizer * OnlineMainSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer * OnlineFiltersSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Filters ") ), wxHORIZONTAL );

    m_OnlineFiltersListBox = new wxListBox( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    m_OnlineFiltersListBox->Append( m_Config->ReadAStr( wxT( "Filter" ), wxEmptyString, wxT( "searchfilters" ) ) );
    OnlineFiltersSizer->Add( m_OnlineFiltersListBox, 1, wxALL|wxEXPAND, 5 );

    wxBoxSizer * OnlineBtnSizer = new wxBoxSizer( wxVERTICAL );

    m_OnlineAddBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
    OnlineBtnSizer->Add( m_OnlineAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

    m_OnlineDelBtn = new wxBitmapButton( m_OnlinePanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_OnlineDelBtn->Disable();
    OnlineBtnSizer->Add( m_OnlineDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

    OnlineFiltersSizer->Add( OnlineBtnSizer, 0, wxEXPAND, 5 );

    OnlineMainSizer->Add( OnlineFiltersSizer, 1, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer * OnlineLangSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Language " ) ), wxHORIZONTAL );

    m_LangStaticText = new wxStaticText( m_OnlinePanel, wxID_ANY, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LangStaticText->Wrap( -1 );
    OnlineLangSizer->Add( m_LangStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_LangChoice = new wxChoice( m_OnlinePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LFMLangNames, 0 );

    int LangIndex = m_LFMLangIds.Index( m_Config->ReadStr( wxT( "Language" ), wxEmptyString, wxT( "lastfm" ) ) );
    if( LangIndex == wxNOT_FOUND ) LangIndex = 0;

    m_LangChoice->SetSelection( LangIndex );
    m_LangChoice->SetMinSize( wxSize( 250,-1 ) );
    OnlineLangSizer->Add( m_LangChoice, 0, wxALL, 5 );

    OnlineMainSizer->Add( OnlineLangSizer, 0, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer * BrowserCmdSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _(" Browser command ") ), wxHORIZONTAL );

    m_BrowserCmdTextCtrl = new wxTextCtrl( m_OnlinePanel, wxID_ANY, m_Config->ReadStr( wxT( "BrowserCommand" ), wxT( "firefox --new-tab" ), wxT( "general" ) ), wxDefaultPosition, wxDefaultSize, 0 );
    BrowserCmdSizer->Add( m_BrowserCmdTextCtrl, 1, wxALL, 5 );

    OnlineMainSizer->Add( BrowserCmdSizer, 0, wxEXPAND|wxALL, 5 );

//	m_RadioMinBitRateRadBox = new wxRadioBox( m_OnlinePanel, wxID_ANY, _( "Radio min. allowed bit rate "), wxDefaultPosition, wxDefaultSize, m_RadioMinBitRateRadBoxChoices, 10, wxRA_SPECIFY_COLS );
//	wxString MinBitRate = m_Config->ReadStr( wxT( "RadioMinBitRate" ), wxT( "128" ), wxT( "radios" ) );
//	int RadioMinBitRateIndex = m_RadioMinBitRateRadBoxChoices.Index( MinBitRate );
//    if( RadioMinBitRateIndex != wxNOT_FOUND )
//    {
//        m_RadioMinBitRateRadBox->SetSelection( RadioMinBitRateIndex );
//    }
//    else
//    {
//        m_RadioMinBitRateRadBox->SetSelection( 4 );
//    }
//	OnlineMainSizer->Add( m_RadioMinBitRateRadBox, 0, wxALL|wxEXPAND, 5 );

    m_LastMinBitRate = m_Config->ReadNum( wxT( "RadioMinBitRate" ), 128, wxT( "radios" ) );
    wxStaticBoxSizer * RadioBitRateSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Minimum radio allowed bit rate " ) ), wxHORIZONTAL );

    m_RadioMinBitRateSlider = new wxSlider( m_OnlinePanel, wxID_ANY, m_LastMinBitRate, 0, 320, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
    RadioBitRateSizer->Add( m_RadioMinBitRateSlider, 1, wxEXPAND, 5 );

    OnlineMainSizer->Add( RadioBitRateSizer, 0, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer * BufferSizeSizer = new wxStaticBoxSizer( new wxStaticBox( m_OnlinePanel, wxID_ANY, _( " Player online buffer size ") ), wxHORIZONTAL );

    m_BufferSizeSlider = new wxSlider( m_OnlinePanel, wxID_ANY, m_Config->ReadNum( wxT( "BufferSize" ), 64, wxT( "general" ) ), 32, 1024, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
    BufferSizeSizer->Add( m_BufferSizeSlider, 1, wxEXPAND, 5 );

    OnlineMainSizer->Add( BufferSizeSizer, 0, wxEXPAND|wxALL, 5 );

    m_OnlinePanel->SetSizer( OnlineMainSizer );
    m_OnlinePanel->Layout();
    OnlineMainSizer->FitInside( m_OnlinePanel );

    //
    //
    //
    m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnFiltersListBoxSelected ), NULL, this );
    m_OnlineAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineAddBtnClick ), NULL, this );
    m_OnlineDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineDelBtnClick ), NULL, this );
    m_OnlineFiltersListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( guPrefDialog::OnOnlineListBoxDClicked ), NULL, this );

    m_RadioMinBitRateSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guPrefDialog::OnOnlineMinBitRateChanged ), NULL, this );
    m_RadioMinBitRateSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guPrefDialog::OnOnlineMinBitRateChanged ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildPodcastsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PODCASTS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_PODCASTS;

    //
    // Podcasts
    //
    wxBoxSizer * PodcastsMainSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer * PodcastsSizer = new wxStaticBoxSizer( new wxStaticBox( m_PodcastPanel, wxID_ANY, _(" Podcasts ") ), wxVERTICAL );

    wxBoxSizer * PathSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText * PodcastPathStaticText = new wxStaticText( m_PodcastPanel, wxID_ANY, _("Destination directory:"), wxDefaultPosition, wxDefaultSize, 0 );
    PodcastPathStaticText->Wrap( -1 );
    PathSizer->Add( PodcastPathStaticText, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    m_PodcastPath = new wxDirPickerCtrl( m_PodcastPanel, wxID_ANY, wxEmptyString, _("Select podcasts folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE | wxDIRP_DIR_MUST_EXIST );
    m_PodcastPath->SetPath( m_Config->ReadStr( wxT( "Path" ), wxGetHomeDir(), wxT( "podcasts" ) ) );
    PathSizer->Add( m_PodcastPath, 1, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

    PodcastsSizer->Add( PathSizer, 0, wxEXPAND, 5 );

    wxBoxSizer * UpdateSizer = new wxBoxSizer( wxHORIZONTAL );

    m_PodcastUpdate = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Check every"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PodcastUpdate->SetValue( m_Config->ReadBool( wxT( "Update" ), true, wxT( "podcasts" ) ) );

    UpdateSizer->Add( m_PodcastUpdate, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    wxString m_PodcastUpdatePeriodChoices[] = { _( "Hour" ), _("Day"), _("Week"), _("Month") };
    int m_PodcastUpdatePeriodNChoices = sizeof( m_PodcastUpdatePeriodChoices ) / sizeof( wxString );
    m_PodcastUpdatePeriod = new wxChoice( m_PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastUpdatePeriodNChoices, m_PodcastUpdatePeriodChoices, 0 );
    m_PodcastUpdatePeriod->SetSelection( m_Config->ReadNum( wxT( "UpdatePeriod" ), 0, wxT( "podcasts" ) ) );
    m_PodcastUpdatePeriod->SetMinSize( wxSize( 150,-1 ) );
    UpdateSizer->Add( m_PodcastUpdatePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

    PodcastsSizer->Add( UpdateSizer, 0, wxEXPAND, 5 );

    wxBoxSizer * DeleteSizer = new wxBoxSizer( wxHORIZONTAL );

    m_PodcastDelete = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Delete after"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PodcastDelete->SetValue( m_Config->ReadBool( wxT( "Delete" ), false, wxT( "podcasts" ) ) );

    DeleteSizer->Add( m_PodcastDelete, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_PodcastDeleteTime = new wxSpinCtrl( m_PodcastPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 99,
    m_Config->ReadNum( wxT( "DeleteTime" ), 15, wxT( "podcasts" ) ) );
    DeleteSizer->Add( m_PodcastDeleteTime, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

    wxString m_PodcastDeletePeriodChoices[] = { _("Days"), _("Weeks"), _("Months") };
    int m_PodcastDeletePeriodNChoices = sizeof( m_PodcastDeletePeriodChoices ) / sizeof( wxString );
    m_PodcastDeletePeriod = new wxChoice( m_PodcastPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_PodcastDeletePeriodNChoices, m_PodcastDeletePeriodChoices, 0 );
    m_PodcastDeletePeriod->SetSelection( m_Config->ReadNum( wxT( "DeletePeriod" ), 0, wxT( "podcasts" ) ) );
    m_PodcastDeletePeriod->SetMinSize( wxSize( 150,-1 ) );
    DeleteSizer->Add( m_PodcastDeletePeriod, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

    DeleteSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_PodcastDeletePlayed = new wxCheckBox( m_PodcastPanel, wxID_ANY, _("Only if played"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PodcastDeletePlayed->SetValue( m_Config->ReadBool( wxT( "DeletePlayed" ), false, wxT( "podcasts" ) ) );

    DeleteSizer->Add( m_PodcastDeletePlayed, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    PodcastsSizer->Add( DeleteSizer, 0, wxEXPAND, 5 );

    PodcastsMainSizer->Add( PodcastsSizer, 0, wxEXPAND|wxALL, 5 );

    m_PodcastPanel->SetSizer( PodcastsMainSizer );
    m_PodcastPanel->Layout();
    PodcastsMainSizer->FitInside( m_PodcastPanel );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildJamendoPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_JAMENDO;

    //
    // Jamendo
    //
    wxBoxSizer * JamMainSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer * JamGenresSizer = new wxStaticBoxSizer( new wxStaticBox( m_JamendoPanel, wxID_ANY, _( " Genres " ) ), wxHORIZONTAL );

    wxArrayString JamendoGenres;
    int Index = 0;
    do {
        wxString GenreName = TStringTowxString( TagLib::ID3v1::genre( Index++ ) );

        if( !GenreName.IsEmpty() )
            JamendoGenres.Add( GenreName );
        else
            break;

    } while( true );

    m_LastJamendoGenres = m_Config->ReadANum( wxT( "Genre" ), 0, wxT( "jamendo/genres" ) );
    guLogMessage( wxT( "Read %li jamendo genres" ), m_LastJamendoGenres.Count() );

    m_JamGenresListBox = new wxCheckListBox( m_JamendoPanel, wxID_ANY, wxDefaultPosition, wxSize( -1, guPREFERENCES_LISTBOX_HEIGHT ), JamendoGenres, 0 );
    int Count = m_LastJamendoGenres.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( m_LastJamendoGenres[ Index ] );
        //guLogMessage( wxT( "Checking %i" ), m_LastJamendoGenres[ Index ] );
    }
    JamGenresSizer->Add( m_JamGenresListBox, 1, wxALL|wxEXPAND, 5 );

    wxBoxSizer * JamGenresBtnSizer = new wxBoxSizer( wxVERTICAL );

    m_JamSelAllBtn = new wxButton( m_JamendoPanel, wxID_ANY, _( "All" ), wxDefaultPosition, wxDefaultSize, 0 );
    JamGenresBtnSizer->Add( m_JamSelAllBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_JamSelNoneBtn = new wxButton( m_JamendoPanel, wxID_ANY, _( "None" ), wxDefaultPosition, wxDefaultSize, 0 );
    JamGenresBtnSizer->Add( m_JamSelNoneBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_JamInvertBtn = new wxButton( m_JamendoPanel, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
    JamGenresBtnSizer->Add( m_JamInvertBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    JamGenresSizer->Add( JamGenresBtnSizer, 0, wxEXPAND, 5 );

    JamMainSizer->Add( JamGenresSizer, 1, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer * JamOtherSizer = new wxStaticBoxSizer( new wxStaticBox( m_JamendoPanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

    wxFlexGridSizer * JamFlexSizer = new wxFlexGridSizer( 2, 0, 0 );
    JamFlexSizer->AddGrowableCol( 1 );
    JamFlexSizer->SetFlexibleDirection( wxBOTH );
    JamFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText * JamFormatLabel = new wxStaticText( m_JamendoPanel, wxID_ANY, _( "Audio format:" ), wxDefaultPosition, wxDefaultSize, 0 );
    JamFormatLabel->Wrap( -1 );
    JamFlexSizer->Add( JamFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT|wxALIGN_RIGHT, 5 );

    wxArrayString m_JamFormatChoices;
    m_JamFormatChoices.Add( wxT( "mp3" ) );
    m_JamFormatChoices.Add( wxT( "ogg" ) );
    m_JamFormatChoice = new wxChoice( m_JamendoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_JamFormatChoices, 0 );
    m_JamFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "jamendo" ) ) );
    JamFlexSizer->Add( m_JamFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText * JamBTCmdLabel = new wxStaticText( m_JamendoPanel, wxID_ANY, _( "Torrent command:" ), wxDefaultPosition, wxDefaultSize, 0 );
    JamBTCmdLabel->Wrap( -1 );
    JamFlexSizer->Add( JamBTCmdLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT|wxALIGN_RIGHT, 5 );

    m_JamBTCmd = new wxTextCtrl( m_JamendoPanel, wxID_ANY, m_Config->ReadStr( wxT( "TorrentCommand" ), wxT( "transmission" ), wxT( "jamendo" ) ), wxDefaultPosition, wxDefaultSize, 0 );
    JamFlexSizer->Add( m_JamBTCmd, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    JamOtherSizer->Add( JamFlexSizer, 1, wxEXPAND, 5 );

    JamMainSizer->Add( JamOtherSizer, 0, wxEXPAND|wxALL, 5 );

    m_JamendoPanel->SetSizer( JamMainSizer );
    m_JamendoPanel->Layout();
    JamMainSizer->FitInside( m_JamendoPanel );

    m_JamSelAllBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectAll ), NULL, this );
    m_JamSelNoneBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoSelectNone ), NULL, this );
    m_JamInvertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnJamendoInvertSelection ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildMagnatunePage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_MAGNATUNE;

    //
    // Magnatune
    //
    wxBoxSizer * MagMainSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer * MagGenresSizer = new wxStaticBoxSizer( new wxStaticBox( m_MagnatunePanel, wxID_ANY, _(" Genres ") ), wxHORIZONTAL );

    wxArrayString MagnatuneGenres = m_Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "magnatune/genrelist" ) );
    if( !MagnatuneGenres.Count() )
    {
        int Index = 0;
        do {
            wxString GenreName = TStringTowxString( TagLib::ID3v1::genre( Index++ ) );

            if( !GenreName.IsEmpty() )
                MagnatuneGenres.Add( GenreName );
            else
                break;

        } while( true );
    }

    m_LastMagnatuneGenres = m_Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "magnatune/genres" ) );

    m_MagGenresListBox = new wxCheckListBox( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxSize( -1, guPREFERENCES_LISTBOX_HEIGHT ), MagnatuneGenres, 0 );
    int Index;
    int Count = m_LastMagnatuneGenres.Count();
    for( Index = 0; Index < Count; Index++ )
    {
    int Pos = MagnatuneGenres.Index( m_LastMagnatuneGenres[ Index ] );
    if( Pos != wxNOT_FOUND )
        m_MagGenresListBox->Check( Pos );
    }
    MagGenresSizer->Add( m_MagGenresListBox, 1, wxALL|wxEXPAND, 5 );

    wxBoxSizer * MagGenresBtnSizer = new wxBoxSizer( wxVERTICAL );

    m_MagSelAllBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _( "All" ), wxDefaultPosition, wxDefaultSize, 0 );
    MagGenresBtnSizer->Add( m_MagSelAllBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_MagSelNoneBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _("None"), wxDefaultPosition, wxDefaultSize, 0 );
    MagGenresBtnSizer->Add( m_MagSelNoneBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_MagInvertBtn = new wxButton( m_MagnatunePanel, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
    MagGenresBtnSizer->Add( m_MagInvertBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    MagGenresSizer->Add( MagGenresBtnSizer, 0, wxEXPAND, 5 );

    MagMainSizer->Add( MagGenresSizer, 1, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer * MagOtherSizer = new wxStaticBoxSizer( new wxStaticBox( m_MagnatunePanel, wxID_ANY, wxEmptyString ), wxVERTICAL );

    wxFlexGridSizer * MagFlexSizer = new wxFlexGridSizer( 2, 0, 0 );
    MagFlexSizer->AddGrowableCol( 1 );
    MagFlexSizer->SetFlexibleDirection( wxBOTH );
    MagFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText * MagMemberLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Membership :" ), wxDefaultPosition, wxDefaultSize, 0 );
    MagMemberLabel->Wrap( -1 );
    MagFlexSizer->Add( MagMemberLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxBoxSizer * MagMembSizer = new wxBoxSizer( wxHORIZONTAL );

    m_MagNoRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "None" ), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_MagStRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "Streaming" ), wxDefaultPosition, wxDefaultSize );
    m_MagDlRadioItem = new wxRadioButton( m_MagnatunePanel, wxID_ANY, _( "Downloading" ), wxDefaultPosition, wxDefaultSize );
    int Membership = m_Config->ReadNum( wxT( "Membership" ), 0, wxT( "magnatune" ) );
    
    if( Membership == 1 ) m_MagStRadioItem->SetValue( true );
    else if( Membership == 2 ) m_MagDlRadioItem->SetValue( true );
    else m_MagNoRadioItem->SetValue( true );

    MagMembSizer->Add( m_MagNoRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
    MagMembSizer->Add( m_MagStRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
    MagMembSizer->Add( m_MagDlRadioItem, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    MagFlexSizer->Add( MagMembSizer, 1, wxEXPAND, 5 );

    wxStaticText * MagUserLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Username :" ), wxDefaultPosition, wxDefaultSize, 0 );
    MagUserLabel->Wrap( -1 );
    MagFlexSizer->Add( MagUserLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_MagUserTextCtrl = new wxTextCtrl( m_MagnatunePanel, wxID_ANY, m_Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "magnatune" ) ), wxDefaultPosition, wxDefaultSize, 0 );
    m_MagUserTextCtrl->Enable( !m_MagNoRadioItem->GetValue() );
    MagFlexSizer->Add( m_MagUserTextCtrl, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );

    wxStaticText * MagPassLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Password :" ), wxDefaultPosition, wxDefaultSize, 0 );
    MagPassLabel->Wrap( -1 );
    MagFlexSizer->Add( MagPassLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

    m_MagPassTextCtrl = new wxTextCtrl( m_MagnatunePanel, wxID_ANY, m_Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "magnatune" ) ), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    m_MagPassTextCtrl->Enable( !m_MagNoRadioItem->GetValue() );
    MagFlexSizer->Add( m_MagPassTextCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

//    wxStaticText * MagFormatLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Format :" ), wxDefaultPosition, wxDefaultSize, 0 );
//    MagFormatLabel->Wrap( -1 );
//    MagFlexSizer->Add( MagFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
//
//    wxArrayString MagFormatChoices;
//    MagFormatChoices.Add( wxT( "mp3" ) );
//    MagFormatChoices.Add( wxT( "ogg" ) );
//    m_MagFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, MagFormatChoices, 0 );
//    m_MagFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) ) );
//    MagFlexSizer->Add( m_MagFormatChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
    wxStaticText * MagFormatLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _( "Stream as :" ), wxDefaultPosition, wxDefaultSize, 0 );
    MagFormatLabel->Wrap( -1 );
    MagFlexSizer->Add( MagFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    wxBoxSizer * FormatSizer = new wxBoxSizer( wxHORIZONTAL );

    wxString m_MagFormatChoiceChoices[] = { wxT("mp3"), wxT("ogg") };
    int m_MagFormatChoiceNChoices = sizeof( m_MagFormatChoiceChoices ) / sizeof( wxString );
    m_MagFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MagFormatChoiceNChoices, m_MagFormatChoiceChoices, 0 );
    m_MagFormatChoice->SetSelection( m_Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "magnatune" ) ) );
    m_MagFormatChoice->SetMinSize( wxSize( 100,-1 ) );

    FormatSizer->Add( m_MagFormatChoice, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

    wxStaticText * MagDownLabel = new wxStaticText( m_MagnatunePanel, wxID_ANY, _("Download as :"), wxDefaultPosition, wxDefaultSize, 0 );
    MagDownLabel->Wrap( -1 );
    FormatSizer->Add( MagDownLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    wxString m_MagDownFormatChoiceChoices[] = { _( "Download Page" ), wxT("mp3 (VBR)"), wxT("mp3 (128Kbits)"), wxT("ogg"), wxT("flac"), wxT("wav") };
    int m_MagDownFormatChoiceNChoices = sizeof( m_MagDownFormatChoiceChoices ) / sizeof( wxString );
    m_MagDownFormatChoice = new wxChoice( m_MagnatunePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_MagDownFormatChoiceNChoices, m_MagDownFormatChoiceChoices, 0 );
    m_MagDownFormatChoice->SetSelection( m_Config->ReadNum( wxT( "DownloadFormat" ), 0, wxT( "magnatune" ) ) );
    m_MagDownFormatChoice->Enable( Membership == 2 );
    m_MagDownFormatChoice->SetMinSize( wxSize( 100,-1 ) );

    FormatSizer->Add( m_MagDownFormatChoice, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    MagFlexSizer->Add( FormatSizer, 1, wxEXPAND, 5 );

    MagOtherSizer->Add( MagFlexSizer, 1, wxEXPAND, 5 );

    MagMainSizer->Add( MagOtherSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_MagnatunePanel->SetSizer( MagMainSizer );
    m_MagnatunePanel->Layout();
    MagMainSizer->FitInside( m_MagnatunePanel );

    m_MagSelAllBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneSelectAll ), NULL, this );
    m_MagSelNoneBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneSelectNone ), NULL, this );
    m_MagInvertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnMagnatuneInvertSelection ), NULL, this );
    m_MagNoRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );
    m_MagStRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );
    m_MagDlRadioItem->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( guPrefDialog::OnMagNoRadioItemChanged ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildLinksPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_LINKS;

    //
    // Links
    //
	wxBoxSizer * LinksMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * LinksLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Links ") ), wxVERTICAL );

	wxBoxSizer * LinksListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_LinksListBox = new wxListBox( m_LinksPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString SearchLinks = m_Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "searchlinks/links" ) );
	m_LinksListBox->Append( SearchLinks );
	m_LinksNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "searchlinks/names" ) );
    int count = m_LinksListBox->GetCount();
	while( ( int ) m_LinksNames.Count() < count )
        m_LinksNames.Add( wxEmptyString );
	while( ( int ) m_LinksNames.Count() > count )
        m_LinksNames.RemoveAt( count );

    int index;
    for( index = 0; index < count; index++ )
    {
        if( m_LinksNames[ index ].IsEmpty() )
        {
            wxURI Uri( SearchLinks[ index ] );
            m_LinksNames[ index ] = Uri.GetServer();
        }
    }

	LinksListBoxSizer->Add( m_LinksListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * LinksBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_LinksAddBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
    m_LinksAddBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveUpBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveUpBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveUpBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksMoveDownBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksMoveDownBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksMoveDownBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LinksDelBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksDelBtn->Enable( false );
	LinksBtnSizer->Add( m_LinksDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	LinksListBoxSizer->Add( LinksBtnSizer, 0, wxEXPAND, 5 );

	LinksLabelSizer->Add( LinksListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * LinksEditorSizer = new wxBoxSizer( wxHORIZONTAL );

    wxFlexGridSizer * LinksFieldsSizer = new wxFlexGridSizer( 2, 0, 0 );
	LinksFieldsSizer->AddGrowableCol( 1 );
	LinksFieldsSizer->SetFlexibleDirection( wxBOTH );
	LinksFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * LinkUrlStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinkUrlStaticText->Wrap( -1 );
	LinksFieldsSizer->Add( LinkUrlStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinksUrlTextCtrl = new wxTextCtrl( m_LinksPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LinksFieldsSizer->Add( m_LinksUrlTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	wxStaticText * LinkNameStaticText = new wxStaticText( m_LinksPanel, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinkNameStaticText->Wrap( -1 );
	LinksFieldsSizer->Add( LinkNameStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_LinksNameTextCtrl = new wxTextCtrl( m_LinksPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LinksFieldsSizer->Add( m_LinksNameTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	LinksEditorSizer->Add( LinksFieldsSizer, 1, wxEXPAND, 5 );

	m_LinksAcceptBtn = new wxBitmapButton( m_LinksPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LinksAcceptBtn->Enable( false );

	LinksEditorSizer->Add( m_LinksAcceptBtn, 0, wxALL, 5 );

	LinksLabelSizer->Add( LinksEditorSizer, 0, wxEXPAND, 5 );

	LinksMainSizer->Add( LinksLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * LinksHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_LinksPanel, wxID_ANY, _(" Define URLs using ") ), wxHORIZONTAL );

	wxStaticText * LinksHelpText = new wxStaticText( m_LinksPanel, wxID_ANY, wxT("{lang}:\n{text}:"), wxDefaultPosition, wxDefaultSize, 0 );
	LinksHelpText->Wrap( -1 );
	LinksHelpSizer->Add( LinksHelpText, 0, wxALL, 5 );

	LinksHelpText = new wxStaticText( m_LinksPanel, wxID_ANY, _( "2 letters language code\nText to search"), wxDefaultPosition, wxDefaultSize, 0 );
	LinksHelpText->Wrap( -1 );
	LinksHelpSizer->Add( LinksHelpText, 0, wxALL, 5 );

	LinksMainSizer->Add( LinksHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_LinksPanel->SetSizer( LinksMainSizer );
	m_LinksPanel->Layout();
	LinksMainSizer->FitInside( m_LinksPanel );

    //
    //
    //
	m_LinksListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnLinksListBoxSelected ), NULL, this );
	m_LinksAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksAddBtnClick ), NULL, this );
	m_LinksDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksDelBtnClick ), NULL, this );
	m_LinksMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveUpBtnClick ), NULL, this );
	m_LinksMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinkMoveDownBtnClick ), NULL, this );
	m_LinksUrlTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnLinksTextChanged ), NULL, this );
	m_LinksAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnLinksSaveBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCommandsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_COMMANDS;

    //
    // Commands Panel
    //
	wxBoxSizer * CmdMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * CmdLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Commands ") ), wxVERTICAL );

	wxBoxSizer * CmdListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CmdListBox = new wxListBox( m_CmdPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    wxArrayString Commands = m_Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );
	m_CmdListBox->Append( Commands );
	m_CmdNames = m_Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "commands/names" ) );
	int index;
    int count = m_CmdListBox->GetCount();
	while( ( int ) m_CmdNames.Count() < count )
        m_CmdNames.Add( wxEmptyString );
    if( ( int ) m_CmdNames.Count() > count )
        m_CmdNames.RemoveAt( count, m_CmdNames.Count() - count );
    for( index = 0; index < count; index++ )
    {
        if( m_CmdNames[ index ].IsEmpty() )
        {
            m_CmdNames[ index ] = Commands[ index ].BeforeFirst( ' ' );
        }
    }

	CmdListBoxSizer->Add( m_CmdListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * CmdBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_CmdAddBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CmdAddBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdAddBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveUpBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveUpBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveUpBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdMoveDownBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdMoveDownBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdMoveDownBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CmdDelBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdDelBtn->Enable( false );
	CmdBtnSizer->Add( m_CmdDelBtn, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CmdListBoxSizer->Add( CmdBtnSizer, 0, wxEXPAND, 5 );

	CmdLabelSizer->Add( CmdListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer * CmdEditorSizer = new wxBoxSizer( wxHORIZONTAL );

    wxFlexGridSizer * CmdFieldsSizer = new wxFlexGridSizer( 2, 0, 0 );
	CmdFieldsSizer->AddGrowableCol( 1 );
	CmdFieldsSizer->SetFlexibleDirection( wxBOTH );
	CmdFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * CmdStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Command:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdStaticText->Wrap( -1 );
	CmdFieldsSizer->Add( CmdStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CmdTextCtrl = new wxTextCtrl( m_CmdPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CmdFieldsSizer->Add( m_CmdTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * CmdNameStaticText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CmdNameStaticText->Wrap( -1 );
	CmdFieldsSizer->Add( CmdNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_CmdNameTextCtrl = new wxTextCtrl( m_CmdPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CmdFieldsSizer->Add( m_CmdNameTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CmdEditorSizer->Add( CmdFieldsSizer, 1, wxEXPAND, 5 );

	m_CmdAcceptBtn = new wxBitmapButton( m_CmdPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CmdAcceptBtn->Enable( false );

	CmdEditorSizer->Add( m_CmdAcceptBtn, 0, wxALL, 5 );

	CmdLabelSizer->Add( CmdEditorSizer, 0, wxEXPAND, 5 );

	CmdMainSizer->Add( CmdLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer * CmdHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CmdPanel, wxID_ANY, _(" Define commands using ") ), wxHORIZONTAL );

	wxStaticText * CmdHelpText = new wxStaticText( m_CmdPanel, wxID_ANY, wxT( "{bp}:\n{bc}:\n{tp}:"), wxDefaultPosition, wxDefaultSize, 0 );
	CmdHelpText->Wrap( -1 );
	CmdHelpSizer->Add( CmdHelpText, 0, wxALL, 5 );

	CmdHelpText = new wxStaticText( m_CmdPanel, wxID_ANY, _( "Album path\nAlbum cover file path\nTrack path"), wxDefaultPosition, wxDefaultSize, 0 );
	CmdHelpText->Wrap( -1 );
	CmdHelpSizer->Add( CmdHelpText, 0, wxALL, 5 );

	CmdMainSizer->Add( CmdHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CmdPanel->SetSizer( CmdMainSizer );
	m_CmdPanel->Layout();
	CmdMainSizer->FitInside( m_CmdPanel );

    //
    //
    //
	m_CmdListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCmdListBoxSelected ), NULL, this );
	m_CmdAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdAddBtnClick ), NULL, this );
	m_CmdDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdDelBtnClick ), NULL, this );
	m_CmdMoveUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveUpBtnClick ), NULL, this );
	m_CmdMoveDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdMoveDownBtnClick ), NULL, this );
	m_CmdTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCmdTextChanged ), NULL, this );
	m_CmdAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCmdSaveBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildCopyToPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_COPYTO;

    //
    // Copy To patterns
    //
	wxBoxSizer * CopyToMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * CopyToLabelSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Patterns ") ), wxVERTICAL );

	wxBoxSizer * CopyToListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CopyToListBox = new wxListBox( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );

	m_CopyToOptions = new guCopyToPatternArray();

	wxArrayString Options = m_Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "copyto/options" ) );
	wxArrayString Names;
    int Index;
    int Count;
	if( ( Count = Options.Count() ) )
	{
	    for( Index = 0; Index < Count; Index++ )
	    {
	        guCopyToPattern * CopyToPattern = new guCopyToPattern( Options[ Index ] );
	        if( CopyToPattern )
	        {
	            m_CopyToOptions->Add( CopyToPattern );
	            Names.Add( CopyToPattern->m_Name );
	        }
	    }
	}

	m_CopyToListBox->Append( Names );

	CopyToListBoxSizer->Add( m_CopyToListBox, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer * CopyToBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_CopyToAddBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToAddBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToAddBtn, 0, wxTOP|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToUpBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_up ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToUpBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToUpBtn, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToDownBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_down ), wxDefaultPosition, wxDefaultSize, 0 );
	m_CopyToDownBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToDownBtn, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToDelBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyToDelBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToDelBtn, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CopyToAcceptBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_CopyToAcceptBtn->Enable( false );
	CopyToBtnSizer->Add( m_CopyToAcceptBtn, 0, wxBOTTOM|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	CopyToListBoxSizer->Add( CopyToBtnSizer, 0, wxEXPAND, 5 );

	CopyToLabelSizer->Add( CopyToListBoxSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* CopyToEditorSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer * CopyToOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Options ") ), wxHORIZONTAL );

    wxFlexGridSizer * CopyToFieldsSizer = new wxFlexGridSizer( 2, 0, 0 );
	CopyToFieldsSizer->AddGrowableCol( 1 );
	CopyToFieldsSizer->SetFlexibleDirection( wxBOTH );
	CopyToFieldsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * CopyToNameStaticText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToNameStaticText->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToNameStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5 );

	m_CopyToNameTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToNameTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * CopyToPatternStaticText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Pattern:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToPatternStaticText->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToPatternStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5 );

	m_CopyToPatternTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToPatternTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxRIGHT, 5 );

	wxStaticText * CopyToFormatLabel = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Format:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFormatLabel->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToFormatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer * CopyToFormatSizer;
	CopyToFormatSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CopyToFormatChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeFormatStrings(), 0 );
	m_CopyToFormatChoice->SetSelection( 0 );
	CopyToFormatSizer->Add( m_CopyToFormatChoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	m_CopyToQualityChoice = new wxChoice( m_CopyPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeQualityStrings(), 0 );
	m_CopyToQualityChoice->SetSelection( guTRANSCODE_QUALITY_KEEP );
	m_CopyToQualityChoice->Enable( false );
	CopyToFormatSizer->Add( m_CopyToQualityChoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	CopyToFieldsSizer->Add( CopyToFormatSizer, 1, wxEXPAND, 5 );

	wxStaticText * CopyToPathStaticText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Path:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToPathStaticText->Wrap( -1 );
	CopyToFieldsSizer->Add( CopyToPathStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5 );

    wxBoxSizer * CopyToPathSizer = new wxBoxSizer( wxHORIZONTAL );

	m_CopyToPathTextCtrl = new wxTextCtrl( m_CopyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	CopyToPathSizer->Add( m_CopyToPathTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

    m_CopyToPathBtn = new wxButton( m_CopyPanel, wxID_ANY, wxT( "..." ), wxDefaultPosition, wxSize( 28,-1 ), 0 );
    CopyToPathSizer->Add( m_CopyToPathBtn, 0, wxRIGHT, 5 );

	CopyToFieldsSizer->Add( CopyToPathSizer, 1, wxEXPAND, 5 );

	CopyToFieldsSizer->Add( 0, 0, 0, wxEXPAND, 5 );

	m_CopyToMoveFilesChkBox = new wxCheckBox( m_CopyPanel, wxID_ANY, _("Remove source files"), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToFieldsSizer->Add( m_CopyToMoveFilesChkBox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	CopyToOptionsSizer->Add( CopyToFieldsSizer, 1, wxEXPAND, 5 );

	CopyToEditorSizer->Add( CopyToOptionsSizer, 1, wxEXPAND, 5 );

//	m_CopyToAcceptBtn = new wxBitmapButton( m_CopyPanel, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_CopyToAcceptBtn->Enable( false );
//
//	CopyToEditorSizer->Add( m_CopyToAcceptBtn, 0, wxALL|wxALIGN_BOTTOM, 5 );

	CopyToLabelSizer->Add( CopyToEditorSizer, 0, wxEXPAND, 5 );

	CopyToMainSizer->Add( CopyToLabelSizer, 1, wxEXPAND|wxALL, 5 );

	wxStaticBoxSizer* CopyToHelpSizer;
	CopyToHelpSizer = new wxStaticBoxSizer( new wxStaticBox( m_CopyPanel, wxID_ANY, _(" Define patterns using ") ), wxHORIZONTAL );

	wxStaticText * CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, wxT( "{a}:\n{aa}:\n{A}:\n{b}:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 0, wxALL, 5 );

	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Artist\nAlbum Artist\n{aa} or {a}\nAlbum" ), wxDefaultPosition, wxDefaultSize, 0 );	CopyToHelpText->Wrap( -1 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 1, wxALL, 5 );

	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, wxT( "{c}:\n{d}:\n{f}:\n{g}:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 0, wxALL, 5 );

	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Composer\nDisk\nFilename\nGenre" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 1, wxALL, 5 );

	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, wxT( "{i}:\n{n}:\n{t}:\n{y}:" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 0, wxALL, 5 );

	CopyToHelpText = new wxStaticText( m_CopyPanel, wxID_ANY, _( "Index\nNumber\nTitle\nYear" ), wxDefaultPosition, wxDefaultSize, 0 );
	CopyToHelpText->Wrap( -1 );
	CopyToHelpSizer->Add( CopyToHelpText, 1, wxALL, 5 );

	CopyToMainSizer->Add( CopyToHelpSizer, 0, wxEXPAND|wxALL, 5 );

	m_CopyPanel->SetSizer( CopyToMainSizer );
	m_CopyPanel->Layout();
	CopyToMainSizer->FitInside( m_CopyPanel );

	m_CopyToListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToListBoxSelected ), NULL, this );
	m_CopyToAddBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToAddBtnClick ), NULL, this );
	m_CopyToDelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToDelBtnClick ), NULL, this );
	m_CopyToUpBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveUpBtnClick ), NULL, this );
	m_CopyToDownBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveDownBtnClick ), NULL, this );
	m_CopyToPatternTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
	m_CopyToPathTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
    m_CopyToPathBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToPathBtnClick ), NULL, this );
	m_CopyToNameTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guPrefDialog::OnCopyToTextChanged ), NULL, this );
	m_CopyToFormatChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToFormatChanged ), NULL, this );
	m_CopyToQualityChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPrefDialog::OnCopyToQualityChanged ), NULL, this );
	m_CopyToMoveFilesChkBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToMoveFilesChanged ), NULL, this );
	m_CopyToAcceptBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnCopyToSaveBtnClick ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::BuildAcceleratorsPage( void )
{
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
        return;

    m_VisiblePanels |= guPREFERENCE_PAGE_FLAG_ACCELERATORS;

    guAccelGetActionNames( m_AccelActionNames );

	m_AccelKeys = m_Config->ReadANum( wxT( "AccelKey"), 0, wxT( "accelerators" ) );
	if( !m_AccelKeys.Count() )
	{
	    guAccelGetDefaultKeys( m_AccelKeys );
	}

	while( m_AccelKeys.Count() < m_AccelActionNames.Count() )
        m_AccelKeys.Add( 0 );

    int Index;
    int Count = m_AccelActionNames.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_AccelKeyNames.Add( guAccelGetKeyCodeString( m_AccelKeys[ Index ] ) );
    }

    //
    // Accelerators Panel
    //
	wxBoxSizer * AccelMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * AccelActionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_AccelPanel, wxID_ANY, _(" Accelerators ") ), wxVERTICAL );

	m_AccelListCtrl = new wxListCtrl( m_AccelPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_VRULES );

    wxListItem AccelColumn;
    AccelColumn.SetText( _( "Action" ) );
    AccelColumn.SetImage( wxNOT_FOUND );
    m_AccelListCtrl->InsertColumn( 0, AccelColumn );

    AccelColumn.SetText( _( "Key" ) );
    AccelColumn.SetAlign( wxLIST_FORMAT_RIGHT );
    m_AccelListCtrl->InsertColumn( 1, AccelColumn );

    m_AccelListCtrl->Hide();

    wxColour EveBgColor  = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    wxColour OddBgColor;
    if( EveBgColor.Red() > 0x0A && EveBgColor.Green() > 0x0A && EveBgColor.Blue() > 0x0A )
    {
        OddBgColor.Set( EveBgColor.Red() - 0xA, EveBgColor.Green() - 0x0A, EveBgColor.Blue() - 0x0A );
    }
    else
    {
        OddBgColor.Set( EveBgColor.Red() + 0xA, EveBgColor.Green() + 0x0A, EveBgColor.Blue() + 0x0A );
    }
    for( Index = 0; Index < Count; Index++ )
    {
        long NewItem = m_AccelListCtrl->InsertItem( Index, m_AccelActionNames[ Index ], 0 );
        m_AccelListCtrl->SetItemData( NewItem, m_AccelKeys[ Index ] );
        m_AccelListCtrl->SetItemBackgroundColour( NewItem, Index & 1 ? OddBgColor : EveBgColor );
        m_AccelListCtrl->SetItem( NewItem, 1, m_AccelKeyNames[ Index ] );
    }

    m_AccelListCtrl->Show();

    m_AccelListCtrl->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_AccelListCtrl->SetColumnWidth( 1, 200 );

	AccelActionsSizer->Add( m_AccelListCtrl, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer * AccelBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_AccelDefBtn = new wxButton( m_AccelPanel, wxID_ANY, _( "Default" ), wxDefaultPosition, wxDefaultSize, 0 );
	AccelBtnSizer->Add( m_AccelDefBtn, 0, wxALIGN_RIGHT|wxRIGHT, 5 );

	AccelActionsSizer->Add( AccelBtnSizer, 0, wxEXPAND, 5 );

	AccelMainSizer->Add( AccelActionsSizer, 1, wxEXPAND|wxALL, 5 );

	m_AccelPanel->SetSizer( AccelMainSizer );
	m_AccelPanel->Layout();

	m_AccelCurIndex = wxNOT_FOUND;
	m_AccelItemNeedClear = false;

	m_AccelListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( guPrefDialog::OnAccelSelected ), NULL, this );
	m_AccelListCtrl->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( guPrefDialog::OnAccelKeyDown ), NULL, this );
	m_AccelDefBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPrefDialog::OnAccelDefaultClicked ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::SaveSettings( void )
{
    m_Config = ( guConfig * ) guConfig::Get();
    if( !m_Config )
        guLogError( wxT( "Invalid m_Config object in preferences dialog" ) );


    // Save all configurations
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        m_Config->WriteNum( wxT( "Language" ), m_MainLangCodes[ m_MainLangChoice->GetSelection() ], wxT( "general" ) );
        m_Config->WriteBool( wxT( "ShowSplashScreen" ), m_ShowSplashChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "StartMinimized" ), m_MinStartChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "LoadDefaultLayouts" ), m_IgnoreLayoutsChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "ShowTaskBarIcon" ), m_TaskIconChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "CloseToTaskBar" ), m_TaskIconChkBox->IsChecked() && m_CloseTaskBarChkBox->GetValue(), wxT( "general" ) );
        if( m_SoundMenuChkBox )
            m_Config->WriteBool( wxT( "SoundMenuIntegration" ), m_SoundMenuChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "DefaultActionEnqueue" ), m_EnqueueChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "DropFilesClearPlaylist" ), m_DropFilesChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "InstantTextSearchEnabled" ), m_InstantSearchChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "TextSearchEnterRelax" ), m_EnterSearchChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteNum( wxT( "CoverFrame" ), m_ShowCDFrameChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "SaveOnClose" ), m_SavePlayListChkBox->GetValue(), wxT( "playlist" ) );
        m_Config->WriteBool( wxT( "SaveCurrentTrackPos" ), m_SavePosCheckBox->GetValue(), wxT( "general" ) );
        m_Config->WriteNum( wxT( "MinSavePlayPosLength" ), m_MinLenSpinCtrl->GetValue(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "ShowCloseConfirm" ), m_ExitConfirmChkBox->GetValue(), wxT( "general" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LIBRARY )
    {
        guMediaCollectionArray  OtherCollections;
        m_Config->LoadCollections( &OtherCollections, guMEDIA_COLLECTION_TYPE_JAMENDO );
        m_Config->LoadCollections( &OtherCollections, guMEDIA_COLLECTION_TYPE_MAGNATUNE );

        m_Config->SaveCollections( &m_Collections );

        m_Config->SaveCollections( &OtherCollections, false );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        m_Config->WriteBool( wxT( "RndPlayOnEmptyPlayList" ), m_RndPlayChkBox->GetValue(), wxT( "general" ) );
        m_Config->WriteNum( wxT( "RndModeOnEmptyPlayList" ), m_RndModeChoice->GetSelection(), wxT( "general" ) );
        m_Config->WriteBool( wxT( "DelTracksPlayed" ), m_DelPlayChkBox->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "ReplayGainMode" ), m_PlayReplayModeChoice->GetSelection(), wxT( "general" ) );
        m_Config->WriteNum( wxT( "ReplayGainPreAmp" ), m_PlayPreAmpLevelSlider->GetValue(), wxT( "general" ) );

        m_Config->WriteBool( wxT( "SilenceDetector" ), m_PlayLevelEnabled->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "SilenceLevel" ), m_PlayLevelSlider->GetValue(), wxT( "playback" ) );
        m_Config->WriteBool( wxT( "SilenceAtEnd" ), m_PlayEndTimeCheckBox->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "SilenceEndTime" ), m_PlayEndTimeSpinCtrl->GetValue(), wxT( "playback" ) );
        m_Config->WriteBool( wxT( "ShowNotifications" ), m_NotifyChkBox->GetValue(), wxT( "general" ) );

        m_Config->WriteNum( wxT( "MinTracksToPlay" ), m_MinTracksSpinCtrl->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "NumTracksToAdd" ), m_NumTracksSpinCtrl->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "MaxTracksPlayed" ), m_MaxTracksPlayed->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "SmartFilterArtists" ), m_SmartPlayArtistsSpinCtrl->GetValue(), wxT( "playback" ) );
        m_Config->WriteNum( wxT( "SmartFilterTracks" ), m_SmartPlayTracksSpinCtrl->GetValue(), wxT( "playback" ) );

        m_Config->WriteNum( wxT( "OutputDevice" ), m_PlayOutDevChoice->GetSelection(), wxT( "playback" ) );
        m_Config->WriteStr( wxT( "OutputDeviceName" ), m_PlayOutDevName->GetValue(), wxT( "playback" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_CROSSFADER )
    {
        m_Config->WriteNum( wxT( "FadeOutTime" ), m_XFadeOutLenSlider->GetValue(), wxT( "crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInTime" ), m_XFadeInLenSlider->GetValue(), wxT( "crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInVolStart" ), m_XFadeInStartSlider->GetValue(), wxT( "crossfader" ) );
        m_Config->WriteNum( wxT( "FadeInVolTriger" ), m_XFadeInTrigerSlider->GetValue(), wxT( "crossfader" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE )
    {
        m_Config->WriteBool( wxT( "SubmitEnabled" ), m_LastFMASEnableChkBox->IsEnabled() && m_LastFMASEnableChkBox->GetValue(), wxT( "lastfm" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_LastFMUserNameTextCtrl->GetValue(), wxT( "lastfm" ) );
        if( !m_LastFMPasswdTextCtrl->IsEmpty() && m_LastFMPasswdTextCtrl->GetValue() != wxT( "******" ) )
        {
            guMD5 MD5;
            m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_LastFMPasswdTextCtrl->GetValue() ), wxT( "lastfm" ) );
            //guLogMessage( wxT( "Pass: %s" ), PasswdTextCtrl->GetValue().c_str() );
            //guLogMessage( wxT( "MD5 : %s" ), MD5.MD5( PasswdTextCtrl->GetValue() ).c_str() );
        }
        m_Config->WriteBool( wxT( "SubmitEnabled" ), m_LibreFMASEnableChkBox->IsEnabled() && m_LibreFMASEnableChkBox->GetValue(), wxT( "librefm" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_LibreFMUserNameTextCtrl->GetValue(), wxT( "librefm" ) );
        if( !m_LibreFMPasswdTextCtrl->IsEmpty() && m_LibreFMPasswdTextCtrl->GetValue() != wxT( "******" ) )
        {
            guMD5 MD5;
            m_Config->WriteStr( wxT( "Password" ), MD5.MD5( m_LibreFMPasswdTextCtrl->GetValue() ), wxT( "librefm" ) );
        }
    }

    // LastFM Panel Info language
    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ONLINE )
    {
        m_Config->WriteStr( wxT( "Language" ), m_LFMLangIds[ m_LangChoice->GetSelection() ], wxT( "lastfm" ) );

        m_Config->WriteAStr( wxT( "Filter" ), m_OnlineFiltersListBox->GetStrings(), wxT( "searchfilters" ) );

        m_Config->WriteStr( wxT( "BrowserCommand" ), m_BrowserCmdTextCtrl->GetValue(), wxT( "general" ) );
        //m_Config->WriteStr( wxT( "RadioMinBitRate" ), m_RadioMinBitRateRadBoxChoices[ m_RadioMinBitRateRadBox->GetSelection() ], wxT( "radios" ) );
        m_Config->WriteNum( wxT( "RadioMinBitRate" ), m_RadioMinBitRateSlider->GetValue(), wxT( "radios" ) );
        m_Config->WriteNum( wxT( "BufferSize" ), m_BufferSizeSlider->GetValue(), wxT( "general" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_RECORD )
    {
        m_Config->WriteBool( wxT( "Enabled" ), m_RecordChkBox->GetValue(), wxT( "record" ) );
        m_Config->WriteStr( wxT( "Path" ), m_RecSelDirPicker->GetPath(), wxT( "record" ) );
        m_Config->WriteNum( wxT( "Format" ), m_RecFormatChoice->GetSelection(), wxT( "record" ) );
        m_Config->WriteNum( wxT( "Quality" ), m_RecQualityChoice->GetSelection(), wxT( "record" ) );
        m_Config->WriteBool( wxT( "Split" ), m_RecSplitChkBox->GetValue(), wxT( "record" ) );
        m_Config->WriteBool( wxT( "DeleteTracks" ), m_RecDelTracks->GetValue(), wxT( "record" ) );
        m_Config->WriteNum( wxT( "DeleteTime" ), m_RecDelTime->GetValue(), wxT( "record" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_PODCASTS )
    {
        m_Config->WriteStr( wxT( "Path" ), m_PodcastPath->GetPath(), wxT( "podcasts" ) );
        m_Config->WriteBool( wxT( "Update" ), m_PodcastUpdate->GetValue(), wxT( "podcasts" ) );
        m_Config->WriteNum( wxT( "UpdatePeriod" ), m_PodcastUpdatePeriod->GetSelection(), wxT( "podcasts" ) );
        m_Config->WriteBool( wxT( "Delete" ), m_PodcastDelete->GetValue(), wxT( "podcasts" ) );
        m_Config->WriteNum( wxT( "DeleteTime" ), m_PodcastDeleteTime->GetValue(), wxT( "podcasts" ) );
        m_Config->WriteNum( wxT( "DeletePeriod" ), m_PodcastDeletePeriod->GetSelection(), wxT( "podcasts" ) );
        m_Config->WriteBool( wxT( "DeletePlayed" ), m_PodcastDeletePlayed->GetValue(), wxT( "podcasts" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LYRICS )
    {
        m_LyricSearchEngine->Save();
        m_Config->WriteStr( wxT( "Font" ), m_LyricFontPicker->GetSelectedFont().GetNativeFontInfoDesc(), wxT( "lyrics" ) );
        m_Config->WriteNum( wxT( "TextAlign" ), m_LyricsAlignChoice->GetSelection(), wxT( "lyrics" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        wxArrayInt EnabledGenres;
        int Index;
        int Count = m_JamGenresListBox->GetCount();
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_JamGenresListBox->IsChecked( Index ) )
                EnabledGenres.Add( Index );
        }

        m_Config->WriteANum( wxT( "Genre" ), EnabledGenres, wxT( "jamendo/genres" ) );
        bool DoUpgrade = ( EnabledGenres.Count() != m_LastJamendoGenres.Count() );
        if( !DoUpgrade )
        {
            Count = EnabledGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( m_LastJamendoGenres.Index( EnabledGenres[ Index ] ) == wxNOT_FOUND )
                {
                    DoUpgrade = true;
                    break;
                }
            }
        }

        m_Config->WriteBool( wxT( "NeedUpgrade" ), DoUpgrade, wxT( "jamendo" ) );
        m_Config->WriteNum( wxT( "AudioFormat" ), m_JamFormatChoice->GetSelection(), wxT( "jamendo" ) );
        m_Config->WriteStr( wxT( "TorrentCommand" ), m_JamBTCmd->GetValue(), wxT( "jamendo" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
    {
        wxArrayString EnabledGenres;
        int Index;
        int Count = m_MagGenresListBox->GetCount();
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_MagGenresListBox->IsChecked( Index ) )
                EnabledGenres.Add( m_MagGenresListBox->GetString( Index ) );
        }
        m_Config->WriteAStr( wxT( "Genre" ), EnabledGenres, wxT( "magnatune/genres" ) );

        bool DoUpgrade = ( EnabledGenres.Count() != m_LastMagnatuneGenres.Count() );
        if( !DoUpgrade )
        {
            Count = EnabledGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( m_LastMagnatuneGenres.Index( EnabledGenres[ Index ] ) == wxNOT_FOUND )
                {
                    DoUpgrade = true;
                    break;
                }
            }
        }

        m_Config->WriteBool( wxT( "NeedUpgrade" ), DoUpgrade, wxT( "magnatune" ) );
        if( m_MagNoRadioItem->GetValue() )
            m_Config->WriteNum( wxT( "Membership" ), 0, wxT( "magnatune" ) );
        else if( m_MagStRadioItem->GetValue() )
            m_Config->WriteNum( wxT( "Membership" ), 1, wxT( "magnatune" ) );
        else
            m_Config->WriteNum( wxT( "Membership" ), 2, wxT( "magnatune" ) );
        m_Config->WriteStr( wxT( "UserName" ), m_MagUserTextCtrl->GetValue(), wxT( "magnatune" ) );
        m_Config->WriteStr( wxT( "Password" ), m_MagPassTextCtrl->GetValue(), wxT( "magnatune" ) );
        m_Config->WriteNum( wxT( "AudioFormat" ), m_MagFormatChoice->GetSelection(), wxT( "magnatune" ) );
        m_Config->WriteNum( wxT( "DownloadFormat" ), m_MagDownFormatChoice->GetSelection(), wxT( "magnatune" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_LINKS )
    {
        wxArrayString SearchLinks = m_LinksListBox->GetStrings();
        m_Config->WriteAStr( wxT( "Link" ), SearchLinks, wxT( "searchlinks/links" ) );
        m_Config->WriteAStr( wxT( "Name" ), m_LinksNames, wxT( "searchlinks/names" ), false );

        // TODO : Make this process in a thread
        int index;
        int count = SearchLinks.Count();
        for( index = 0; index < count; index++ )
        {
            wxURI Uri( SearchLinks[ index ] );
            if( !wxDirExists( guPATH_LINKICONS ) )
            {
                wxMkdir( guPATH_LINKICONS, 0770 );
            }
            wxString IconFile = guPATH_LINKICONS + Uri.GetServer() + wxT( ".ico" );
            if( !wxFileExists( IconFile ) )
            {
                if( DownloadFile( Uri.GetServer() + wxT( "/favicon.ico" ), IconFile ) )
                {
                    wxImage Image( IconFile, wxBITMAP_TYPE_ANY );
                    if( Image.IsOk() )
                    {
                        if( Image.GetWidth() > 25 || Image.GetHeight() > 25 )
                        {
                            Image.Rescale( 25, 25, wxIMAGE_QUALITY_HIGH );
                        }
                        if( Image.IsOk() )
                        {
                            Image.SaveFile( IconFile, wxBITMAP_TYPE_ICO );
                        }
                    }
                }
                else
                {
                    guLogError( wxT( "Coult not get the icon from SearchLink server '%s'" ), Uri.GetServer().c_str() );
                }
            }
        }
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COMMANDS )
    {
        wxArrayString Commands = m_CmdListBox->GetStrings();
        m_Config->WriteAStr( wxT( "Exec" ), Commands, wxT( "commands/execs" ) );
        m_Config->WriteAStr( wxT( "Name" ), m_CmdNames, wxT( "commands/names" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_COPYTO )
    {
        wxArrayString Options;
        int Index;
        int Count = m_CopyToOptions->Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( Index );
            Options.Add( CopyToPattern.ToString() );
        }

        m_Config->WriteAStr( wxT( "Option" ), Options, wxT( "copyto/options" ) );
    }

    if( m_VisiblePanels & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        m_Config->WriteANum( wxT( "AccelKey" ), m_AccelKeys, wxT( "accelerators" ) );
    }

    m_Config->Flush();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateTaskBarIcon( wxCommandEvent& event )
{
    if( m_SoundMenuChkBox )
        m_SoundMenuChkBox->Enable( m_TaskIconChkBox->IsChecked() );
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && ( !m_SoundMenuChkBox || !m_SoundMenuChkBox->IsChecked() ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateSoundMenuIntegration( wxCommandEvent& event )
{
    m_CloseTaskBarChkBox->Enable( m_TaskIconChkBox->IsChecked() && !m_SoundMenuChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnActivateInstantSearch( wxCommandEvent& event )
{
    m_EnterSearchChkBox->Enable( m_InstantSearchChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRndPlayClicked( wxCommandEvent& event )
{
    m_RndModeChoice->Enable( m_RndPlayChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnDelPlayedTracksChecked( wxCommandEvent& event )
{
    m_MaxTracksPlayed->Enable( !m_DelPlayChkBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibOptionsLoadControls( void )
{
    bool CollectSelected = ( m_CollectSelected != wxNOT_FOUND ) &&
                           ( m_Collections[ m_CollectSelected ].m_Type == guMEDIA_COLLECTION_TYPE_NORMAL );

    m_PathSelected = wxNOT_FOUND;
    m_CoverSelected = wxNOT_FOUND;

    m_LibPathListBox->Clear();
    m_LibCoverListBox->Clear();
    if( CollectSelected )
    {
        guMediaCollection &CurCollection = m_Collections[ m_CollectSelected ];
        int Index;
        int Count = CurCollection.m_Paths.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            m_LibPathListBox->Append( CurCollection.m_Paths[ Index ] );
        }

        Count = CurCollection.m_CoverWords.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            m_LibCoverListBox->Append( CurCollection.m_CoverWords[ Index ] );
        }

        m_LibOptAutoUpdateChkBox->SetValue( CurCollection.m_UpdateOnStart );
        m_LibOptCreatePlayListChkBox->SetValue( CurCollection.m_ScanPlaylists );
        m_LibOptFollowLinksChkBox->SetValue( CurCollection.m_ScanFollowSymLinks );
        m_LibOptCheckEmbeddedChkBox->SetValue( CurCollection.m_ScanEmbeddedCovers );
        m_LibOptEmbedTagsChkBox->SetValue( CurCollection.m_EmbeddMetadata );
        if( !m_LibOptCopyToChoice->SetStringSelection( CurCollection.m_DefaultCopyAction ) )
            m_LibOptCopyToChoice->SetSelection( 0 );
    }

    m_LibCollectUpBtn->Enable( m_CollectSelected > 0 );
    m_LibCollectDownBtn->Enable( m_CollectSelected != wxNOT_FOUND && ( m_CollectSelected <  int( m_Collections.Count() - 1 ) ) );
    m_LibCollectDelBtn->Enable( m_CollectSelected != wxNOT_FOUND && ( m_Collections.Count() > 1 ) );
    //
	m_LibPathListBox->Enable( CollectSelected );
	m_LibOptAddPathBtn->Enable( CollectSelected );
	m_LibOptDelPathBtn->Enable( CollectSelected && ( m_PathSelected != wxNOT_FOUND ) );
	m_LibCoverListBox->Enable( CollectSelected );
    m_LibOptAddCoverBtn->Enable( CollectSelected );
	m_LibOptUpCoverBtn->Enable( CollectSelected && ( m_CoverSelected != wxNOT_FOUND ) );
	m_LibOptDownCoverBtn->Enable( CollectSelected && ( m_CoverSelected != wxNOT_FOUND ) );
	m_LibOptDelCoverBtn->Enable( CollectSelected && ( m_CoverSelected != wxNOT_FOUND ) );

    m_LibOptAutoUpdateChkBox->Enable( CollectSelected );
    m_LibOptCreatePlayListChkBox->Enable( CollectSelected );
    m_LibOptFollowLinksChkBox->Enable( CollectSelected );
    m_LibOptCheckEmbeddedChkBox->Enable( CollectSelected );
    m_LibOptEmbedTagsChkBox->Enable( CollectSelected );
    m_LibOptCopyToChoice->Enable( CollectSelected );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCollectSelected( wxCommandEvent& event )
{
    m_CollectSelected = event.GetInt();
    guLogMessage( wxT( "guPrefDialog::OnLibCollectSelected( %i )" ), m_CollectSelected );

    OnLibOptionsLoadControls();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCollectDClicked( wxCommandEvent& event )
{
    int CollectIndex = event.GetInt();
    if( CollectIndex != wxNOT_FOUND &&
        ( m_Collections[ CollectIndex ].m_Type == guMEDIA_COLLECTION_TYPE_NORMAL ) )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Collection: " ), _( "Enter the collection name" ), m_LibCollectListBox->GetString( CollectIndex ) );
        if( EntryDialog )
        {
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                wxString NewName = EntryDialog->GetValue();
                if( m_LibCollectListBox->FindString( NewName ) == wxNOT_FOUND )
                {
                    m_LibCollectListBox->SetString( CollectIndex, NewName );
                    m_Collections[ m_CollectSelected ].m_Name = NewName;
                }
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibAddCollectClick( wxCommandEvent& event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Collection: " ), _( "Enter the collection name" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            wxString NewName = EntryDialog->GetValue();
            if( !NewName.IsEmpty() && ( m_LibCollectListBox->FindString( NewName ) == wxNOT_FOUND ) )
            {
                m_LibCollectListBox->Append( NewName );
                guMediaCollection * Collection = new guMediaCollection();
                Collection->m_Name = NewName;
                if( m_Collections.Count() )
                {
                    Collection->m_CoverWords = m_Collections[ 0 ].m_CoverWords;
                }
                else
                {
                    Collection->m_CoverWords.Add( wxT( "cover" ) );
                    Collection->m_CoverWords.Add( wxT( "front" ) );
                    Collection->m_CoverWords.Add( wxT( "folder" ) );
                }
                Collection->m_UpdateOnStart = false;
                Collection->m_ScanPlaylists = true;
                Collection->m_ScanFollowSymLinks = false;
                Collection->m_ScanEmbeddedCovers = true;
                Collection->m_EmbeddMetadata = false;
                Collection->m_LastUpdate = wxNOT_FOUND;
                m_Collections.Add( Collection );

                m_CollectSelected = m_LibCollectListBox->GetCount() - 1;
                m_LibCollectListBox->SetSelection( m_CollectSelected );
                OnLibOptionsLoadControls();

            }
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibUpCollectClick( wxCommandEvent& event )
{
    wxString SavedLabel = m_LibCollectListBox->GetString( m_CollectSelected );

    guMediaCollection * SavedCollection = m_Collections.Detach( m_CollectSelected );
    m_LibCollectListBox->SetString( m_CollectSelected, m_LibCollectListBox->GetString( m_CollectSelected - 1 ) );

    m_CollectSelected--;

    m_LibCollectListBox->SetString( m_CollectSelected, SavedLabel );
    m_Collections.Insert( SavedCollection, m_CollectSelected );

    m_LibCollectListBox->SetSelection( m_CollectSelected );

    event.SetInt( m_CollectSelected );
    OnLibCollectSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDownCollectClick( wxCommandEvent& event )
{
    wxString SavedLabel = m_LibCollectListBox->GetString( m_CollectSelected );

    guMediaCollection * SavedCollection = m_Collections.Detach( m_CollectSelected );
    m_LibCollectListBox->SetString( m_CollectSelected, m_LibCollectListBox->GetString( m_CollectSelected + 1 ) );

    m_CollectSelected++;

    m_LibCollectListBox->SetString( m_CollectSelected, SavedLabel );
    m_Collections.Insert( SavedCollection, m_CollectSelected );

    m_LibCollectListBox->SetSelection( m_CollectSelected );

    event.SetInt( m_CollectSelected );
    OnLibCollectSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDelCollectClick( wxCommandEvent& event )
{
    if( m_CollectSelected != wxNOT_FOUND )
    {
        if( !guRemoveDir( guPATH_COLLECTIONS + m_Collections[ m_CollectSelected ].m_UniqueId ) )
            guLogMessage( wxT( "Could not delete the collection folder '%s'" ), m_Collections[ m_CollectSelected ].m_UniqueId.c_str() );
        m_Collections.RemoveAt( m_CollectSelected );
        m_LibCollectListBox->Delete( m_CollectSelected );
        //m_CollectSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibPathSelected( wxCommandEvent& event )
{
    m_PathSelected = event.GetInt();
    guLogMessage( wxT( "guPrefDialog::OnLibPathSelected( %i )" ), m_PathSelected );
    m_LibOptDelPathBtn->Enable( ( m_CollectSelected != wxNOT_FOUND ) &&
                                ( m_PathSelected != wxNOT_FOUND ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibPathDClicked( wxCommandEvent& event )
{
    int PathIndex = event.GetInt();
    guLogMessage( wxT( "guPrefDialog::OnLibPathDClicked( %i )" ), PathIndex );
    if( PathIndex != wxNOT_FOUND )
    {
        wxDirDialog * DirDialog = new wxDirDialog( this, _( "Change library path" ), m_LibPathListBox->GetString( PathIndex ) );
        if( DirDialog )
        {
            if( DirDialog->ShowModal() == wxID_OK )
            {
                wxString NewPath = DirDialog->GetPath();
                if( m_LibPathListBox->FindString( NewPath, true ) == wxNOT_FOUND )
                {
                    m_LibPathListBox->SetString( PathIndex, NewPath + wxT( "/" ) );
                    m_Collections[ m_CollectSelected ].m_Paths[ PathIndex ] = NewPath + wxT( "/" );
                    m_LibPathsChanged = true;
                }
            }
            DirDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibAddPathBtnClick( wxCommandEvent& event )
{
    guLogMessage( wxT( "guPrefDialog::OnLibAddPathBtnClick( %i )" ), m_PathSelected );
    wxDirDialog * DirDialog = new wxDirDialog( this, _( "Select library path" ), wxGetHomeDir() );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            wxString NewPath = DirDialog->GetPath();
            if( m_LibPathListBox->FindString( NewPath, true ) == wxNOT_FOUND )
            {
                m_LibPathListBox->Append( NewPath + wxT( "/" ) );
                m_Collections[ m_CollectSelected ].m_Paths.Add( NewPath + wxT( "/" ) );
                m_LibPathsChanged = true;
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDelPathBtnClick( wxCommandEvent& event )
{
    guLogMessage( wxT( "guPrefDialog::OnLibDelPathBtnClick( %i )" ), m_CoverSelected );
    if( m_PathSelected != wxNOT_FOUND )
    {
        m_Collections[ m_CollectSelected ].m_Paths.RemoveAt( m_PathSelected );
        m_LibPathListBox->Delete( m_PathSelected );
        //m_PathSelected = wxNOT_FOUND;
        m_LibPathsChanged = true;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCoverSelected( wxCommandEvent& event )
{
    m_CoverSelected = event.GetInt();
    guLogMessage( wxT( "guPrefDialog::OnLibCoverSelected( %i )" ), m_CoverSelected );
    if( m_CoverSelected != wxNOT_FOUND )
    {
        m_LibOptUpCoverBtn->Enable( m_CoverSelected > 0 );
        m_LibOptDownCoverBtn->Enable( m_CoverSelected < int( m_LibCoverListBox->GetCount() - 1 ) );
        m_LibOptDelCoverBtn->Enable();
    }
    else
    {
        m_LibOptUpCoverBtn->Disable();
        m_LibOptDownCoverBtn->Disable();
        m_LibOptDelCoverBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCoverDClicked( wxCommandEvent& event )
{
    int CoverIndex = event.GetInt();
    guLogMessage( wxT( "guPrefDialog::OnLibCoverDClicked( %i )" ), CoverIndex );
    if( CoverIndex != wxNOT_FOUND )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Word: " ), _( "Enter the text to find covers" ), m_LibCoverListBox->GetString( CoverIndex ) );
        if( EntryDialog )
        {
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                wxString NewWord = EntryDialog->GetValue().Lower();
                if( m_LibCoverListBox->FindString( NewWord ) == wxNOT_FOUND )
                {
                    m_LibCoverListBox->SetString( CoverIndex, NewWord );
                    m_Collections[ m_CollectSelected ].m_CoverWords[ CoverIndex ] = NewWord;
                }
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibAddCoverBtnClick( wxCommandEvent& event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Word: " ), _( "Enter the text to find covers" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            wxString NewWord = EntryDialog->GetValue().Lower();
            if( m_LibCoverListBox->FindString( NewWord ) == wxNOT_FOUND )
            {
                m_LibCoverListBox->Append( NewWord );
                m_Collections[ m_CollectSelected ].m_CoverWords.Add( NewWord );
            }
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibUpCoverBtnClick( wxCommandEvent& event )
{
    wxString CoverWord = m_LibCoverListBox->GetString( m_CoverSelected );
    m_LibCoverListBox->SetString( m_CoverSelected, m_LibCoverListBox->GetString( m_CoverSelected - 1 ) );
    guMediaCollection &Collection = m_Collections[ m_CollectSelected ];
    Collection.m_CoverWords[ m_CoverSelected ] = Collection.m_CoverWords[ m_CoverSelected - 1 ];
    m_CoverSelected--;
    m_LibCoverListBox->SetString( m_CoverSelected, CoverWord );
    Collection.m_CoverWords[ m_CoverSelected ] = CoverWord;

    m_LibCoverListBox->SetSelection( m_CoverSelected );

    event.SetInt( m_CoverSelected );
    OnLibCoverSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDownCoverBtnClick( wxCommandEvent& event )
{
    wxString CoverWord = m_LibCoverListBox->GetString( m_CoverSelected );
    m_LibCoverListBox->SetString( m_CoverSelected, m_LibCoverListBox->GetString( m_CoverSelected + 1 ) );
    guMediaCollection &Collection = m_Collections[ m_CollectSelected ];
    Collection.m_CoverWords[ m_CoverSelected ] = Collection.m_CoverWords[ m_CoverSelected + 1 ];
    m_CoverSelected++;
    m_LibCoverListBox->SetString( m_CoverSelected, CoverWord );
    Collection.m_CoverWords[ m_CoverSelected ] = CoverWord;

    m_LibCoverListBox->SetSelection( m_CoverSelected );

    event.SetInt( m_CoverSelected );
    OnLibCoverSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDelCoverBtnClick( wxCommandEvent& event )
{
    if( m_CoverSelected != wxNOT_FOUND )
    {
        m_Collections[ m_CollectSelected ].m_CoverWords.RemoveAt( m_CoverSelected );
        m_LibCoverListBox->Delete( m_CoverSelected );
        //m_CoverSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibAutoUpdateChanged( wxCommandEvent& event )
{
    m_Collections[ m_CollectSelected ].m_UpdateOnStart = event.IsChecked();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCreatePlayListsChanged( wxCommandEvent& event )
{
    m_Collections[ m_CollectSelected ].m_ScanPlaylists = event.IsChecked();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibFollowSymLinksChanged( wxCommandEvent& event )
{
    m_Collections[ m_CollectSelected ].m_ScanFollowSymLinks = event.IsChecked();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibCheckEmbeddedChanged( wxCommandEvent& event )
{
    m_Collections[ m_CollectSelected ].m_ScanEmbeddedCovers = event.IsChecked();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibEmbeddMetadataChanged( wxCommandEvent& event )
{
    m_Collections[ m_CollectSelected ].m_EmbeddMetadata = event.IsChecked();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibDefaultCopyToChanged( wxCommandEvent& event )
{
//    guLogMessage( wxT( "Selected: %i" ), event.GetInt() );
    m_Collections[ m_CollectSelected ].m_DefaultCopyAction = m_LibOptCopyToChoice->GetString( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSourceSelected( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Selected %i" ), event.GetInt() );
    m_LyricSourceSelected = event.GetInt();
    m_LyricsUpButton->Enable( m_LyricSourceSelected > 0 );
    m_LyricsDownButton->Enable( m_LyricSourceSelected >= 0 && ( m_LyricSourceSelected < ( ( int ) m_LyricSearchEngine->SourcesCount() - 1 ) ) );
    m_LyricsDelButton->Enable( m_LyricSourceSelected != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSourceDClicked( wxCommandEvent &event )
{
    guLyricSource * CurrentLyricSource = m_LyricSearchEngine->GetSource( m_LyricSourceSelected );
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, CurrentLyricSource );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            bool WasActive = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );
            m_LyricsSrcListBox->SetString( m_LyricSourceSelected, CurrentLyricSource->Name() );
            m_LyricsSrcListBox->Check( m_LyricSourceSelected, WasActive );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSourceToggled( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Toggled %i %i" ), event.GetInt(), m_LyricsSrcListBox->IsChecked( event.GetInt() ) );
    m_LyricSourceSelected = event.GetInt();
    guLyricSource * LyricSource = m_LyricSearchEngine->GetSource( m_LyricSourceSelected );
    LyricSource->Enabled( m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricAddBtnClick( wxCommandEvent &event )
{
    guLyricSource LyricSource;
    LyricSource.Type( guLYRIC_SOURCE_TYPE_DOWNLOAD );
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, &LyricSource );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            m_LyricsSrcListBox->Append( LyricSource.Name() );
            m_LyricSearchEngine->SourceAdd( new guLyricSource( LyricSource ) );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricUpBtnClick( wxCommandEvent &event )
{
    wxString LyricSource = m_LyricsSrcListBox->GetString( m_LyricSourceSelected );
    bool     LyricChecked = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );

    m_LyricSearchEngine->SourceMoveUp( m_LyricSourceSelected );

    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, m_LyricsSrcListBox->GetString( m_LyricSourceSelected - 1 ) );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected - 1 ) );
    m_LyricSourceSelected--;
    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, LyricSource );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, LyricChecked );
    m_LyricsSrcListBox->SetSelection( m_LyricSourceSelected );


    event.SetInt( m_LyricSourceSelected );
    OnLyricSourceSelected( event );

}
// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricDownBtnClick( wxCommandEvent &event )
{
    wxString LyricSource = m_LyricsSrcListBox->GetString( m_LyricSourceSelected );
    bool     LyricChecked = m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected );

    m_LyricSearchEngine->SourceMoveDown( m_LyricSourceSelected );

    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, m_LyricsSrcListBox->GetString( m_LyricSourceSelected + 1 ) );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, m_LyricsSrcListBox->IsChecked( m_LyricSourceSelected + 1 ) );
    m_LyricSourceSelected++;
    m_LyricsSrcListBox->SetString( m_LyricSourceSelected, LyricSource );
    m_LyricsSrcListBox->Check( m_LyricSourceSelected, LyricChecked );
    m_LyricsSrcListBox->SetSelection( m_LyricSourceSelected );

    event.SetInt( m_LyricSourceSelected );
    OnLyricSourceSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricDelBtnClick( wxCommandEvent &event )
{
    if( m_LyricSourceSelected != wxNOT_FOUND )
    {
        m_LyricSearchEngine->SourceRemoveAt( m_LyricSourceSelected );
        m_LyricsSrcListBox->Delete( m_LyricSourceSelected );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveSelected( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Selected %i" ), event.GetInt() );
    m_LyricTargetSelected = event.GetInt();
    m_LyricsSaveUpButton->Enable( m_LyricTargetSelected > 0 );
    m_LyricsSaveDownButton->Enable( m_LyricTargetSelected >= 0 && ( m_LyricTargetSelected < ( ( int ) m_LyricSearchEngine->TargetsCount() - 1 ) ) );
    m_LyricsSaveDelButton->Enable( m_LyricTargetSelected != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDClicked( wxCommandEvent &event )
{
    guLyricSource * CurrentLyricTarget = m_LyricSearchEngine->GetTarget( m_LyricTargetSelected );
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, CurrentLyricTarget, true );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            bool WasActive = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );
            m_LyricsSaveListBox->SetString( m_LyricTargetSelected, CurrentLyricTarget->Name() );
            m_LyricsSaveListBox->Check( m_LyricTargetSelected, WasActive );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveToggled( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Toggled %i %i" ), event.GetInt(), m_LyricsSaveListBox->IsChecked( event.GetInt() ) );
    m_LyricTargetSelected = event.GetInt();
    guLyricSource * LyricTarget = m_LyricSearchEngine->GetTarget( m_LyricTargetSelected );
    LyricTarget->Enabled( m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected ) );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveAddBtnClick( wxCommandEvent &event )
{
    guLyricSource LyricTarget;
    guLyricSourceEditor * LyricSourceEditor = new guLyricSourceEditor( this, &LyricTarget, true );
    if( LyricSourceEditor )
    {
        if( LyricSourceEditor->ShowModal() == wxID_OK )
        {
            LyricSourceEditor->UpdateLyricSource();

            m_LyricsSaveListBox->Append( LyricTarget.Name() );
            m_LyricSearchEngine->TargetAdd( new guLyricSource( LyricTarget ) );
        }

        LyricSourceEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveUpBtnClick( wxCommandEvent &event )
{
    wxString LyricTarget = m_LyricsSaveListBox->GetString( m_LyricTargetSelected );
    bool     LyricChecked = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );

    m_LyricSearchEngine->TargetMoveUp( m_LyricTargetSelected );

    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, m_LyricsSaveListBox->GetString( m_LyricTargetSelected - 1 ) );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected - 1 ) );
    m_LyricTargetSelected--;
    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, LyricTarget );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, LyricChecked );
    m_LyricsSaveListBox->SetSelection( m_LyricTargetSelected );


    event.SetInt( m_LyricTargetSelected );
    OnLyricSourceSelected( event );

}
// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDownBtnClick( wxCommandEvent &event )
{
    wxString LyricTarget = m_LyricsSaveListBox->GetString( m_LyricTargetSelected );
    bool     LyricChecked = m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected );

    m_LyricSearchEngine->TargetMoveDown( m_LyricTargetSelected );

    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, m_LyricsSaveListBox->GetString( m_LyricTargetSelected + 1 ) );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, m_LyricsSaveListBox->IsChecked( m_LyricTargetSelected + 1 ) );
    m_LyricTargetSelected++;
    m_LyricsSaveListBox->SetString( m_LyricTargetSelected, LyricTarget );
    m_LyricsSaveListBox->Check( m_LyricTargetSelected, LyricChecked );
    m_LyricsSaveListBox->SetSelection( m_LyricTargetSelected );

    event.SetInt( m_LyricTargetSelected );
    OnLyricSourceSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLyricSaveDelBtnClick( wxCommandEvent &event )
{
    if( m_LyricTargetSelected != wxNOT_FOUND )
    {
        m_LyricSearchEngine->TargetRemoveAt( m_LyricTargetSelected );
        m_LyricsSaveListBox->Delete( m_LyricTargetSelected );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnFiltersListBoxSelected( wxCommandEvent &event )
{
    m_FilterSelected = event.GetInt();
    if( m_FilterSelected != wxNOT_FOUND )
    {
        m_OnlineDelBtn->Enable();
    }
    else
    {
        m_OnlineDelBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayLevelEnabled( wxCommandEvent& event )
{
	m_PlayLevelSlider->Enable( event.IsChecked() );
	m_PlayLevelVal->Enable( event.IsChecked() );
	m_PlayEndTimeCheckBox->Enable( event.IsChecked() );
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() && m_PlayEndTimeCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnReplayGainModeChanged( wxCommandEvent &event )
{
    bool Enabled = m_PlayReplayModeChoice->GetSelection();
    m_PlayPreAmpLevelVal->Enable( Enabled );
    m_PlayPreAmpLevelSlider->Enable( Enabled );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayPreAmpLevelValueChanged( wxScrollEvent &event )
{
    int Value = m_PlayPreAmpLevelSlider->GetValue();
    m_PlayPreAmpLevelVal->SetLabel( wxString::Format( wxT( "%idb" ), Value ) );
    m_PlayPreAmpLevelVal->GetParent()->GetSizer()->Layout();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayLevelValueChanged( wxScrollEvent &event )
{
    int Value = m_PlayLevelSlider->GetValue();
    m_PlayLevelVal->SetLabel( wxString::Format( wxT( "%02idb" ), Value ) );
    m_PlayLevelVal->GetParent()->GetSizer()->Layout();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayEndTimeEnabled( wxCommandEvent& event )
{
	m_PlayEndTimeSpinCtrl->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnPlayOutDevChanged( wxCommandEvent& event )
{
    m_PlayOutDevName->Enable( event.GetInt() > guOUTPUT_DEVICE_GCONF );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCrossFadeChanged( wxScrollEvent& event )
{
    bool IsEnabled = m_XFadeOutLenSlider->GetValue();
    m_XFadeInLenSlider->Enable( IsEnabled );
    m_XFadeInStartSlider->Enable( IsEnabled );
    m_XFadeInTrigerSlider->Enable( IsEnabled );

    double FadeOutLen = double( m_XFadeOutLenSlider->GetValue() ) / 10.0;
    double FadeInLen = double( m_XFadeInLenSlider->GetValue() ) / 10.0;
    double FadeInTriger = double( m_XFadeInTrigerSlider->GetValue() ) / 10.0;
    double FadeInVolStart = double( m_XFadeInStartSlider->GetValue() ) / 10.0;

    m_XFadeOutLenVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeOutLen ) );
    m_XFadeInLenVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInLen ) );
    m_XFadeInStartVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInVolStart ) );
    m_XFadeTrigerVal->SetLabel( wxString::Format( wxT( "%2.1f" ), FadeInTriger ) );

    m_XFadeInLenVal->Enable( IsEnabled );
    m_XFadeInStartVal->Enable( IsEnabled );
    m_XFadeTrigerVal->Enable( IsEnabled );

    m_XFadeTrigerVal->GetParent()->GetSizer()->Layout();

    wxBitmap * FadeBitmap = new wxBitmap( 400, 200 );
    if( FadeBitmap )
    {
        if( FadeBitmap->IsOk() )
        {
            wxMemoryDC MemDC;
            MemDC.SelectObject( * FadeBitmap );
            MemDC.Clear();

            wxRect Rect( 0, 0, 400, 200 );

            wxPoint FadeOutPoints[ 4 ];
            FadeOutPoints[ 0 ] = wxPoint( 0, 0 );
            FadeOutPoints[ 1 ] = wxPoint( 0, 200 );
            FadeOutPoints[ 2 ] = wxPoint( ( FadeOutLen + 1 ) * 20, 200 );
            FadeOutPoints[ 3 ] = wxPoint( 20, 0 );
            wxRegion OutRegion( WXSIZEOF( FadeOutPoints ), FadeOutPoints );

            wxPoint FadeInPoints[ 5 ];
            int FadeInStartX = FadeOutPoints[ 2 ].x - ( FadeInTriger * ( ( FadeOutPoints[ 2 ].x - 20 ) / 10 ) );
            int FadeInStartY = IsEnabled ? ( 200 - ( FadeInVolStart * 20 ) ) : 0;
            FadeInPoints[ 0 ] = wxPoint( FadeInStartX, 200 );
            FadeInPoints[ 1 ] = wxPoint( FadeInStartX, FadeInStartY );
            FadeInPoints[ 2 ] = wxPoint( FadeInStartX + ( FadeInLen * 20 ), 0 );
            FadeInPoints[ 3 ] = wxPoint( 400, 0 );
            FadeInPoints[ 4 ] = wxPoint( 400, 200 );
            wxRegion InRegion( WXSIZEOF( FadeInPoints ), FadeInPoints );
            MemDC.SetDeviceClippingRegion( InRegion );
            MemDC.GradientFillLinear( Rect, * wxLIGHT_GREY, * wxGREEN, wxRIGHT );
            MemDC.DestroyClippingRegion();

            MemDC.SetDeviceClippingRegion( OutRegion );
            MemDC.GradientFillLinear( Rect, wxColour( 0, 200, 200 ), wxColour( 0, 128, 128 ), wxRIGHT );
            MemDC.DestroyClippingRegion();

            OutRegion.Subtract( InRegion );
            MemDC.SetDeviceClippingRegion( OutRegion );
            MemDC.GradientFillLinear( Rect, * wxBLUE, * wxLIGHT_GREY, wxRIGHT );
            MemDC.DestroyClippingRegion();

            m_FadeBitmap->SetBitmap( * FadeBitmap );
            m_FadeBitmap->Refresh();
        }
        delete FadeBitmap;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRecEnableClicked( wxCommandEvent& event )
{
	m_RecSelDirPicker->Enable( event.IsChecked() );
	m_RecFormatChoice->Enable( event.IsChecked() );
	m_RecQualityChoice->Enable( event.IsChecked() );
	m_RecSplitChkBox->Enable( event.IsChecked() );
	m_RecDelTracks->Enable( event.IsChecked() );
	m_RecDelTime->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnRecDelTracksClicked( wxCommandEvent& event )
{
	m_RecDelTime->Enable( event.IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLastFMASUserNameChanged( wxCommandEvent &event )
{
    if( m_LastFMUserNameTextCtrl->IsEmpty() || m_LastFMPasswdTextCtrl->IsEmpty() )
    {
        m_LastFMASEnableChkBox->Disable();
    }
    else
    {
        m_LastFMASEnableChkBox->Enable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLibreFMASUserNameChanged( wxCommandEvent &event )
{
    if( m_LibreFMUserNameTextCtrl->IsEmpty() || m_LibreFMPasswdTextCtrl->IsEmpty() )
    {
        m_LibreFMASEnableChkBox->Disable();
    }
    else
    {
        m_LibreFMASEnableChkBox->Enable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineAddBtnClick( wxCommandEvent& event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter: " ), _( "Enter the text to filter" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            if( m_OnlineFiltersListBox->FindString( EntryDialog->GetValue(), true ) == wxNOT_FOUND )
            {
                m_OnlineFiltersListBox->Append( EntryDialog->GetValue() );
            }
        }
        EntryDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineDelBtnClick( wxCommandEvent& event )
{
    if( m_FilterSelected != wxNOT_FOUND )
    {
        m_OnlineFiltersListBox->Delete( m_FilterSelected );
        //m_FilterSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineListBoxDClicked( wxCommandEvent &event )
{
    int index = event.GetInt();
    if( index != wxNOT_FOUND )
    {
        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter: " ), _( "Edit the text to filter" ), m_OnlineFiltersListBox->GetString( index ) );
        if( EntryDialog )
        {
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                if( m_OnlineFiltersListBox->FindString( EntryDialog->GetValue(), true ) == wxNOT_FOUND )
                {
                    m_OnlineFiltersListBox->SetString( index, EntryDialog->GetValue() );
                }
            }
            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnOnlineMinBitRateChanged( wxScrollEvent &event )
{
    int CurPosition = event.GetPosition();
    //guLogMessage( wxT( "RadioMinBitrate: %i => %i" ), CurPosition, m_LastMinBitRate );
    if( m_LastMinBitRate != CurPosition )
    {
        if( CurPosition > m_LastMinBitRate )
        {
            if( CurPosition > 256 )
                CurPosition = 320;
            else if( CurPosition > 192 )
                CurPosition = 256;
            else if( CurPosition > 160 )
                CurPosition = 192;
            else if( CurPosition > 128 )
                CurPosition = 160;
            else if( CurPosition > 96 )
                CurPosition = 128;
            else if( CurPosition > 64 )
                CurPosition = 96;
            else if( CurPosition > 32 )
                CurPosition = 64;
            else if( CurPosition > 16 )
                CurPosition = 32;
            else if( CurPosition > 0 )
                CurPosition = 16;
            else
                CurPosition = 0;
        }
        else
        {
            if( CurPosition < 16 )
                CurPosition = 0;
            else if( CurPosition < 32 )
                CurPosition = 16;
            else if( CurPosition < 64 )
                CurPosition = 32;
            else if( CurPosition < 96 )
                CurPosition = 64;
            else if( CurPosition < 128 )
                CurPosition = 96;
            else if( CurPosition < 192 )
                CurPosition = 128;
            else if( CurPosition < 256 )
                CurPosition = 192;
            else if( CurPosition < 320 )
                CurPosition = 256;
            else
                CurPosition = 320;
        }

        m_LastMinBitRate = CurPosition;

        m_RadioMinBitRateSlider->SetValue( CurPosition );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnJamendoSelectAll( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnJamendoSelectNone( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnJamendoInvertSelection( wxCommandEvent& event )
{
    int Index;
    int Count = m_JamGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_JamGenresListBox->Check( Index, !m_JamGenresListBox->IsChecked( Index ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneSelectAll( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneSelectNone( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index, false );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagnatuneInvertSelection( wxCommandEvent& event )
{
    int Index;
    int Count = m_MagGenresListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_MagGenresListBox->Check( Index, !m_MagGenresListBox->IsChecked( Index ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnMagNoRadioItemChanged( wxCommandEvent& event )
{
    bool Enabled = !m_MagNoRadioItem->GetValue();
    m_MagUserTextCtrl->Enable( Enabled );
    m_MagPassTextCtrl->Enable( Enabled );
    m_MagDownFormatChoice->Enable( m_MagDlRadioItem->GetValue() );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksListBoxSelected( wxCommandEvent &event )
{
    m_LinkSelected = event.GetInt();
    if( m_LinkSelected != wxNOT_FOUND )
    {
        m_LinksDelBtn->Enable();

        if( m_LinkSelected > 0 )
            m_LinksMoveUpBtn->Enable();
        else
            m_LinksMoveUpBtn->Disable();

        if( m_LinkSelected < ( int ) ( m_LinksListBox->GetCount() - 1 ) )
            m_LinksMoveDownBtn->Enable();
        else
            m_LinksMoveDownBtn->Disable();

        m_LinksUrlTextCtrl->SetValue( m_LinksListBox->GetString( m_LinkSelected ) );
        m_LinksNameTextCtrl->SetValue( m_LinksNames[ m_LinkSelected ] );
        m_LinksAcceptBtn->Disable();
    }
    else
    {
        m_LinksDelBtn->Disable();
        m_LinksMoveUpBtn->Disable();
        m_LinksMoveDownBtn->Disable();
        m_LinksAcceptBtn->Disable();
        m_LinksUrlTextCtrl->SetValue( wxEmptyString );
        m_LinksNameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksAddBtnClick( wxCommandEvent& event )
{
    wxString Url = m_LinksUrlTextCtrl->GetValue();
    if( !Url.IsEmpty() )
    {
        m_LinksListBox->Append( m_LinksUrlTextCtrl->GetValue() );
        m_LinksNames.Add( m_LinksNameTextCtrl->GetValue() );

        m_LinkSelected = m_LinksNames.Count() - 1;
        m_LinksListBox->SetSelection( m_LinkSelected );
        event.SetInt( m_LinkSelected );
        OnLinksListBoxSelected( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksDelBtnClick( wxCommandEvent& event )
{
    if( m_LinkSelected != wxNOT_FOUND )
    {
        m_LinksNames.RemoveAt( m_LinkSelected );
        m_LinksListBox->Delete( m_LinkSelected );
        //m_LinkSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinkMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_LinksListBox->GetString( m_LinkSelected );
    wxString CurName = m_LinksNames[ m_LinkSelected ];
    m_LinksListBox->SetString( m_LinkSelected, m_LinksListBox->GetString( m_LinkSelected - 1 ) );
    m_LinksNames[ m_LinkSelected ] = m_LinksNames[ m_LinkSelected - 1 ];
    m_LinkSelected--;
    m_LinksListBox->SetString( m_LinkSelected, CurUrl );
    m_LinksNames[ m_LinkSelected ] = CurName;
    m_LinksListBox->SetSelection( m_LinkSelected );

    event.SetInt( m_LinkSelected );
    OnLinksListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinkMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_LinksListBox->GetString( m_LinkSelected );
    wxString CurName = m_LinksNames[ m_LinkSelected ];
    m_LinksListBox->SetString( m_LinkSelected, m_LinksListBox->GetString( m_LinkSelected + 1 ) );
    m_LinksNames[ m_LinkSelected ] = m_LinksNames[ m_LinkSelected + 1 ];
    m_LinkSelected++;
    m_LinksListBox->SetString( m_LinkSelected, CurUrl );
    m_LinksNames[ m_LinkSelected ] = CurName;
    m_LinksListBox->SetSelection( m_LinkSelected );

    event.SetInt( m_LinkSelected );
    OnLinksListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksTextChanged( wxCommandEvent &event )
{
    if( !m_LinksUrlTextCtrl->IsEmpty() )
    {
        m_LinksAddBtn->Enable();
        if( m_LinkSelected != wxNOT_FOUND )
            m_LinksAcceptBtn->Enable();
    }
    else
    {
        m_LinksAddBtn->Disable();
        if( m_LinkSelected != wxNOT_FOUND )
            m_LinksAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnLinksSaveBtnClick( wxCommandEvent &event )
{
    m_LinksListBox->SetString( m_LinkSelected, m_LinksUrlTextCtrl->GetValue() );
    m_LinksNames[ m_LinkSelected ] = m_LinksNameTextCtrl->GetValue();
    if( m_LinksNames[ m_LinkSelected ].IsEmpty() )
    {
        wxURI Uri( m_LinksUrlTextCtrl->GetValue() );
        m_LinksNames[ m_LinkSelected ] = Uri.GetServer();
        m_LinksNameTextCtrl->SetValue( Uri.GetServer() );
    }
    m_LinksAcceptBtn->Disable();
}


// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdListBoxSelected( wxCommandEvent &event )
{
    m_CmdSelected = event.GetInt();
    if( m_CmdSelected != wxNOT_FOUND )
    {
        m_CmdDelBtn->Enable();

        if( m_CmdSelected > 0 )
            m_CmdMoveUpBtn->Enable();
        else
            m_CmdMoveUpBtn->Disable();

        if( m_CmdSelected < ( int ) ( m_CmdListBox->GetCount() - 1 ) )
            m_CmdMoveDownBtn->Enable();
        else
            m_CmdMoveDownBtn->Disable();

        m_CmdTextCtrl->SetValue( m_CmdListBox->GetString( m_CmdSelected ) );
        m_CmdNameTextCtrl->SetValue( m_CmdNames[ m_CmdSelected ] );
        m_CmdAcceptBtn->Disable();
    }
    else
    {
        m_CmdDelBtn->Disable();
        m_CmdMoveUpBtn->Disable();
        m_CmdMoveDownBtn->Disable();
        m_CmdAcceptBtn->Disable();
        m_CmdTextCtrl->SetValue( wxEmptyString );
        m_CmdNameTextCtrl->SetValue( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdAddBtnClick( wxCommandEvent& event )
{
    wxString Cmd = m_CmdTextCtrl->GetValue();
    if( !Cmd.IsEmpty() )
    {
        m_CmdListBox->Append( m_CmdTextCtrl->GetValue() );
        m_CmdNames.Add( m_CmdNameTextCtrl->GetValue() );

        m_CmdSelected = m_CmdNames.Count() - 1;
        m_CmdListBox->SetSelection( m_CmdSelected );
        event.SetInt( m_CmdSelected );
        OnCmdListBoxSelected( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdDelBtnClick( wxCommandEvent& event )
{
    if( m_CmdSelected != wxNOT_FOUND )
    {
        m_CmdNames.RemoveAt( m_CmdSelected );
        m_CmdListBox->Delete( m_CmdSelected );
        //m_CmdSelected = wxNOT_FOUND;
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_CmdListBox->GetString( m_CmdSelected );
    wxString CurName = m_CmdNames[ m_CmdSelected ];
    m_CmdListBox->SetString( m_CmdSelected, m_CmdListBox->GetString( m_CmdSelected - 1 ) );
    m_CmdNames[ m_CmdSelected ] = m_CmdNames[ m_CmdSelected - 1 ];
    m_CmdSelected--;
    m_CmdListBox->SetString( m_CmdSelected, CurUrl );
    m_CmdNames[ m_CmdSelected ] = CurName;
    m_CmdListBox->SetSelection( m_CmdSelected );

    event.SetInt( m_CmdSelected );
    OnCmdListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurUrl = m_CmdListBox->GetString( m_CmdSelected );
    wxString CurName = m_CmdNames[ m_CmdSelected ];
    m_CmdListBox->SetString( m_CmdSelected, m_CmdListBox->GetString( m_CmdSelected + 1 ) );
    m_CmdNames[ m_CmdSelected ] = m_CmdNames[ m_CmdSelected + 1 ];
    m_CmdSelected++;
    m_CmdListBox->SetString( m_CmdSelected, CurUrl );
    m_CmdNames[ m_CmdSelected ] = CurName;
    m_CmdListBox->SetSelection( m_CmdSelected );

    event.SetInt( m_CmdSelected );
    OnCmdListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdTextChanged( wxCommandEvent &event )
{
    if( !m_CmdTextCtrl->IsEmpty() )
    {
        m_CmdAddBtn->Enable();
        if( m_CmdSelected != wxNOT_FOUND )
            m_CmdAcceptBtn->Enable();
    }
    else
    {
        m_CmdAddBtn->Disable();
        if( m_CmdSelected != wxNOT_FOUND )
            m_CmdAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCmdSaveBtnClick( wxCommandEvent &event )
{
    m_CmdListBox->SetString( m_CmdSelected, m_CmdTextCtrl->GetValue() );
    m_CmdNames[ m_CmdSelected ] = m_CmdNameTextCtrl->GetValue();
    if( m_CmdNames[ m_CmdSelected ].IsEmpty() )
    {
        m_CmdNames[ m_CmdSelected ] = m_CmdTextCtrl->GetValue().BeforeFirst( ' ' );
        m_CmdNameTextCtrl->SetValue( m_CmdNames[ m_CmdSelected ] );
    }
    m_CmdAcceptBtn->Disable();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToListBoxSelected( wxCommandEvent &event )
{
    m_CopyToSelected = event.GetInt();
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        m_CopyToDelBtn->Enable();

        if( m_CopyToSelected > 0 )
            m_CopyToUpBtn->Enable();
        else
            m_CopyToUpBtn->Disable();

        if( m_CopyToSelected < ( int ) ( m_CopyToListBox->GetCount() - 1 ) )
            m_CopyToDownBtn->Enable();
        else
            m_CopyToDownBtn->Disable();

        guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
        m_CopyToNameTextCtrl->SetValue( CopyToPattern.m_Name );
        m_CopyToPatternTextCtrl->SetValue( CopyToPattern.m_Pattern );
        m_CopyToPathTextCtrl->SetValue( CopyToPattern.m_Path );
        m_CopyToFormatChoice->SetSelection( CopyToPattern.m_Format );
        m_CopyToQualityChoice->SetSelection( CopyToPattern.m_Quality );
        m_CopyToQualityChoice->Enable( CopyToPattern.m_Format );
        m_CopyToMoveFilesChkBox->SetValue( CopyToPattern.m_MoveFiles );

        m_CopyToAcceptBtn->Disable();
    }
    else
    {
        m_CopyToDelBtn->Disable();
        m_CopyToUpBtn->Disable();
        m_CopyToDownBtn->Disable();
        m_CopyToAcceptBtn->Disable();
        m_CopyToPatternTextCtrl->SetValue( wxEmptyString );
        m_CopyToPathTextCtrl->SetValue( wxEmptyString );
        m_CopyToNameTextCtrl->SetValue( wxEmptyString );
        m_CopyToFormatChoice->SetSelection( 0 );
        m_CopyToQualityChoice->SetSelection( 0 );
        m_CopyToQualityChoice->Disable();
        m_CopyToMoveFilesChkBox->SetValue( false );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToAddBtnClick( wxCommandEvent& event )
{
    wxString Cmd = m_CopyToPatternTextCtrl->GetValue();
    if( !Cmd.IsEmpty() )
    {
        m_CopyToListBox->Append( m_CopyToNameTextCtrl->GetValue() );
        guCopyToPattern * CopyToPattern = new guCopyToPattern();
        CopyToPattern->m_Name = m_CopyToNameTextCtrl->GetValue();
        CopyToPattern->m_Pattern = m_CopyToPatternTextCtrl->GetValue();
        CopyToPattern->m_Path = m_CopyToPathTextCtrl->GetValue();
        CopyToPattern->m_Format = m_CopyToFormatChoice->GetSelection();
        CopyToPattern->m_Quality = m_CopyToQualityChoice->GetSelection();
        CopyToPattern->m_MoveFiles = m_CopyToMoveFilesChkBox->GetValue();
        m_CopyToOptions->Add( CopyToPattern );

        UpdateCopyToOptions();

        m_CopyToSelected = m_CopyToOptions->Count() - 1;
        m_CopyToListBox->SetSelection( m_CopyToSelected );
        event.SetInt( m_CopyToSelected );
        OnCopyToListBoxSelected( event );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToDelBtnClick( wxCommandEvent& event )
{
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        m_CopyToOptions->RemoveAt( m_CopyToSelected );
        m_CopyToListBox->Delete( m_CopyToSelected );
        //m_CopyToSelected = wxNOT_FOUND;

        UpdateCopyToOptions();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveUpBtnClick( wxCommandEvent &event )
{
    wxString CurName = m_CopyToListBox->GetString( m_CopyToSelected );
    guCopyToPattern CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToListBox->GetString( m_CopyToSelected - 1 ) );
    m_CopyToOptions->Item( m_CopyToSelected ) = m_CopyToOptions->Item( m_CopyToSelected - 1 );
    m_CopyToSelected--;
    m_CopyToListBox->SetString( m_CopyToSelected, CurName );
    m_CopyToOptions->Item( m_CopyToSelected ) = CopyToPattern;
    m_CopyToListBox->SetSelection( m_CopyToSelected );

    UpdateCopyToOptions();

    event.SetInt( m_CopyToSelected );
    OnCopyToListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveDownBtnClick( wxCommandEvent &event )
{
    wxString CurName = m_CopyToListBox->GetString( m_CopyToSelected );
    guCopyToPattern CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToListBox->GetString( m_CopyToSelected + 1 ) );
    m_CopyToOptions->Item( m_CopyToSelected ) = m_CopyToOptions->Item( m_CopyToSelected + 1 );
    m_CopyToSelected++;
    m_CopyToListBox->SetString( m_CopyToSelected, CurName );
    m_CopyToOptions->Item( m_CopyToSelected ) = CopyToPattern;
    m_CopyToListBox->SetSelection( m_CopyToSelected );

    UpdateCopyToOptions();

    event.SetInt( m_CopyToSelected );
    OnCopyToListBoxSelected( event );
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToTextChanged( wxCommandEvent &event )
{
    if( !m_CopyToPatternTextCtrl->IsEmpty() )
    {
        m_CopyToAddBtn->Enable();
        if( m_CopyToSelected != wxNOT_FOUND )
            m_CopyToAcceptBtn->Enable();
    }
    else
    {
        m_CopyToAddBtn->Disable();
        if( m_CopyToSelected != wxNOT_FOUND )
            m_CopyToAcceptBtn->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToPathBtnClick( wxCommandEvent &event )
{
    wxString CurPath = m_CopyToPathTextCtrl->GetValue();
    if( !CurPath.EndsWith( wxT( "/" ) ) )
        CurPath.Append( wxT( "/" ) );

    wxDirDialog * DirDialog = new wxDirDialog( this,
                        _( "Select path" ), CurPath, wxDD_DIR_MUST_EXIST );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            m_CopyToPathTextCtrl->SetValue( DirDialog->GetPath() + wxT( "/" ) );
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToFormatChanged( wxCommandEvent &event )
{
    m_CopyToQualityChoice->Enable( m_CopyToFormatChoice->GetSelection() );
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToQualityChanged( wxCommandEvent &event )
{
    m_CopyToQualityChoice->Enable( m_CopyToFormatChoice->GetSelection() );
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToMoveFilesChanged( wxCommandEvent &event )
{
    if( m_CopyToSelected != wxNOT_FOUND )
    {
        if( !m_CopyToPatternTextCtrl->IsEmpty() )
        {
            m_CopyToAcceptBtn->Enable();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnCopyToSaveBtnClick( wxCommandEvent &event )
{
    m_CopyToListBox->SetString( m_CopyToSelected, m_CopyToNameTextCtrl->GetValue() );
    guCopyToPattern &CopyToPattern = m_CopyToOptions->Item( m_CopyToSelected );
    CopyToPattern.m_Name = m_CopyToNameTextCtrl->GetValue();
    CopyToPattern.m_Pattern = m_CopyToPatternTextCtrl->GetValue();
    CopyToPattern.m_Path = m_CopyToPathTextCtrl->GetValue();
    CopyToPattern.m_Format = m_CopyToFormatChoice->GetSelection();
    CopyToPattern.m_Quality = m_CopyToQualityChoice->GetSelection();
    CopyToPattern.m_MoveFiles = m_CopyToMoveFilesChkBox->GetValue();

    if( CopyToPattern.m_Name.IsEmpty() )
    {
        CopyToPattern.m_Name = CopyToPattern.m_Pattern;
        m_CopyToNameTextCtrl->SetValue( CopyToPattern.m_Pattern );
    }
    m_CopyToAcceptBtn->Disable();

    UpdateCopyToOptions();
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::UpdateCopyToOptions( void )
{
    if( m_LibOptCopyToChoice )
    {
        wxString CurSelected = m_LibOptCopyToChoice->GetStringSelection();

        m_LibOptCopyToChoice->Clear();
        m_LibOptCopyToChoice->Append( wxEmptyString );

        int Index;
        int Count;
        if( m_CopyToOptions )
        {
            Count = m_CopyToOptions->Count();
            for( Index = 0; Index < Count; Index++ )
            {
                m_LibOptCopyToChoice->Append( m_CopyToOptions->Item( Index ).m_Name );
            }
        }
        else
        {
            wxArrayString CopyToOptions = m_Config->ReadAStr( wxT( "Option" ), wxEmptyString, wxT( "copyto/options" ) );
            Count = CopyToOptions.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                m_LibOptCopyToChoice->Append( CopyToOptions[ Index ].BeforeFirst( wxT( ':' ) ) );
            }
        }

        m_LibOptCopyToChoice->SetStringSelection( CurSelected );
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAccelSelected( wxListEvent &event )
{
    if( m_AccelItemNeedClear )
    {
        m_AccelItemNeedClear = false;
        m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxEmptyString );
    }

    m_AccelCurIndex = event.GetIndex();
    //guLogMessage( wxT( "Selected Accel %i" ), m_AccelCurIndex );
    if( m_AccelCurIndex != wxNOT_FOUND )
    {
        m_AccelLastKey = m_AccelKeys[ m_AccelCurIndex ];
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAccelKeyDown( wxKeyEvent &event )
{
    if( m_AccelCurIndex != wxNOT_FOUND )
    {
        //guLogMessage( wxT( "Mod : %08X Key: %08X %i %c" ), event.GetModifiers(), event.GetKeyCode(), event.GetKeyCode(), event.GetKeyCode() );
        int Modifiers = event.GetModifiers();
        int KeyCode = event.GetKeyCode();

        switch( KeyCode )
        {
            case WXK_SHIFT :
            case WXK_ALT :
            case WXK_CONTROL :
            case WXK_MENU :
            case WXK_PAUSE :
            case WXK_CAPITAL :
                event.Skip();
                return;

            default :
                break;
        }

        if( Modifiers == 0 )
        {
            switch( KeyCode )
            {
                case WXK_HOME :
                case WXK_END :
                case WXK_PAGEUP :
                case WXK_PAGEDOWN :
                case WXK_UP :
                case WXK_DOWN :
                    event.Skip();
                    return;

                case WXK_ESCAPE :
                    if( m_AccelLastKey != m_AccelKeys[ m_AccelCurIndex ] )
                    {
                        m_AccelKeys[ m_AccelCurIndex ] = m_AccelLastKey;
                        m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, guAccelGetKeyCodeString( m_AccelLastKey ) );
                    }
                    return;

                case WXK_DELETE :
                    m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxEmptyString );
                    m_AccelKeys[ m_AccelCurIndex ] = 0;
                    return;

                default :
                    break;
            }

            if( wxIsalnum( KeyCode ) || wxIsprint( KeyCode ) )
            {
                return;
            }
        }

        int AccelCurKey = ( Modifiers << 16 ) | KeyCode;
        int KeyIndex = m_AccelKeys.Index( AccelCurKey );
        if( ( KeyIndex == wxNOT_FOUND ) || ( KeyIndex == m_AccelCurIndex ) )
        {
            if( m_AccelItemNeedClear )
            {
                m_AccelItemNeedClear = false;
            }
            m_AccelKeys[ m_AccelCurIndex ] = AccelCurKey;
            m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, guAccelGetKeyCodeString( AccelCurKey ) );
        }
        else
        {
            if( !m_AccelItemNeedClear )
            {
                m_AccelItemNeedClear = true;
                m_AccelListCtrl->SetItem( m_AccelCurIndex, 1, wxString( _( "Key used by '" ) ) + m_AccelActionNames[ KeyIndex ] + wxT( "'") );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPrefDialog::OnAccelDefaultClicked( wxCommandEvent &event )
{
    m_AccelKeys.Empty();
    guAccelGetDefaultKeys( m_AccelKeys );

    int Index;
    int Count = m_AccelKeys.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_AccelListCtrl->SetItem( Index, 1, guAccelGetKeyCodeString( m_AccelKeys[ Index ] ) );
    }
}

// -------------------------------------------------------------------------------- //
