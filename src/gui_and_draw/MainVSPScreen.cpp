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
#include <QMenuBar>
#include <QWindow>
#include <QScreen>
#include <QStatusBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QGuiApplication>

using namespace vsp;
using VSPGUI::VspGlWindow;


class MainVSPScreen::Private : public QMainWindow, public VspScreenQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( MainVSPScreen )

    QLabel fileLabel;
    QMenuBar menuBar;
    VSPGUI::VspGlWindow * glWindow;
    SelectFileScreen selectFileScreen;

    QAction * advParmLink, * userParm, * parmDebug, * apiReturn;

    Private( MainVSPScreen * q , ScreenMgr * mgr );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE;
    void closeEvent( QCloseEvent * ) Q_DECL_OVERRIDE;

    std::string CheckAddVSP3Ext( string fname );

    // File
    Q_SLOT void onFileNew();
    Q_SLOT void onFileOpen();
    Q_SLOT void onFileSave();
    Q_SLOT void onFileSaveAs();
    Q_SLOT void onFileSaveSet();
    Q_SLOT void onFileInsert();
    Q_SLOT void onFileRunScript();
    Q_SLOT void onFileExport()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_EXPORT_SCREEN );
    }
    Q_SLOT void onFileImport()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_IMPORT_SCREEN );
    }
    Q_SLOT void onFileQuit()
    {
        close();
    }

    // Edit
    Q_SLOT void onEditUndo()
    {
        VehicleMgr.GetVehicle()->UnDo();
    }

    // View
    Q_SLOT void onViewTop()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_TOP );
    }
    Q_SLOT void onViewFront()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_FRONT );
    }
    Q_SLOT void onViewLeftSide()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_LEFT );
    }
    Q_SLOT void onViewLeftIso()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_LEFT_ISO );
    }
    Q_SLOT void onViewBottom()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_BOTTOM );
    }
    Q_SLOT void onViewBack()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_REAR );
    }
    Q_SLOT void onViewRightSide()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_RIGHT );
    }
    Q_SLOT void onViewRightIso()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_RIGHT_ISO );
    }
    Q_SLOT void onViewCenter()
    {
        glWindow->setView( VSPGraphic::Common::VSP_CAM_CENTER );
    }
    Q_SLOT void onViewAdjust()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_VIEW_SCREEN );
    }

    // Model
    Q_SLOT void onModelGeometry()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN );
    }
    Q_SLOT void onModelSetEditor()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_SET_EDITOR_SCREEN );
    }
    Q_SLOT void onModelTypeEditor()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_TYPE_EDITOR_SCREEN );
    }
    Q_SLOT void onModelUserParm()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_USER_PARM_SCREEN );
    }
    Q_SLOT void onModelTexture()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_MANAGE_TEXTURE_SCREEN );
    }
    Q_SLOT void onModelParmLink()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PARM_LINK_SCREEN );
    }
    Q_SLOT void onModelAdvParmLink()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_ADV_LINK_SCREEN );
    }
    Q_SLOT void onModelLighting()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_LIGHTING_SCREEN );
    }
    Q_SLOT void onModelClipping()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_CLIPPING_SCREEN );
    }
    Q_SLOT void onModelLabels()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_LABEL_SCREEN );
    }
    Q_SLOT void onModelParmDebug()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PARM_DEBUG_SCREEN );
    }
    Q_SLOT void onModelDesignVar()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_DESIGN_VAR_SCREEN );
    }

    // Analysis
    Q_SLOT void onAnalysisCompGeom()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_COMP_GEOM_SCREEN );
    }
    Q_SLOT void onAnalysisDegenGeom()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_DEGEN_GEOM_SCREEN );
    }
    Q_SLOT void onAnalysisCFDMesh()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_CFD_MESH_SCREEN );
    }
    Q_SLOT void onAnalysisMassProp()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_MASS_PROP_SCREEN );
    }
    Q_SLOT void onAnalysisPlanarSlice()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PSLICE_SCREEN );
    }
    Q_SLOT void onAnalysisAwaveSlice()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_AWAVE_SCREEN );
    }
    Q_SLOT void onAnalysisWingStructure();

    // Window
    Q_SLOT void onWindowOne()
    {
        glWindow->setWindowLayout( 1, 1 );
        SetUpdateFlag();
    }
    Q_SLOT void onWindowFour()
    {
        glWindow->setWindowLayout( 2, 2 );
        SetUpdateFlag();
    }
    Q_SLOT void onWindowTwoHorz()
    {
        glWindow->setWindowLayout( 1, 2 );
        SetUpdateFlag();
    }
    Q_SLOT void onWindowTwoVert()
    {
        glWindow->setWindowLayout( 2, 1 );
        SetUpdateFlag();
    }
    Q_SLOT void onWindowBackground()
    {
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_BACKGROUND_SCREEN );
    }
    Q_SLOT void onWindowScreenshot();

    // Help


    // API Return
    Q_SLOT void onApiReturn()
    {
        GetScreenMgr()->SetRunGui( false );
    }

};
VSP_DEFINE_PRIVATE( MainVSPScreen )

