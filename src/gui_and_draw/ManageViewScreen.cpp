//
// License terms are missing.
//
//////////////////////////////////////////////////////////////////////

#include "ManageViewScreen.h"
#include "ScreenMgr.h"
#include "MainVSPScreen.h"
#include "MainGLWindow.h"
#include "VspScreenQt_p.h"
#include "ui_ManageViewScreen.h"

class ManageViewScreen::Private : public QDialog, public VspScreenQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( ManageViewScreen )
    Q_PRIVATE_SLOT( self(), void SetUpdateFlag() )
    Ui::ManageViewScreen Ui;

    Private( ManageViewScreen * q );
    VSPGUI::VspGlWindow * glwin();
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE { return true; }

    Q_SLOT void on_xPosMinus1_clicked()
    {
        if (glwin()) glwin()->pan( -1, 0, true );
    }
    Q_SLOT void on_xPosMinus2_clicked()
    {
        if (glwin()) glwin()->pan( -1, 0, false );
    }
    Q_SLOT void on_xPosPlus1_clicked()
    {
        if (glwin()) glwin()->pan( 1, 0, true );
    }
    Q_SLOT void on_xPosPlus2_clicked()
    {
        if (glwin()) glwin()->pan( 1, 0, false );
    }
    Q_SLOT void on_yPosMinus1_clicked()
    {
        if (glwin()) glwin()->pan( 0, -1, true );
    }
    Q_SLOT void on_yPosMinus2_clicked()
    {
        if (glwin()) glwin()->pan( 0, -1, false );
    }
    Q_SLOT void on_yPosPlus1_clicked()
    {
        if (glwin()) glwin()->pan( 0, 1, true );
    }
    Q_SLOT void on_yPosPlus2_clicked()
    {
        if (glwin()) glwin()->pan( 0, 1, false );
    }
    Q_SLOT void on_zoomMinus1_clicked()
    {
        if (glwin()) glwin()->zoom( 1, true );
    }
    Q_SLOT void on_zoomMinus2_clicked()
    {
        if (glwin()) glwin()->zoom( 1, false );
    }
    Q_SLOT void on_zoomPlus1_clicked()
    {
        if (glwin()) glwin()->zoom( -1, true );
    }
    Q_SLOT void on_zoomPlus2_clicked()
    {
        if (glwin()) glwin()->zoom( -1, false );
    }
};
VSP_DEFINE_PRIVATE( ManageViewScreen )

ManageViewScreen::Private::Private( ManageViewScreen * q ) :
    VspScreenQt::Private( q )
{
    Ui.setupUi( this );
    ConnectUpdateFlag();
}

VSPGUI::VspGlWindow * ManageViewScreen::Private::glwin()
{
    auto main =
            dynamic_cast<MainVSPScreen*>( GetScreen( ScreenMgr::VSP_MAIN_SCREEN ) );
    if ( !main ) return 0;
    return main->GetGLWindow();
}

ManageViewScreen::ManageViewScreen( ScreenMgr * mgr ) :
    VspScreenQt( *new ManageViewScreen::Private( this ), mgr )
{
    d_func()->move( 775, 50 );
}

ManageViewScreen::~ManageViewScreen()
{
}

#include "ManageViewScreen.moc"
