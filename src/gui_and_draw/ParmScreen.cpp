//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

/* A lot of this code was commented out. The Qt port approximates the inferred functionality
 * of the original code. It has to be reviewed what are this screen's use cases, and the code
 * has to be fixed accordingly.
 */
#include "ParmScreen.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "StringUtil.h"
#include "ui_ParmScreen.h"
#include "VspScreenQt_p.h"

#include <QDialog>

using std::string;

class ParmScreen::Private : public QDialog, public VspScreenQt::Private
{
    Q_OBJECT
    Q_PRIVATE_SLOT( self(), void SetUpdateFlag() )
    VSP_DECLARE_PUBLIC( ParmScreen )

    Ui::ParmScreen ui;
    string currParm;

    Private( ParmScreen * q );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE;
    void Update( Parm * );
    void SelectToLink( int sel );
    void SelectFromLink( int sel );

    Q_SLOT void disabled_on_linkToBrowser_currentRowChanged( int row )
    {
        SelectToLink( row );
    }
    Q_SLOT void disabled_on_linkFromBrowser_currentRowChanged( int row )
    {
        SelectFromLink( row );
    }
    Q_SLOT void disabled_on_setParmAButton_clicked()
    {
        LinkMgr.SetCurrLinkIndex( -1 );
        LinkMgr.SetParm( true, currParm );
    }
    Q_SLOT void disabled_on_setParmBButton_clicked()
    {
        LinkMgr.SetCurrLinkIndex( -1 );
        LinkMgr.SetParm( false, currParm );
    }
};
VSP_DEFINE_PRIVATE( ParmScreen )

ParmScreen::Private::Private( ParmScreen * q ) :
    VspScreenQt::Private( q )
{
    ui.setupUi( this );
    ui.nameBox->setObjectName( "sectionHeader_0 ");
    move( 780, 30 );
    BlockSignalsInUpdates();
    ConnectUpdateFlag();
}

ParmScreen::ParmScreen( ScreenMgr* mgr ) :
    VspScreenQt( *new ParmScreen::Private( this ), mgr )
{
}

bool ParmScreen::Private::Update()
{
    Parm * const active = ParmMgr.GetActiveParm();
    if ( ! active )
    {
        q_func()->Hide();
        return false;
    }

    currParm = active->GetID();
    Update( active );
    return true;
}

void ParmScreen::Private::Update( Parm * p )
{
    string pId = p->GetID();
    ui.nameBox->setText( p->GetName().c_str() );
    ui.parmDescription->setText( p->GetDescript().c_str() );

    ui.linkedFromBrowser->clear();
    ui.linkedToBrowser->clear();
    const int N = LinkMgr.GetNumLinks();
    for ( int i = 0; i < N; ++ i )
    {
        Link * const link = LinkMgr.GetLink( i );
        if ( link->GetParmA() == pId )
            ui.linkedToBrowser->addItem( link->GetParmB().c_str() );
        if ( link->GetParmB() == pId )
            ui.linkedFromBrowser->addItem( link->GetParmA().c_str() );
    }
}

void ParmScreen::Update( const std::string & parm )
{
    Parm * const p = ParmMgr.FindParm( parm );
    if ( p ) d_func()->Update( p );
}

void ParmScreen::Show( const std::string & parm )
{
    Update( parm );
    VspScreenQt::Show();
}

void ParmScreen::Private::SelectToLink( int sel )
{
    int const N = LinkMgr.GetNumLinks();
    int count = 0;
    for ( int i = 0; i < N; ++ i )
    {
        if ( LinkMgr.GetLink(i)->GetParmA() == currParm )
        {
            count++;
            if ( count == sel )
            {
                LinkMgr.SetCurrLinkIndex( i );
                /// \todo
                /// Renew is used in place of old parmLinkMgrPtr->RebuildAllLink();
                /// Is this correct? - KO
                LinkMgr.Renew();
            }
        }
    }
}

void ParmScreen::Private::SelectFromLink( int sel )
{
    int const N = LinkMgr.GetNumLinks();
    int count = 0;
    for ( int i = 0; i < N; ++ i )
    {
        if ( LinkMgr.GetLink( i )->GetParmB() == currParm )
        {
            count++;
            if ( count == sel )
            {
                LinkMgr.SetCurrLinkIndex( i );
                /// \todo
                /// Renew is used in place of old parmLinkMgrPtr->RebuildAllLink();
                /// Is this correct? - KO
                LinkMgr.Renew();
            }
        }
    }
}

ParmScreen::~ParmScreen() {}

#include "ParmScreen.moc"
