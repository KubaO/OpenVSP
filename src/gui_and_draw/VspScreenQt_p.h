//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSCREENQT_P_INCLUDED_)
#define VSPSCREENQT_P_INCLUDED_

#include <QScopedValueRollback>
#include <QDebug>

class Geom;
class Vehicle;
class ScreenMgr;
class QWidget;
class QComboBox;
class VspScreenQtPrivate {
    Q_DISABLE_COPY( VspScreenQtPrivate )
    Q_DECLARE_PUBLIC( VspScreenQt )
    bool blockSignalsInNextUpdate;
    bool blockSignalsInUpdates;
    bool inUpdate;
    bool enableUpdateFlags;
    QScopedValueRollback<bool> * updateFlagRollback;
public:
    VspScreenQtPrivate( VspScreenQt * q );
    virtual ~VspScreenQtPrivate();
    virtual QWidget * widget() = 0;
    VspScreenQtPrivate * self() { return this; }
    virtual void GuiDeviceCallBack( GuiDevice * );
    Vehicle* veh();
    virtual bool Update() = 0;
    virtual Geom* geom();
    ScreenMgr * GetScreenMgr();
    VspScreen * GetScreen( int id );
    void ConnectUpdateFlag();
    void ConnectUpdateFlag( QWidget * widget );
    void ConnectUpdateFlag( QWidget * widget, QMetaMethod & flagMethod );
    void SetUpdateFlag();
    void CommitUpdateFlag();
    void BlockSignalsInNextUpdate();
    void BlockSignalsInUpdates();
    void EnableUpdateFlags();
    /// Options
    enum { KeepIndex = -2 };
    /// Indices
    enum { StartWithUserSets = 1 };
    void LoadSetChoice( QComboBox *, int index, int options = 0 );
protected:
    VspScreenQt * const q_ptr;
};

#endif // VSPSCREENQT_P_INCLUDED_
