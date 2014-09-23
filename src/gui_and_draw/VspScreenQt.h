//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
/// \class VspScreenQt
/// The base of Qt-based screens.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSCREENQT__INCLUDED_)
#define VSPSCREENQT__INCLUDED_

#include "VSPScreen.h"
#include "VspObj.h"
#include <QScopedPointer>

class VspScreenQt : public VspScreen
{
    Q_DISABLE_COPY( VspScreenQt )
    VSP_DECLARE_PRIVATE( VspScreenQt )
    bool Update() Q_DECL_OVERRIDE;
public:
    ~VspScreenQt();

    void Show() Q_DECL_OVERRIDE;
    void Hide() Q_DECL_OVERRIDE;
    bool IsShown() Q_DECL_OVERRIDE;
    void SetNonModal() Q_DECL_OVERRIDE;
    void GuiDeviceCallBack( GuiDevice* device ) Q_DECL_OVERRIDE;

    virtual int x() Q_DECL_OVERRIDE;
    virtual int y() Q_DECL_OVERRIDE;
    virtual int w() Q_DECL_OVERRIDE;
    virtual int h() Q_DECL_OVERRIDE;
    virtual void position( int X, int Y ) Q_DECL_OVERRIDE;

protected:
    QScopedPointer< Private > const d_ptr;
    VspScreenQt( Private & dd, ScreenMgr * mgr );
};

#endif // VSPSCREENQT__INCLUDED_
