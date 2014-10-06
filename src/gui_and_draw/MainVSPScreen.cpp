//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MainVSPScreen.h"

#include "ScreenMgr.h"
#include "VehicleMgr.h"
#include "CfdMeshMgr.h"
#include "AdvLinkMgr.h"
#include "MainGLWindow.h"
#include "SelectFileScreen.h"
#include "GraphicEngine.h"
#include "APIDefines.h"
#include "Exit.h"
#include "main.h"
#include "Display.h"
#include "Common.h"

#include "VspScreenQt_p.h"

#include <QLabel>
#include <QWindow>
#include <QScreen>
#include <QStatusBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>

using namespace vsp;
using VSPGUI::VspGlWindow;


class MainVSPScreen::Private : public QMainWindow, public VspScreenQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( MainVSPScreen )

    QLabel * fileLabel;
    VSPGUI::VspGlWindow * glWindow;
    SelectFileScreen selectFileScreen;

    Private( MainVSPScreen * q , ScreenMgr * mgr );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE;
    void closeEvent( QCloseEvent * ) Q_DECL_OVERRIDE;

    std::string CheckAddVSP3Ext( string fname );
};
VSP_DEFINE_PRIVATE( MainVSPScreen )

MainVSPScreen::Private::Private( MainVSPScreen * q, ScreenMgr * mgr ) :
    VspScreenQt::Private( q ),
    fileLabel( new QLabel ),
    glWindow( new VspGlWindow( mgr, DrawObj::VSP_MAIN_SCREEN ) )
{
#if 0
    QSize const scr = window()->windowHandle()->screen()->size();
    // Figure out which is smaller, half the screen width or the height
    int side = ( 0.5 * scr.width() < 0.9 * scr.height() ) ? 0.9 * scr.height() : 0.5 * scr.width();
    move( 10, 30 );
    resize( side, side );
#endif
    setCentralWidget( glWindow );

#if 0
    AddMenuCallBack( m_MainUI->NewMenu );
    AddMenuCallBack( m_MainUI->OpenMenu );
    AddMenuCallBack( m_MainUI->SaveMenu );
    AddMenuCallBack( m_MainUI->SaveAsMenu );
    AddMenuCallBack( m_MainUI->SaveSetMenu );
    AddMenuCallBack( m_MainUI->InsertFileMenu );
    AddMenuCallBack( m_MainUI->ImportMenu );
    AddMenuCallBack( m_MainUI->ExportMenu );
    AddMenuCallBack( m_MainUI->TempDirMenu );
    AddMenuCallBack( m_MainUI->VersionMenu );
    AddMenuCallBack( m_MainUI->RunScriptMenu  );
    AddMenuCallBack( m_MainUI->ExitMenu );

    AddMenuCallBack( m_MainUI->GeomModifyMenu );
    AddMenuCallBack( m_MainUI->TypeEditorMenu  );
    AddMenuCallBack( m_MainUI->UserParmMenu  );
    AddMenuCallBack( m_MainUI->SetEditorMenu  );
    AddMenuCallBack( m_MainUI->StructureMenu );
    AddMenuCallBack( m_MainUI->CompGeomMenu );
    AddMenuCallBack( m_MainUI->DegenGeomMenu );
    AddMenuCallBack( m_MainUI->CFDMeshGeomMenu );
    AddMenuCallBack( m_MainUI->ParmLinkMenu );
    AddMenuCallBack( m_MainUI->ParmDebugMenu );
    AddMenuCallBack( m_MainUI->DesignVarMenu );
    AddMenuCallBack( m_MainUI->MassPropMenu );
    AddMenuCallBack( m_MainUI->AwaveMenu );
    AddMenuCallBack( m_MainUI->PSliceMenu );
    AddMenuCallBack( m_MainUI->AdvParmLinkMenu );

    AddMenuCallBack( m_MainUI->OneScreenMenu );
    AddMenuCallBack( m_MainUI->FourScreenMenu );
    AddMenuCallBack( m_MainUI->TwoHorzScreenMenu );
    AddMenuCallBack( m_MainUI->TwoVertScreenMenu );
    AddMenuCallBack( m_MainUI->BackgroundMenu );

    AddMenuCallBack( m_MainUI->LightingMenu );
    AddMenuCallBack( m_MainUI->ClippingMenu );
    AddMenuCallBack( m_MainUI->LabelsMenu );
    AddMenuCallBack( m_MainUI->ScreenShotMenu );

    AddMenuCallBack( m_MainUI->TopViewMenu );
    AddMenuCallBack( m_MainUI->FrontViewMenu );
    AddMenuCallBack( m_MainUI->LeftSideViewMenu );
    AddMenuCallBack( m_MainUI->LeftIsoViewMenu );
    AddMenuCallBack( m_MainUI->BottomViewMenu );
    AddMenuCallBack( m_MainUI->BackViewMenu );
    AddMenuCallBack( m_MainUI->RightSideViewMenu );
    AddMenuCallBack( m_MainUI->RightIsoViewMenu );
    AddMenuCallBack( m_MainUI->CenterViewMenu );
    AddMenuCallBack( m_MainUI->AdjustViewMenu );
    AddMenuCallBack( m_MainUI->AntialiasMenu );
    AddMenuCallBack( m_MainUI->TextureMenu );

    AddMenuCallBack( m_MainUI->RevertMenu );

    AddMenuCallBack( m_MainUI->ShowAbout );
    AddMenuCallBack( m_MainUI->ShowHelpWebsite );

    AddMenuCallBack( m_MainUI->ReturnToAPI );

    m_MainUI->AdvParmLinkMenu->hide();
    m_MainUI->UserParmMenu->hide();
    m_MainUI->ParmDebugMenu->hide();
#endif

    setWindowTitle( VSPVERSION2 );
    statusBar()->addPermanentWidget( fileLabel, 1 );
    statusBar()->addPermanentWidget( new QLabel( VSPVERSION3 ) );
}

