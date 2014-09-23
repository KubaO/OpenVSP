//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PickSetScreen.h"
#include "ScreenMgr.h"
#include "ui_PickSetScreen.h"
#include "VspScreenQt_p.h"

class PickSetScreen::Private : public QDialog, public VspScreenQt::Private {
    Q_OBJECT
    VSP_DECLARE_PUBLIC( PickSetScreen )
    Ui::PickSetScreen Ui;

    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE { return true; }
    Private( PickSetScreen * q );
};
VSP_DEFINE_PRIVATE( PickSetScreen )

PickSetScreen::Private::Private( PickSetScreen * q ) :
    VspScreenQt::Private( q )
{
    Ui.setupUi( this );
    connect( Ui.buttonBox, SIGNAL( accepted() ), SLOT( accept() ) );
    connect( Ui.buttonBox, SIGNAL( rejected() ), SLOT( reject() ) );
}

PickSetScreen::PickSetScreen( ScreenMgr * mgr ) :
    VspScreenQt( *new PickSetScreen::Private( this ), mgr )
{
}

int PickSetScreen::PickSet( const std::string & title )
{
    V_D( PickSetScreen );
    d->Ui.screenHeader->setText( title.c_str() );
    d->LoadSetChoice( d->Ui.setChoice, VspScreenQt::Private::KeepIndex );
    if ( d->exec() == QDialog::Accepted )
    {
        return d->Ui.setChoice->currentIndex();
    }
    return -1;
}

PickSetScreen::~PickSetScreen()
{}

#include "PickSetScreen.moc"