MainVSPScreen::Private::Private( MainVSPScreen * q, ScreenMgr * mgr ) :
    VspScreenQt::Private( q ),
    glWindow( new VspGlWindow( mgr, DrawObj::VSP_MAIN_SCREEN ) )
{
    QRect const geom = QGuiApplication::primaryScreen()->availableGeometry();
    QSize const scr = geom.size();
    // Figure out which is smaller, half the screen width or the height
    int side = ( 0.5 * scr.width() < 0.9 * scr.height() ) ? 0.9 * scr.height() : 0.5 * scr.width();
    move( geom.topLeft() );
    resize( side, side );
    setCentralWidget( glWindow );
#ifndef Q_OS_MAC
    setMenuBar( &menuBar );
#endif

    auto file = menuBar.addMenu("File");
    file->addAction("New", this, SLOT(onFileNew()));
    file->addAction("Open...", this, SLOT(onFileOpen()));
    file->addAction("Save...", this, SLOT(onFileSave()));
    file->addAction("Save As...", this, SLOT(onFileSaveAs()));
    file->addAction("Save Set...", this, SLOT(onFileSaveSet()));
    file->addAction("Insert...", this, SLOT(onFileInsert()));
    file->addAction("Import...", this, SLOT(onFileImport()));
    file->addAction("Export...", this, SLOT(onFileExport()));
    file->addAction("Set Temp Dir...", this, SLOT(onFileSetTempDir()));
    file->addAction("Run Script...", this, SLOT(onFileRunScript()));
    file->addAction("Quit", this, SLOT(onFileQuit()));

#if 0
    auto edit = menuBar.addMenu("Edit");
#endif
    menuBar.addAction("Undo", this, SLOT(onEditUndo()));

    auto view = menuBar.addMenu("View");
    view->addAction("Top", this, SLOT(onViewTop()));
    view->addAction("Front", this, SLOT(onViewFront()));
    view->addAction("Left Side", this, SLOT(onViewLeftSide()));
    view->addAction("Left Iso", this, SLOT(onViewLeftIso()));
    view->addAction("Bottom", this, SLOT(onViewBottom()));
    view->addAction("Back", this, SLOT(onViewBack()));
    view->addAction("Right Side", this, SLOT(onViewRightSide()));
    view->addAction("Right Iso", this, SLOT(onViewRightIso()));
    view->addAction("Center", this, SLOT(onViewCenter()));
    view->addAction("Adjust...", this, SLOT(onViewAdjust()));
    if ( false )
        view->addAction("Antialias Lines", this, SLOT(onViewAntialias()));

    auto model = menuBar.addMenu("Model");
    model->addAction("Geometry...", this, SLOT(onModelGeometry()));
    model->addAction("Type Editor...", this, SLOT(onModelTypeEditor()));
    model->addAction("Set Editor...", this, SLOT(onModelSetEditor()));
    model->addAction("Link...", this, SLOT(onModelParmLink()));
    model->addAction("Design Variables...", this, SLOT(onModelDesignVar()));
    model->addAction("Labels...", this, SLOT(onModelLabels()));
    model->addAction("Lighting...", this, SLOT(onModelLighting()));
    model->addAction("Clipping...", this, SLOT(onModelClipping()));
    model->addAction("Texture...", this, SLOT(onModelTexture()));
    parmDebug = model->addAction("Debug Parameters...", this, SLOT(onModelParmDebug()));
    advParmLink = model->addAction("Adv Link...", this, SLOT(onModelAdvParmLink()));
    userParm = model->addAction("User Parms...", this, SLOT(onModelUserParm()));

    auto analysis = menuBar.addMenu("Analysis");
    analysis->addAction("Comp Geom...", this, SLOT(onAnalysisCompGeom()));
    analysis->addAction("Awave Slice...", this, SLOT(onAnalysisAwaveSlice()));
    analysis->addAction("Planar Slice...", this, SLOT(onAnalysisPlanarSlice()));
    analysis->addAction("Mass Prop...", this, SLOT(onAnalysisMassProp()));
    analysis->addAction("CFD Mesh...", this, SLOT(onAnalysisCFDMesh()));
    analysis->addAction("Wing Structure...", this, SLOT(onAnalysisWingStructure()));
    analysis->addAction("Degen Geom...", this, SLOT(onAnalysisDegenGeom()));

    auto window = menuBar.addMenu("Window");
    window->addAction("One", this, SLOT(onWindowOne()));
    window->addAction("Two Horizontal", this, SLOT(onWindowTwoHorz()));
    window->addAction("Two Vertical", this, SLOT(onWindowTwoVert()));
    window->addAction("Four", this, SLOT(onWindowFour()));
    window->addAction("Background...", this, SLOT(onWindowBackground()));
    window->addAction("Screenshot...", this, SLOT(onWindowScreenshot()));

    auto help = menuBar.addMenu("Help");
    if ( false ) {
        help->addAction("About...", this, SLOT(onHelpAbout()));
        help->addAction("Contents", this, SLOT(onHelpContents()));
        help->addAction("Check Latest Version...", this, SLOT(onHelpVersion()));
    }

    apiReturn = menuBar.addAction("API Return", this, SLOT(onApiReturn()));

    advParmLink->setEnabled( false );
    userParm->setEnabled( false );
    parmDebug->setEnabled( false );

    setWindowTitle( VSPVERSION2 );
    statusBar()->addPermanentWidget( &fileLabel, 1 );
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
    d_func()->apiReturn->setVisible( true );
}

