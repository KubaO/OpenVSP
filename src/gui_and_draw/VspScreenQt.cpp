//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "VspScreenQt.h"
#include "VspScreenQt_p.h"
#include "UiSignalBlocker.h"
#include "ScreenMgr.h"
#include "APIDefines.h"
#include <QFile>
#include <QComboBox>
#include <QScopedValueRollback>
#include <QMetaProperty>
#include <QAbstractButton>

VSP_DEFINE_PRIVATE( VspScreenQt )

VspScreenQt::VspScreenQt( VspScreenQt::Private & dd, ScreenMgr * mgr ) :
    VspScreen( mgr ),
    d_ptr( &dd )
{
}

void VspScreenQt::Show()
{
    if (Update()) d_func()->widget()->show();
}

void VspScreenQt::Hide()
{
    d_func()->widget()->hide();
}

bool VspScreenQt::IsShown()
{
    return d_func()->widget()->isVisible();
}

void VspScreenQt::SetNonModal()
{
    d_func()->widget()->setWindowModality( Qt::NonModal ) ;
}

bool VspScreenQt::Update()
{
    V_D( VspScreenQt );
    if ( d->inUpdate ) return false;
    typedef QScopedValueRollback<bool> BoolRollback;
    BoolRollback updateFlagRollback( GetScreenMgr()->GetUpdateFlag() );
    QScopedValueRollback<BoolRollback*> roll(d->updateFlagRollback);
    d->updateFlagRollback = &updateFlagRollback;

    UiSignalBlocker blocker;
    if ( d->blockSignalsInNextUpdate || d->blockSignalsInUpdates ) {
        blocker.block( d->widget() );
    }
    d->inUpdate = true;
    bool rc = d->Update();
    d->inUpdate = false;
    d->blockSignalsInNextUpdate = false;
    return rc;
}

void VspScreenQt::GuiDeviceCallBack( GuiDevice* device )
{
}

int VspScreenQt::x()
{
    return d_func()->widget()->x();
}

int VspScreenQt::y()
{
    return d_func()->widget()->y();
}

int VspScreenQt::w()
{
    return d_func()->widget()->width();
}

int VspScreenQt::h()
{
    return d_func()->widget()->height();
}

void VspScreenQt::position( int X, int Y )
{
    d_func()->widget()->move( X, Y );
}

VspScreenQt::~VspScreenQt()
{
}

VspScreenQt::Private::Private( VspScreenQt * q ) :
    blockSignalsInNextUpdate( false ),
    blockSignalsInUpdates( false ),
    inUpdate( false ),
    updateFlagRollback( 0 ),
    q_ptr( q )
{
}

Vehicle* VspScreenQt::Private::veh() {
    return GetScreenMgr()->GetVehiclePtr();
}

ScreenMgr* VspScreenQt::Private::GetScreenMgr() {
    return q_func()->GetScreenMgr();
}

VspScreen* VspScreenQt::Private::GetScreen( int id ) {
    return GetScreenMgr()->GetScreen( id );
}

/// Connect the SetUpdateFlag to every widget's user property change notification.
void VspScreenQt::Private::ConnectUpdateFlag()
{
    const QMetaObject * const mo = widget()->metaObject();
    QMetaMethod flagMethod = mo->method( mo->indexOfSlot("SetUpdateFlag()") );
    foreach ( QWidget * w, widget()->findChildren<QWidget*>() ) {
        // Skip internal Qt subcontrols.
        if ( w->objectName().startsWith( "qt_" ) )
            continue;
        // Non-checkable buttons interest us only when they get clicked.
        QAbstractButton * button = qobject_cast<QAbstractButton*>( w );
        if ( button && !button->isCheckable() ) {
            widget()->connect( button, SIGNAL( clicked() ), SLOT( SetUpdateFlag() ) );
            continue;
        }
        // Update when the user property notifies of a change.
        const QMetaObject * const mo = w->metaObject();
        QMetaProperty mp = mo->userProperty();
        if ( mp.isValid() && mp.hasNotifySignal() )
            QObject::connect( w, mp.notifySignal(), widget(), flagMethod );
    }
    EnableUpdateFlags();
}

/// Sets the global update flag on the screen manager. Does nothing by default
/// until EnableUpdateFlags() is first called.
void VspScreenQt::Private::SetUpdateFlag() {
    if ( enableUpdateFlags ) GetScreenMgr()->SetUpdateFlag( true );
}

/// Commits changes to UpdateFlag. Has no effect outside of Update.
void VspScreenQt::Private::CommitUpdateFlag()
{
    if ( updateFlagRollback ) updateFlagRollback->commit();
}

void VspScreenQt::Private::BlockSignalsInNextUpdate()
{
    blockSignalsInNextUpdate = true;
}

void VspScreenQt::Private::BlockSignalsInUpdates()
{
    blockSignalsInUpdates = true;
}

void VspScreenQt::Private::EnableUpdateFlags()
{
    enableUpdateFlags = true;
}

void VspScreenQt::Private::LoadSetChoice( QComboBox * widget, int index, int options )
{
    if ( index == KeepIndex ) {
        index = qMax(0, widget->currentIndex());
    }
    widget->clear();
    int firstSet = options & StartWithUserSets ? vsp::SET_FIRST_USER : 0;
    std::vector< std::string > const setNames = veh()->GetSetNameVec();
    for ( size_t i = firstSet; i < setNames.size(); ++i )
    {
        widget->addItem( setNames[i].c_str() );
    }
    widget->setCurrentIndex( index );
}

VspScreenQt::Private::~Private()
{
}
