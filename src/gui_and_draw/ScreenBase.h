//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef SCREENBASE__INCLUDED_
#define SCREENBASE__INCLUDED_

#define QPoint QQPoint
#include <FL/Fl.H>
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
protected:
    GeomScreen( GeomScreenPrivate &, ScreenMgr * );
};


class SkinScreenPrivate;
class SkinScreen : public GeomScreen
{
    VSP_DECLARE_PRIVATE( SkinScreen )
public:
    SkinScreen( ScreenMgr* mgr, int w, int h, const string & title );
    ~SkinScreen();
protected:
    SkinScreen( SkinScreenPrivate &, ScreenMgr * );
};


class XSecViewScreen : public VspScreenFLTK
{
public:
    XSecViewScreen( ScreenMgr* mgr );
    virtual ~XSecViewScreen() {}
    virtual bool Update();

protected:

    VSPGUI::VspSubGlWindow * m_GlWin;
};

#endif // SCREENBASE__INCLUDED_