MainVSPScreen::MainVSPScreen( ScreenMgr* mgr ) :
    VspScreenQt( *new Private( this, mgr ), mgr )
{
    SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
    HideReturnToAPI();
}

VSPGUI::VspGlWindow * MainVSPScreen::GetGLWindow()
{
    return d_func()->glWindow;
}

void MainVSPScreen::ShowReturnToAPI()
{
    /// \todo
#if 0
    m_MainUI->ReturnToAPI->show();
#endif
}

void MainVSPScreen::HideReturnToAPI()
{
    /// \todo
#if 0
    m_MainUI->ReturnToAPI->hide();
#endif
}

bool MainVSPScreen::Private::Update()
{
    glWindow->update();
    return true;
}

#if 0
void MainVSPScreen::MenuCallBack( Fl_Widget *w )
{
    Fl_Menu_* mw = static_cast< Fl_Menu_* >( w );
    const Fl_Menu_Item* m = mw->mvalue();

    if ( m == m_MainUI->OneScreenMenu )
    {
        m_GlWin->setWindowLayout( 1, 1 );
        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->FourScreenMenu )
    {
        m_GlWin->setWindowLayout( 2, 2 );
        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->TwoHorzScreenMenu )
    {
        m_GlWin->setWindowLayout( 1, 2 );
        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->TwoVertScreenMenu )
    {
        m_GlWin->setWindowLayout( 2, 1 );
        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->ScreenShotMenu )
    {
        std::string fileName = m_selectFileScreen->FileSave( "Save Screen?", "*.jpg" );

        if( !fileName.empty() )
        {
            std::string::size_type extIndex = fileName.find_last_of( '.' );

            if( extIndex == std::string::npos )
            {
                fileName += ".jpg";
            }
            else
            {
                std::string ext = fileName.substr( extIndex, fileName.size() - extIndex );
                std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
                if( ext != ".jpg" )
                {
                    fileName += ".jpg";
                }
            }
            m_GlWin->getGraphicEngine()->dumpScreenJPEG( fileName );
        }
    }
    else if ( m == m_MainUI->TopViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_TOP );
    }
    else if ( m == m_MainUI->FrontViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_FRONT );
    }
    else if ( m == m_MainUI->LeftSideViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT );
    }
    else if ( m == m_MainUI->LeftIsoViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
    }
    else if ( m == m_MainUI->BottomViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_BOTTOM );
    }
    else if ( m == m_MainUI->BackViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_REAR );
    }
    else if ( m == m_MainUI->RightSideViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT );
    }
    else if ( m == m_MainUI->RightIsoViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_RIGHT_ISO );
    }
    else if ( m == m_MainUI->CenterViewMenu )
    {
        m_GlWin->setView( VSPGraphic::Common::VSP_CAM_CENTER );
    }
    else if ( m == m_MainUI->AdjustViewMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_VIEW_SCREEN );
    }
    else if ( m == m_MainUI->BackgroundMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_BACKGROUND_SCREEN );
    }
    else if ( m == m_MainUI->ExportMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_EXPORT_SCREEN );
    }
    else if ( m == m_MainUI->ImportMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_IMPORT_SCREEN );
    }
    else if ( m == m_MainUI->GeomModifyMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->RevertMenu )
    {
        VehicleMgr.GetVehicle()->UnDo();
    }
    else if ( m == m_MainUI->SetEditorMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_SET_EDITOR_SCREEN );
    }
    else if ( m == m_MainUI->TypeEditorMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_TYPE_EDITOR_SCREEN );
    }
    else if ( m == m_MainUI->UserParmMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_USER_PARM_SCREEN );
    }
    else if ( m == m_MainUI->TextureMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MANAGE_TEXTURE_SCREEN );
    }
    else if ( m == m_MainUI->ParmLinkMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PARM_LINK_SCREEN );
    }
    else if ( m == m_MainUI->AdvParmLinkMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_ADV_LINK_SCREEN );
    }
    else if ( m == m_MainUI->LightingMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_LIGHTING_SCREEN );
    }
    else if ( m == m_MainUI->ClippingMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CLIPPING_SCREEN );
    }
    else if ( m == m_MainUI->LabelsMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_LABEL_SCREEN );
    }
    else if ( m == m_MainUI->ParmDebugMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PARM_DEBUG_SCREEN );
    }
    else if ( m == m_MainUI->DesignVarMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_DESIGN_VAR_SCREEN );
    }
    else if ( m == m_MainUI->CompGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_COMP_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->DegenGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_DEGEN_GEOM_SCREEN );
    }
    else if ( m == m_MainUI->CFDMeshGeomMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CFD_MESH_SCREEN );
    }
    else if ( m == m_MainUI->MassPropMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MASS_PROP_SCREEN );
    }
    else if ( m == m_MainUI->PSliceMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_PSLICE_SCREEN );
    }
    else if ( m == m_MainUI->AwaveMenu )
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_AWAVE_SCREEN );
    }
    else if ( m == m_MainUI->StructureMenu )
    {
        m_ScreenMgr->Alert( "Wing structure support not implemented at this time." );
//      m_ScreenMgr->ShowScreen( ScreenMgr::VSP_FEA_MESH_SCREEN );
    }
    else if ( m == m_MainUI->NewMenu )
    {
        VehicleMgr.GetVehicle()->Renew();

        SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
        m_GlWin->getGraphicEngine()->getDisplay()->setCOR( 0.0, 0.0, 0.0 );

        m_ScreenMgr->SetUpdateFlag( true );
    }
    else if ( m == m_MainUI->OpenMenu )
    {
        string openfile = m_ScreenMgr->GetSelectFileScreen()->FileOpen( "Open VSP File", "*.vsp3" );
        if ( openfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->Renew();
            VehicleMgr.GetVehicle()->SetVSP3FileName( openfile );
            VehicleMgr.GetVehicle()->ReadXMLFile( openfile );

            CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

            SetFileLabel( openfile );

            BndBox bbox = VehicleMgr.GetVehicle()->GetBndBox();
            vec3d p = bbox.GetCenter();
            m_GlWin->getGraphicEngine()->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveMenu )
    {
        string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

        if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
        {
            savefile = m_ScreenMgr->GetSelectFileScreen()->FileSave( "Save VSP File", "*.vsp3" );
        }

        if ( savefile.compare( "" ) != 0 )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

            CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

            SetFileLabel( savefile );

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveAsMenu )
    {
        string savefile = m_ScreenMgr->GetSelectFileScreen()->FileSave( "Save VSP File As", "*.vsp3" );
        if ( savefile.compare( "" ) != 0 )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

            CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

            SetFileLabel( savefile );

            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->SaveSetMenu )
    {
        int set = m_ScreenMgr->GetPickSetScreen()->PickSet( "Pick Save Set" );

        if( set >= 0 )
        {
            string savefile = m_ScreenMgr->GetSelectFileScreen()->FileSave( "Save VSP Set File As", "*.vsp3" );
            if ( savefile.compare( "" ) != 0 )
            {
                savefile = CheckAddVSP3Ext( savefile );
                VehicleMgr.GetVehicle()->WriteXMLFile( savefile, set );
            }
        }
    }
    else if ( m == m_MainUI->InsertFileMenu )
    {
        string openfile = m_ScreenMgr->GetSelectFileScreen()->FileOpen( "Insert VSP File", "*.vsp3" );
        if ( openfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->ReadXMLFile( openfile );
            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->RunScriptMenu )
    {
        string scriptfile = m_ScreenMgr->GetSelectFileScreen()->FileOpen( "Script File To Run", "*.as" );
        if ( scriptfile.compare( "" ) != 0 )
        {
            VehicleMgr.GetVehicle()->RunScript( scriptfile );
            m_ScreenMgr->SetUpdateFlag( true );
        }
    }
    else if ( m == m_MainUI->ExitMenu )
    {
        ExitVSP();
    }
    else if ( m == m_MainUI->ReturnToAPI )
    {
        m_ScreenMgr->SetRunGui( false );
    }
}
#endif

string MainVSPScreen::Private::CheckAddVSP3Ext( string fname )
{
    string ext = ".vsp3";
    string fext = fname.substr( fname.length() - ext.length(), ext.length() );

    // STL string tolower
    transform( fext.begin(), fext.end(), fext.begin(), ::tolower );

    if ( fext.compare( ext ) != 0 )
    {
        fname.append( ".vsp3" );
    }

    return fname;
}

void MainVSPScreen::SetFileLabel( const string & fname )
{
    string label = "File Name: ";
    label.append( fname );
    d_func()->fileLabel->setText( label.c_str() );
}

void MainVSPScreen::Private::closeEvent( QCloseEvent * ev )
{
    QMessageBox box( QMessageBox::Question, "Exit VSP?",
                     "OpenVSP is exiting. Save or discard your changes.",
                     QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save, this );
    box.exec();
    auto button = box.standardButton( box.clickedButton() );
    if ( button == QMessageBox::Cancel ) {
        ev->ignore();
        return;
    }
    else if ( button == QMessageBox::Save ) {
        string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

        if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
        {
            savefile = GetScreenMgr()->GetSelectFileScreen()->FileSave( "Save VSP File", "*.vsp3" );
        }

        if ( !savefile.empty() )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );
            vsp_exit();
        }
        else
        {
            ev->ignore();
        }
    }
    else if ( button == QMessageBox::Discard ) {
        vsp_exit();
    }
}

MainVSPScreen::~MainVSPScreen() {}

#include "MainVSPScreen.moc"
