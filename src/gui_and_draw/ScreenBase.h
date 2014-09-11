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

//==== Geom Screen ====//
class GeomScreen : public TabScreen
{
public:
    GeomScreen( ScreenMgr* mgr, int w, int h, const string & title );
    virtual ~GeomScreen()                               {}

    virtual bool Update( );
    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );
    virtual void SubSurfDispGroup( GroupLayout * group );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( GeomScreen* )data )->CallBack( w );
    }

    virtual void UpdateMaterialNames();

    //==== Group Layouts ====//
    GroupLayout m_GenLayout;
    GroupLayout m_Density;
    GroupLayout m_Shell;

    GroupLayout m_XFormLayout;
    GroupLayout m_SubSurfLayout;

    //==== Names, Color, Material ====//
    StringInput m_NameInput;
    ColorPicker m_ColorPicker;
    Choice m_MaterialChoice;
    TriggerButton m_CustomMaterialButton;
    Choice m_ExportNameChoice;

    //==== Tesselation ====//
    SliderInput m_NumUSlider;
    SliderInput m_NumWSlider;

    //==== Mass Props ====//
    Input m_DensityInput;
    Input m_ShellMassAreaInput;
    ToggleButton m_ThinShellButton;
    Counter m_PriorCounter;

    Fl_Check_Browser* m_SetBrowser;

    //==== XForms ====//
    ToggleButton m_XFormAbsoluteToggle;
    ToggleButton m_XFormRelativeToggle;
    ToggleRadioGroup m_XFormAbsRelToggle;

    SliderAdjRange2Input m_XLocSlider;
    SliderAdjRange2Input m_YLocSlider;
    SliderAdjRange2Input m_ZLocSlider;

    SliderAdjRange2Input m_XRotSlider;
    SliderAdjRange2Input m_YRotSlider;
    SliderAdjRange2Input m_ZRotSlider;

    SliderAdjRangeInput m_RotOriginSlider;

    CheckButtonBit m_XYSymToggle;
    CheckButtonBit m_XZSymToggle;
    CheckButtonBit m_YZSymToggle;

    ToggleButton m_AxialNoneToggle;
    ToggleButton m_AxialXToggle;
    ToggleButton m_AxialYToggle;
    ToggleButton m_AxialZToggle;
    ToggleRadioGroup m_AxialToggleGroup;
    SliderInput m_AxialNSlider;

    SliderInput m_ScaleSlider;
    TriggerButton m_ScaleResetButton;
    TriggerButton m_ScaleAcceptButton;

    //==== Attachments
    ToggleButton m_TransNoneButton;
    ToggleButton m_TransCompButton;
    ToggleButton m_TransUVButton;
    ToggleRadioGroup m_TransToggleGroup;

    ToggleButton m_RotNoneButton;
    ToggleButton m_RotCompButton;
    ToggleButton m_RotUVButton;
    ToggleRadioGroup m_RotToggleGroup;

    SliderInput m_AttachUSlider;
    SliderInput m_AttachVSlider;

    //====== SubSurface Tab =====//
    int m_SubSurfTab_ind;
    GroupLayout* m_CurSubDispGroup;
    Fl_Browser* m_SubSurfBrowser;
    TriggerButton m_DelSubSurfButton;
    TriggerButton m_AddSubSurfButton;
    Choice m_SubSurfChoice;

    GroupLayout m_SSCommonGroup;
    StringInput m_SubNameInput;

    // SS_Line
    GroupLayout m_SSLineGroup;
    SliderInput m_SSLineConstSlider; // Either Constant U or W
    ToggleButton m_SSLineConstUButton;
    ToggleButton m_SSLineConstWButton;
    ToggleRadioGroup m_SSLineConstToggleGroup;

    ToggleButton m_SSLineGreaterToggle;
    ToggleButton m_SSLineLessToggle;
    ToggleRadioGroup m_SSLineTestToggleGroup;

    // SS_Rectangle
    GroupLayout m_SSRecGroup;
    SliderInput m_SSRecCentUSlider;
    SliderInput m_SSRecCentWSlider;
    SliderInput m_SSRecULenSlider;
    SliderInput m_SSRecWLenSlider;
    SliderAdjRangeInput m_SSRecThetaSlider;
    ToggleButton m_SSRecInsideButton;
    ToggleButton m_SSRecOutsideButton;
    ToggleRadioGroup m_SSRecTestToggleGroup;

    // SS_Ellipse
    GroupLayout m_SSEllGroup;
    SliderInput m_SSEllCentUSlider;
    SliderInput m_SSEllCentWSlider;
    SliderInput m_SSEllULenSlider;
    SliderInput m_SSEllWLenSlider;
    SliderInput m_SSEllTessSlider;
    SliderAdjRangeInput m_SSEllThetaSlider;
    ToggleButton m_SSEllInsideButton;
    ToggleButton m_SSEllOutsideButton;
    ToggleRadioGroup m_SSEllTestToggleGroup;
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
