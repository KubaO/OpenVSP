//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSCREENQT_P_INCLUDED_)
#define VSPSCREENQT_P_INCLUDED_

#include "VspScreenQt.h"
#include <QScopedValueRollback>
#include <QDebug>

class Vehicle;
class ScreenMgr;
class QWidget;
class QComboBox;
class VspScreenQt::Private {
    Q_DISABLE_COPY( Private )
    VSP_DECLARE_PUBLIC( VspScreenQt )
    bool blockSignalsInNextUpdate;
    bool blockSignalsInUpdates;
    bool inUpdate;
    bool enableUpdateFlags;
    QScopedValueRollback<bool> * updateFlagRollback;
public:
    Private( VspScreenQt * q );
    virtual ~Private();
    virtual QWidget * widget() = 0;
    Private * self() { return this; }
    Vehicle* veh();
    virtual bool Update() = 0;
    ScreenMgr * GetScreenMgr();
    VspScreen * GetScreen( int id );
    void ConnectUpdateFlag();
    void SetUpdateFlag();
    void CommitUpdateFlag();
    void BlockSignalsInNextUpdate();
    void BlockSignalsInUpdates();
    void EnableUpdateFlags();
    enum { KeepIndex = -2 };
    enum { StartWithUserSets = 1 };
    void LoadSetChoice( QComboBox *, int index, int options = 0 );
protected:
    VspScreenQt * const q_ptr;
};

#endif // VSPSCREENQT_P_INCLUDED_
