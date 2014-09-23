//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "UiSignalBlocker.h"
#include <QWidget>
#include <QPointer>
#include <QList>

class UiSignalBlocker::Private {
public:
    QList< QPointer<QWidget> > blocked;
};
VSP_DEFINE_PRIVATE( UiSignalBlocker )

UiSignalBlocker::UiSignalBlocker( QWidget * b ) :
    d_ptr( new UiSignalBlocker::Private )
{
    if ( b ) block( b );
}

void UiSignalBlocker::block( QWidget * b )
{
    V_D( UiSignalBlocker );
    unblock();
    if ( ! b ) return;
    if ( ! b->signalsBlocked() ) {
        d->blocked << b;
        b->blockSignals( true );
    }
    foreach ( QWidget * w, b->findChildren< QWidget* >() )
    {
        if ( w->objectName().startsWith( "qt_" ) || w->signalsBlocked() ) continue;
        d->blocked << w;
        w->blockSignals( true );
    }

}

void UiSignalBlocker::unblock()
{
    V_D( UiSignalBlocker );
    foreach ( QPointer<QWidget> w, d->blocked )
    {
        // A QPointer is a weak smart pointer. If the object was destroyed, the
        // pointer value will be zero.
        if ( w ) w->blockSignals( false );
    }
    d->blocked.clear();
}

UiSignalBlocker::~UiSignalBlocker()
{
    unblock();
}
