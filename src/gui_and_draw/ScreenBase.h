//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCREENBASE__INCLUDED_)
#define SCREENBASE__INCLUDED_

#define QPoint QQPoint
#include <FL/Fl.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#undef QPoint

#include <vector>
#include <map>

#include "GuiDevice.h"
#include "GroupLayout.h"
#include "SubGLWindow.h"
#include <QtGlobal>

#include "VspScreenQt.h"
#include "GroupLayout.h"

#include "VSPWindow.h"

class QWidget;
class QScrollArea;
class ScreenMgr;
class Vehicle;
class Geom;

class BasicScreenPrivate;
/// A Screen with a Title
class BasicScreen : public VspScreenQt
{
    VSP_DECLARE_PRIVATE( BasicScreen )
public:
    BasicScreen( ScreenMgr*, int w, int h, const std::string & title );
    ~BasicScreen();
    void SetTitle( const string & title );
protected:
    BasicScreen( BasicScreenPrivate&, ScreenMgr* );
};

class TabScreenPrivate;
class TabScreen : public BasicScreen
{
    VSP_DECLARE_PRIVATE( TabScreen )
public:
    TabScreen( ScreenMgr* mgr, int w, int h, const string & title, int baseymargin = 0 );
    ~TabScreen();

    virtual QWidget* AddTab( const string& title );
    virtual QWidget* AddTab( const string& title, int indx );
    virtual QWidget* GetTab( int index );

    virtual void AddTab( QWidget* grp );
    virtual void AddTab( QWidget* grp, int indx );
    virtual void RemoveTab( QWidget* grp );

    /// Create A Sub Group In Tab - With Border in Pixels
    virtual QWidget* AddSubGroup( QWidget* group, int border  );
    virtual QScrollArea* AddSubScroll( QWidget* group, int border, int lessh = 0  );
protected:
    TabScreen( TabScreenPrivate &, ScreenMgr * );
};


class GeomScreenPrivate;
class GeomScreen : public TabScreen
{
    VSP_DECLARE_PRIVATE( GeomScreen )
public:
    GeomScreen( ScreenMgr* mgr, int w, int h, const string & title );
    ~GeomScreen();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
};

//==== Skin Screen ====//
class SkinScreen : public GeomScreen
{
public:
    SkinScreen( ScreenMgr* mgr, int w, int h, const string & title );
    virtual ~SkinScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( SkinScreen* )data )->CallBack( w );
    }

protected:

    GroupLayout m_SkinLayout;

    IndexSelector m_SkinIndexSelector;

    ToggleButton m_AllSymButton;
    SkinHeader m_TopHeader;
    SkinControl m_TopAngleSkinControl;
    SkinControl m_TopStrengthSkinControl;
    SkinControl m_TopCurvatureSkinControl;

    SkinHeader m_RightHeader;
    SkinControl m_RightAngleSkinControl;
    SkinControl m_RightStrengthSkinControl;
    SkinControl m_RightCurvatureSkinControl;

    ToggleButton m_TBSymButton;
    SkinHeader m_BottomHeader;
    SkinControl m_BottomAngleSkinControl;
    SkinControl m_BottomStrengthSkinControl;
    SkinControl m_BottomCurvatureSkinControl;

    ToggleButton m_RLSymButton;
    SkinHeader m_LeftHeader;
    SkinControl m_LeftAngleSkinControl;
    SkinControl m_LeftStrengthSkinControl;
    SkinControl m_LeftCurvatureSkinControl;

};

class XSecViewScreen : public BasicScreen
{
public:
    XSecViewScreen( ScreenMgr* mgr );
    virtual ~XSecViewScreen() {}
    virtual bool Update();

protected:

    VSPGUI::VspSubGlWindow * m_GlWin;
};

#endif // !defined(SCREENBASE__INCLUDED_)