void MainVSPScreen::HideReturnToAPI()
{
    d_func()->apiReturn->setVisible( false );
}

bool MainVSPScreen::Private::Update()
{
    glWindow->update();
    return true;
}

void MainVSPScreen::Private::onWindowScreenshot()
{
    std::string fileName = selectFileScreen.FileSave( "Save Screen?", "*.jpg" );

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
        glWindow->getGraphicEngine()->dumpScreenJPEG( fileName );
    }
}

void MainVSPScreen::Private::onAnalysisWingStructure()
{
    if ( true )
        GetScreenMgr()->Alert( "Wing structure support not implemented at this time." );
    else
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_FEA_MESH_SCREEN );
}

void MainVSPScreen::Private::onFileNew()
{
    VehicleMgr.GetVehicle()->Renew();

    q_func()->SetFileLabel( VehicleMgr.GetVehicle()->GetVSP3FileName() );
    glWindow->getGraphicEngine()->getDisplay()->setCOR( 0.0, 0.0, 0.0 );

    SetUpdateFlag();
}

void MainVSPScreen::Private::onFileOpen()
{
    string openfile = GetScreenMgr()->GetSelectFileScreen()->FileOpen( "Open VSP File", "*.vsp3" );
    if ( openfile.compare( "" ) != 0 )
    {
        VehicleMgr.GetVehicle()->Renew();
        VehicleMgr.GetVehicle()->SetVSP3FileName( openfile );
        VehicleMgr.GetVehicle()->ReadXMLFile( openfile );

        CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

        q_func()->SetFileLabel( openfile );

        BndBox bbox = VehicleMgr.GetVehicle()->GetBndBox();
        vec3d p = bbox.GetCenter();
        glWindow->getGraphicEngine()->getDisplay()->setCOR( -p.x(), -p.y(), -p.z() );

        SetUpdateFlag();
    }
}

void MainVSPScreen::Private::onFileSave()
{
    string savefile = VehicleMgr.GetVehicle()->GetVSP3FileName();

    if ( savefile.compare( "Unnamed.vsp3" ) == 0 )
    {
        savefile = GetScreenMgr()->GetSelectFileScreen()->FileSave( "Save VSP File", "*.vsp3" );
    }

    if ( savefile.compare( "" ) != 0 )
    {
        savefile = CheckAddVSP3Ext( savefile );
        VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
        VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

        CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

        q_func()->SetFileLabel( savefile );
        SetUpdateFlag();
    }
}

void MainVSPScreen::Private::onFileSaveAs()
{
    string savefile = GetScreenMgr()->GetSelectFileScreen()->FileSave( "Save VSP File As", "*.vsp3" );
    if ( savefile.compare( "" ) != 0 )
    {
        savefile = CheckAddVSP3Ext( savefile );
        VehicleMgr.GetVehicle()->SetVSP3FileName( savefile );
        VehicleMgr.GetVehicle()->WriteXMLFile( savefile, SET_ALL );

        CfdMeshMgr.GetCfdSettingsPtr()->ResetExportFileNames();

        q_func()->SetFileLabel( savefile );
        SetUpdateFlag();
    }
}

void MainVSPScreen::Private::onFileSaveSet()
{
    int set = GetScreenMgr()->GetPickSetScreen()->PickSet( "Pick Save Set" );

    if( set >= 0 )
    {
        string savefile = GetScreenMgr()->GetSelectFileScreen()->FileSave( "Save VSP Set File As", "*.vsp3" );
        if ( savefile.compare( "" ) != 0 )
        {
            savefile = CheckAddVSP3Ext( savefile );
            VehicleMgr.GetVehicle()->WriteXMLFile( savefile, set );
        }
    }
}

void MainVSPScreen::Private::onFileInsert()
{
    string openfile = GetScreenMgr()->GetSelectFileScreen()->FileOpen( "Insert VSP File", "*.vsp3" );
    if ( openfile.compare( "" ) != 0 )
    {
        VehicleMgr.GetVehicle()->ReadXMLFile( openfile );
        SetUpdateFlag();
    }
}

void MainVSPScreen::Private::onFileRunScript()
{
    string scriptfile = GetScreenMgr()->GetSelectFileScreen()->FileOpen( "Script File To Run", "*.as" );
    if ( scriptfile.compare( "" ) != 0 )
    {
        VehicleMgr.GetVehicle()->RunScript( scriptfile );
        SetUpdateFlag();
    }
}

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
    d_func()->fileLabel.setText( label.c_str() );
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
