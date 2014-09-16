//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "APIDefines.h"
#include "MaterialMgr.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Camera.h"
#include "SubGLWindow.h"
#include "MaterialEditScreen.h"
#include "UiBuilder.h"
#include "VSPWindow.h"
#include "VspScreenQt_p.h"
#include <QLabel>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QScrollArea>
#include <QGridLayout>

using namespace vsp;
using std::map;

//=====================================================================//
//=====================================================================//
//=====================================================================//

/// \todo Should use VSP_Window
class BasicScreenPrivate : public QWidget, public VspScreenQtPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC( BasicScreen )
    Q_PRIVATE_SLOT( self(), void SetUpdateFlag() )
public:
    QGridLayout layout;
    QLabel titleBox;

    BasicScreenPrivate( BasicScreen *, int w, int h, const string & title );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE { return true; }
};
VSP_DEFINE_PRIVATE( BasicScreen )

BasicScreenPrivate::BasicScreenPrivate( BasicScreen * q, int w, int h, const string & title ) :
    VspScreenQtPrivate( q ),
    layout( this )
{
    resize( w, h );
    layout.addWidget( &titleBox, layout.rowCount(), 0 );
    titleBox.setObjectName( "screenHeader" );
    q->SetTitle( title );
}

BasicScreen::BasicScreen( ScreenMgr* mgr, int w, int h, const string & title  ) :
    VspScreenQt( *new BasicScreenPrivate( this, w, h, title ), mgr )
{}

BasicScreen::BasicScreen( BasicScreenPrivate & dd, ScreenMgr * mgr ) :
    VspScreenQt( dd, mgr )
{}

/// Set Title Name on Window And Box
void BasicScreen::SetTitle( const string& title )
{
    Q_D( BasicScreen );
    if ( title != d->titleBox.text().toStdString() )
    {
        d->titleBox.setText( title.c_str() );
        d->setWindowTitle( title.c_str() );
    }
}

BasicScreen::~BasicScreen() {}

//=====================================================================//
//=====================================================================//
//=====================================================================//

class TabScreenPrivate : public BasicScreenPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC( TabScreen )
public:
    enum { TAB_H = 25 };

    QTabWidget tabs;

    TabScreenPrivate( TabScreen *, int w, int h, const std::string & title, int baseymargin = 0 );
    static QWidget* MakeTab( const string& title );
};
VSP_DEFINE_PRIVATE( TabScreen )

TabScreenPrivate::TabScreenPrivate( TabScreen * q, int w, int h, const std::string & title, int baseymargin ) :
    BasicScreenPrivate( q, w, h, title )
{
    layout.addWidget( &tabs, layout.rowCount(), 0 );
    tabs.setObjectName( "menuTabs" );
}

TabScreen::TabScreen( ScreenMgr* mgr, int w, int h, const string & title, int baseymargin ) :
    BasicScreen( *new TabScreenPrivate( this, w, h, title, baseymargin ), mgr )
{}

TabScreen::TabScreen( TabScreenPrivate & dd, ScreenMgr * mgr ) :
    BasicScreen( dd, mgr )
{}

QWidget* TabScreenPrivate::MakeTab( const string& title )
{
    auto tab = new QWidget;
    tab->setWindowTitle( title.c_str() );
    return tab;
}

QWidget* TabScreen::AddTab( const string& title )
{
    auto tab = TabScreenPrivate::MakeTab( title );
    d_func()->tabs.addTab( tab, tab->windowTitle() );
    return tab;
}

/// Insert a Tab
QWidget* TabScreen::AddTab( const string& title, int indx )
{
    auto tab = TabScreenPrivate::MakeTab( title );
    d_func()->tabs.insertTab( indx, tab, title.c_str() );
    return tab;
}

void TabScreen::RemoveTab( QWidget* tab )
{
    Q_D( TabScreen );
    d->tabs.removeTab( d->tabs.indexOf( tab ) );
}

void TabScreen::AddTab( QWidget* tab )
{
    d_func()->tabs.addTab( tab, tab->windowTitle() );
}

/// Insert a Tab
void TabScreen::AddTab( QWidget* tab, int indx )
{
    d_func()->tabs.insertTab( indx, tab, tab->windowTitle() );
}

/// Get Tab At Index
QWidget * TabScreen::GetTab( int index )
{
    return d_func()->tabs.widget( index );
}

QWidget* TabScreen::AddSubGroup( QWidget* group, int border )
{
    Q_D( TabScreen );
    if ( !group ) return NULL;

    QRect cli = group->rect();
    cli.adjust( border, border, -border, -border );
    QWidget * sub_group = new QWidget( group );
    sub_group->setGeometry( cli );
    sub_group->show();
    return sub_group;
}

/// Add A Sub Scroll To a Tab
QScrollArea* TabScreen::AddSubScroll( QWidget* group, int border, int lessh )
{
    Q_D( TabScreen );
    if ( !group ) return NULL;

    QRect cli = group->rect();
    cli.adjust( border, border, -border, -border );
    QScrollArea * sub_group = new QScrollArea( group );
    sub_group->setGeometry( cli );
    sub_group->show();
    return sub_group;
}

TabScreen::~TabScreen() {}

//=====================================================================//
//=====================================================================//
//=====================================================================//

class GeomScreenPrivate : public TabScreenPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC( GeomScreen )
public:

    //==== Gen Tab ====//
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

    QListWidget* m_SetBrowser;

    //==== XForm Tab ====//
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
    UiGroup * m_CurSubDispGroup;
    QTableWidget* m_SubSurfBrowser;
    TriggerButton m_DelSubSurfButton;
    TriggerButton m_AddSubSurfButton;
    Choice m_SubSurfChoice;

    UiGroup m_SSCommonGroup;
    StringInput m_SubNameInput;

    // SS_Line
    UiGroup m_SSLineGroup;
    SliderInput m_SSLineConstSlider; // Either Constant U or W
    ToggleButton m_SSLineConstUButton;
    ToggleButton m_SSLineConstWButton;
    ToggleRadioGroup m_SSLineConstToggleGroup;

    ToggleButton m_SSLineGreaterToggle;
    ToggleButton m_SSLineLessToggle;
    ToggleRadioGroup m_SSLineTestToggleGroup;

    // SS_Rectangle
    UiGroup m_SSRecGroup;
    SliderInput m_SSRecCentUSlider;
    SliderInput m_SSRecCentWSlider;
    SliderInput m_SSRecULenSlider;
    SliderInput m_SSRecWLenSlider;
    SliderAdjRangeInput m_SSRecThetaSlider;
    ToggleButton m_SSRecInsideButton;
    ToggleButton m_SSRecOutsideButton;
    ToggleRadioGroup m_SSRecTestToggleGroup;

    // SS_Ellipse
    UiGroup m_SSEllGroup;
    SliderInput m_SSEllCentUSlider;
    SliderInput m_SSEllCentWSlider;
    SliderInput m_SSEllULenSlider;
    SliderInput m_SSEllWLenSlider;
    SliderInput m_SSEllTessSlider;
    SliderAdjRangeInput m_SSEllThetaSlider;
    ToggleButton m_SSEllInsideButton;
    ToggleButton m_SSEllOutsideButton;
    ToggleRadioGroup m_SSEllTestToggleGroup;

    GeomScreenPrivate( GeomScreen * q, int w, int h, const string & title );
    bool Update() Q_DECL_OVERRIDE;
    void GuiDeviceCallBack( GuiDevice* device ) Q_DECL_OVERRIDE;
    void SubSurfDispGroup( UiGroup * group );
    void UpdateMaterialNames();

    Q_SLOT void on_SubSurfBrowser_currentCellChanged( int row )
    {
        SubSurfaceMgr.SetCurrSubSurfInd( row );
        SubSurface* sub_surf = geom()->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        if ( sub_surf )
        {
            m_SubSurfChoice.SetVal( sub_surf->GetType() );
        }
    }
    Q_SLOT void on_SetBrowser_currentIndexChanged( int row )
    {
        bool flag = m_SetBrowser->currentItem()->checkState() == Qt::Checked;
        geom()->SetSetFlag( row, flag );
    }
};
VSP_DEFINE_PRIVATE( GeomScreen )

GeomScreenPrivate::GeomScreenPrivate( GeomScreen * q, int w, int h, const string &title ) :
    TabScreenPrivate( q, w, h, title )
{
    QWidget* gen_tab = q->AddTab( "Gen" );
    QWidget* xform_tab = q->AddTab( "XForm" );
    QWidget* subsurf_tab = q->AddTab( "Sub" );
    m_SubSurfTab_ind = tabs.count() - 1;
    m_SubSurfBrowser->setObjectName( "SubSurfBrowser" );
    m_SetBrowser->setObjectName( "SetBrowser" );
    ConnectUpdateFlag( m_SubSurfBrowser );
    ConnectUpdateFlag( m_SetBrowser );

    //==== Gen Group Layout ====//
    UiBuilder gen( gen_tab );
    gen.AddDividerBox( "Name & Color" );
    gen.AddInput( m_NameInput, "Name:" );
    gen.AddYGap();
    gen.AddColorPicker( m_ColorPicker );
    gen.AddYGap();
    UpdateMaterialNames();

    gen.StartLabelAlignment();
    gen.StartLine();
    gen.SetNextExpanding();
    gen.AddChoice( m_MaterialChoice, "Material:" );
    gen.AddButton( m_CustomMaterialButton, "Custom" );
    gen.AddYGap();

    m_ExportNameChoice.AddItem( "NONE" );
    m_ExportNameChoice.AddItem( "WING" );
    m_ExportNameChoice.AddItem( "FUSELAGE" );
    m_ExportNameChoice.AddItem( "TAIL" );
    m_ExportNameChoice.AddItem( "CANOPY" );
    m_ExportNameChoice.AddItem( "POD" );
    gen.SetNextExpanding();
    gen.AddChoice( m_ExportNameChoice, "Export Name:" );
    gen.AddYGap();
    gen.EndLabelAlignment();

    gen.AddDividerBox( "Tesselation" );
    gen.AddSlider( m_NumUSlider, "Num_U", 100, " %5.0f" );
    gen.AddSlider( m_NumWSlider, "Num_W", 100, " %5.0f" );

    gen.AddYGap();
    gen.AddDividerBox( "Mass Properties" );

    gen.StartLine();

    gen.StartColumn();
    gen.AddInput( m_DensityInput, "Density", " %7.5f" );
    gen.AddCounter( m_PriorCounter, "Priority" );
    gen.EndColumn();

    gen.StartColumn();
    gen.AddButton( m_ThinShellButton, "Thin Shell" );
    gen.AddInput( m_ShellMassAreaInput, "Mass/Area", " %7.5f" );
    gen.EndColumn();

    gen.AddYGap();

    gen.AddDividerBox( "Set Export/Analysis" );
    m_SetBrowser = gen.AddCheckBrowser();

    gen_tab->show();

    //==== XForm Layout ====//
    UiBuilder xform( xform_tab );
    xform.AddDividerBox( "Transforms" );

    xform.StartLabelAlignment();
    xform.StartLine();
    xform.AddLabel( "Coord System:" );
    xform.AddButton( m_XFormRelativeToggle, "Rel" );
    xform.AddButton( m_XFormAbsoluteToggle, "Abs" );
    xform.AddYGap();
    xform.EndLabelAlignment();

    m_XFormAbsRelToggle.Init( q );
    m_XFormAbsRelToggle.AddButton( m_XFormAbsoluteToggle.GetFlButton() );
    m_XFormAbsRelToggle.AddButton( m_XFormRelativeToggle.GetFlButton() );

    xform.AddSlider( m_XLocSlider, "XLoc", 10.0, "%7.3f" );
    xform.AddSlider( m_YLocSlider, "YLoc", 10.0, "%7.3f" );
    xform.AddSlider( m_ZLocSlider, "ZLoc", 10.0, "%7.3f" );
    xform.AddYGap();
    xform.AddSlider( m_XRotSlider, "XRot", 10.0, "%7.3f" );
    xform.AddSlider( m_YRotSlider, "YRot", 10.0, "%7.3f" );
    xform.AddSlider( m_ZRotSlider, "ZRot", 10.0, "%7.3f" );
    xform.AddYGap();

    xform.StartLabelAlignment();
    xform.StartLine();
    xform.AddSlider( m_RotOriginSlider, "Rot Origin(X)", 1.0, "%5.3f" );
    xform.EndLine();
    xform.AddYGap();
    xform.EndLabelAlignment();

    xform.AddDividerBox( "Symmetry" );

    xform.StartLine();
    xform.AddLabel( "Planar:" );
    xform.AddButton( m_XYSymToggle, "XY", vsp::SYM_XY );
    xform.AddButton( m_XZSymToggle, "XZ", vsp::SYM_XZ );
    xform.AddButton( m_YZSymToggle, "YZ", vsp::SYM_YZ );
    xform.AddYGap();

    xform.StartLine();
    xform.AddLabel( "Axial:" );
    xform.AddButton( m_AxialNoneToggle, "None" );
    xform.AddButton( m_AxialXToggle, "X" );
    xform.AddButton( m_AxialYToggle, "Y" );
    xform.AddButton( m_AxialZToggle, "Z" );
    xform.EndLine();

    m_AxialToggleGroup.Init( q );
    m_AxialToggleGroup.AddButton( m_AxialNoneToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialXToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialYToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialZToggle.GetFlButton() );

    //==== Because SymAxFlag is Not 0-N Need To Map Vals ====//
    vector< int > axial_val_map;
    axial_val_map.push_back( 0 );
    axial_val_map.push_back( vsp::SYM_ROT_X );
    axial_val_map.push_back( vsp::SYM_ROT_Y );
    axial_val_map.push_back( vsp::SYM_ROT_Z );
    m_AxialToggleGroup.SetValMapVec( axial_val_map );

    xform.AddSlider( m_AxialNSlider, "N", 100, " %5.0f" );
    xform.AddYGap();

    xform.AddDividerBox( "Scale Factor" );

    xform.StartLine();
    xform.AddSlider( m_ScaleSlider, "Scale", 1, " %5.4f" );
    xform.AddButton( m_ScaleResetButton, "Reset" );
    xform.AddButton( m_ScaleAcceptButton, "Accept" );
    xform.AddYGap();

    xform.AddDividerBox( "Attach To Parent" );

    xform.StartLine();
    xform.AddLabel( "Translate:" );
    xform.AddButton( m_TransNoneButton, "None" );
    xform.AddButton( m_TransCompButton, "Comp" );
    xform.AddButton( m_TransUVButton, "UW" );
    xform.AddYGap();

    m_TransToggleGroup.Init( q );
    m_TransToggleGroup.AddButton( m_TransNoneButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransCompButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransUVButton.GetFlButton() );

    xform.AddLabel( "Rotate:" );
    xform.AddButton( m_RotNoneButton, "None" );
    xform.AddButton( m_RotCompButton, "Comp" );
    xform.AddButton( m_RotUVButton, "UW" );
    xform.AddYGap();

    m_RotToggleGroup.Init( q );
    m_RotToggleGroup.AddButton( m_RotNoneButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotCompButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotUVButton.GetFlButton() );

    xform.AddSlider( m_AttachUSlider, "U", 1, " %5.4f" );
    xform.AddSlider( m_AttachVSlider, "W", 1, " %5.4f" );

    //=============== SubSurface Tab ===================//
    UiBuilder subsurf( subsurf_tab );
    subsurf.AddDividerBox( "Sub-Surface List" );
    m_SubSurfBrowser = new QTableWidget();
    subsurf.AddBrowser( m_SubSurfBrowser );
    subsurf.AddYGap();

    subsurf.AddButton( m_DelSubSurfButton, "Delete" );
    subsurf.AddYGap();

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_LINE ) );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_RECTANGLE ) );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_ELLIPSE ) );

    subsurf.StartLine();
    subsurf.AddChoice( m_SubSurfChoice, "Type" );
    subsurf.AddButton( m_AddSubSurfButton, "Add" );
    subsurf.AddYGap();

    subsurf.StartGroup();
    subsurf.AddDividerBox( "Sub-Surface Parameters" );
    subsurf.AddInput( m_SubNameInput, "Name" );
    m_SSCommonGroup = subsurf.EndGroup();

    // Indivdual SubSurface Parameters
    subsurf.StartStack();

    //==== SSLine ====//
    subsurf.StartStackPage();
    subsurf.StartLabelAlignment();
    subsurf.StartLine();
    subsurf.AddLabel( "Line Type" );
    subsurf.AddButton( m_SSLineConstUButton, "U" );
    subsurf.AddButton( m_SSLineConstWButton, "W" );
    subsurf.EndLine();

    m_SSLineConstToggleGroup.Init( q );
    m_SSLineConstToggleGroup.AddButton( m_SSLineConstUButton.GetFlButton() );
    m_SSLineConstToggleGroup.AddButton( m_SSLineConstWButton.GetFlButton() );

    subsurf.StartLine();
    subsurf.AddLabel( "Test" );
    subsurf.AddButton( m_SSLineGreaterToggle, "Greater" );
    subsurf.AddButton( m_SSLineLessToggle, "Less" );
    subsurf.EndLine();

    m_SSLineTestToggleGroup.Init( q );
    m_SSLineTestToggleGroup.AddButton( m_SSLineGreaterToggle.GetFlButton() );
    m_SSLineTestToggleGroup.AddButton( m_SSLineLessToggle.GetFlButton() );

    subsurf.AddSlider( m_SSLineConstSlider, "Value", 1, "%5.4f" );
    m_SSLineGroup = subsurf.EndStackPage();

    //==== SSRectangle ====//
    subsurf.StartStackPage();
    subsurf.StartLabelAlignment();
    subsurf.StartLine();
    subsurf.AddLabel( "Tag" );
    subsurf.AddButton( m_SSRecInsideButton, "Inside" );
    subsurf.AddButton( m_SSRecOutsideButton, "Outside" );
    subsurf.EndLine();

    m_SSRecTestToggleGroup.Init( q );
    m_SSRecTestToggleGroup.AddButton( m_SSRecInsideButton.GetFlButton() );
    m_SSRecTestToggleGroup.AddButton( m_SSRecOutsideButton.GetFlButton() );

    subsurf.AddSlider( m_SSRecCentUSlider, "Center U", 1, "%5.4f" );
    subsurf.AddSlider( m_SSRecCentWSlider, "Center W", 1, "%5.4f" );
    subsurf.AddSlider( m_SSRecULenSlider, "U Length", 1, "%5.4f" );
    subsurf.AddSlider( m_SSRecWLenSlider, "W Length", 1, "%5.4f" );
    subsurf.AddSlider( m_SSRecThetaSlider, "Theta", 25, "%5.4f" );
    m_SSRecGroup = subsurf.EndStackPage();

    //==== SS_Ellipse ====//
    subsurf.StartStackPage();
    subsurf.StartLabelAlignment();

    subsurf.StartLine();
    subsurf.AddLabel( "Tag" );
    subsurf.AddButton( m_SSEllInsideButton, "Inside" );
    subsurf.AddButton( m_SSEllOutsideButton, "Outside" );
    subsurf.EndLine();

    m_SSEllTestToggleGroup.Init( q );
    m_SSEllTestToggleGroup.AddButton( m_SSEllInsideButton.GetFlButton() );
    m_SSEllTestToggleGroup.AddButton( m_SSEllOutsideButton.GetFlButton() );

    subsurf.AddSlider( m_SSEllTessSlider, "Num Points", 100, "%5.0f" );
    subsurf.AddSlider( m_SSEllCentUSlider, "Center U", 1, "%5.4f" );
    subsurf.AddSlider( m_SSEllCentWSlider, "Center W", 1, "%5.4f" );
    subsurf.AddSlider( m_SSEllULenSlider, "U Length", 1, "%5.4f" );
    subsurf.AddSlider( m_SSEllWLenSlider, "W Length", 1, "%5.4f" );
    subsurf.AddSlider( m_SSEllThetaSlider, "Theta", 25, "%5.4f" );
    m_SSEllGroup = subsurf.EndStackPage();

    EnableUpdateFlags();
    BlockSignalsInUpdates();
    QMetaObject::connectSlotsByName( this );
}

GeomScreen::GeomScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    TabScreen( mgr, w, h, title )
{
#if 0
    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);
    vsp_win->SetGeomScreenFlag( true );
#endif
}

GeomScreen::GeomScreen( GeomScreenPrivate & dd, ScreenMgr* mgr ) :
    TabScreen( dd, mgr )
{
#if 0
    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);
    vsp_win->SetGeomScreenFlag( true );
#endif
}

bool GeomScreenPrivate::Update()
{
    Q_Q( GeomScreen );
    Geom * geom_ptr = geom();
    if ( !geom_ptr ) return false;

    TabScreenPrivate::Update();

    //==== Name ===//
    m_NameInput.Update(  geom_ptr->GetName() );

    //==== Color ====//
    m_ColorPicker.Update( geom_ptr->GetColor() );

    //==== Material ====//
    Material * mat = geom_ptr->GetMaterial();

    UpdateMaterialNames();
    m_MaterialChoice.UpdateItems();

    m_MaterialChoice.SetVal( 0 );

    std::vector< std::string > choices = m_MaterialChoice.GetItems();
    for ( int i = 0; i < (int)choices.size(); i++ )
    {
        if( mat->m_Name == choices[i] )
        {
            m_MaterialChoice.SetVal(i);
            break;
        }
    }

    //==== XForms ====//
    m_ScaleSlider.Update( geom_ptr->m_Scale.GetID() );

    //===== Rel of Abs ====//
    m_XFormAbsRelToggle.Update( geom_ptr->m_AbsRelFlag.GetID() );
    geom_ptr->DeactivateXForms();
    if ( geom_ptr->m_AbsRelFlag() ==  GeomXForm::RELATIVE_XFORM )
    {
        m_XLocSlider.Update( 1, geom_ptr->m_XRelLoc.GetID(), geom_ptr->m_XLoc.GetID() );
        m_YLocSlider.Update( 1, geom_ptr->m_YRelLoc.GetID(), geom_ptr->m_YLoc.GetID() );
        m_ZLocSlider.Update( 1, geom_ptr->m_ZRelLoc.GetID(), geom_ptr->m_ZLoc.GetID() );
        m_XRotSlider.Update( 1, geom_ptr->m_XRelRot.GetID(), geom_ptr->m_XRot.GetID() );
        m_YRotSlider.Update( 1, geom_ptr->m_YRelRot.GetID(), geom_ptr->m_YRot.GetID() );
        m_ZRotSlider.Update( 1, geom_ptr->m_ZRelRot.GetID(), geom_ptr->m_ZRot.GetID() );
    }
    else
    {
        m_XLocSlider.Update( 2, geom_ptr->m_XRelLoc.GetID(), geom_ptr->m_XLoc.GetID() );
        m_YLocSlider.Update( 2, geom_ptr->m_YRelLoc.GetID(), geom_ptr->m_YLoc.GetID() );
        m_ZLocSlider.Update( 2, geom_ptr->m_ZRelLoc.GetID(), geom_ptr->m_ZLoc.GetID() );
        m_XRotSlider.Update( 2, geom_ptr->m_XRelRot.GetID(), geom_ptr->m_XRot.GetID() );
        m_YRotSlider.Update( 2, geom_ptr->m_YRelRot.GetID(), geom_ptr->m_YRot.GetID() );
        m_ZRotSlider.Update( 2, geom_ptr->m_ZRelRot.GetID(), geom_ptr->m_ZRot.GetID() );
    }
    m_RotOriginSlider.Update( geom_ptr->m_Origin.GetID() );

    //==== Symmetry ====//
    m_XYSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_XZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_YZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_AxialToggleGroup.Update( geom_ptr->m_SymAxFlag.GetID() );
    m_AxialNSlider.Update( geom_ptr->m_SymRotN.GetID() );

    //==== Mass Props ====//
    m_DensityInput.Update( geom_ptr->m_Density.GetID() );
    m_ShellMassAreaInput.Update( geom_ptr->m_MassArea.GetID() );
    m_ThinShellButton.Update( geom_ptr->m_ShellFlag.GetID() );
    m_PriorCounter.Update( geom_ptr->m_MassPrior.GetID() );

    //==== Attachments ====//
    m_TransToggleGroup.Update( geom_ptr->m_TransAttachFlag.GetID() );
    m_RotToggleGroup.Update( geom_ptr->m_RotAttachFlag.GetID() );
    m_AttachUSlider.Update( geom_ptr->m_ULoc.GetID() );
    m_AttachVSlider.Update( geom_ptr->m_WLoc.GetID() );


    if ( geom_ptr->m_ShellFlag.Get() )
    {
        m_ShellMassAreaInput.Activate();
    }
    else
    {
        m_ShellMassAreaInput.Deactivate();
    }

    m_NumUSlider.Update( geom_ptr->m_TessU.GetID() );
    m_NumWSlider.Update( geom_ptr->m_TessW.GetID() );

    //==== Set Browser ====//
    geom_ptr->UpdateSets();
    vector< string > set_name_vec = veh()->GetSetNameVec();
    vector< bool > set_flag_vec = geom_ptr->GetSetFlags();

    assert( set_name_vec.size() == set_flag_vec.size() );

    //==== Load Set Names and Values ====//
    m_SetBrowser->clear();
    for ( int i = SET_SHOWN ; i < ( int )set_name_vec.size() ; i++ )
    {
        QListWidgetItem * item = new QListWidgetItem( set_name_vec[i].c_str() );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
        item->setCheckState( set_flag_vec[i] ? Qt::Checked : Qt::Unchecked );
        m_SetBrowser->addItem( item );
    }

    //================= SubSurfaces Tab ===================//
    SubSurface* subsurf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );

    if ( subsurf )
    {
        m_SubNameInput.Update( subsurf->GetName() );
        if ( subsurf->GetType() == SubSurface::SS_LINE )
        {
            SSLine* ssline = dynamic_cast< SSLine* >( subsurf );
            assert( ssline );

            m_SSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
            m_SSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
            m_SSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
            SubSurfDispGroup( &m_SSLineGroup );

        }
        else if ( subsurf->GetType() == SubSurface::SS_RECTANGLE )
        {
            SSRectangle* ssrec = dynamic_cast< SSRectangle* >( subsurf );
            assert( subsurf );

            m_SSRecTestToggleGroup.Update( ssrec->m_TestType.GetID() );
            m_SSRecCentUSlider.Update( ssrec->m_CenterU.GetID() );
            m_SSRecCentWSlider.Update( ssrec->m_CenterW.GetID() );
            m_SSRecULenSlider.Update( ssrec->m_ULength.GetID() );
            m_SSRecWLenSlider.Update( ssrec->m_WLength.GetID() );
            m_SSRecThetaSlider.Update( ssrec->m_Theta.GetID() );
            SubSurfDispGroup( &m_SSRecGroup );
        }
        else if ( subsurf->GetType() == SubSurface::SS_ELLIPSE )
        {
            SSEllipse* ssell = dynamic_cast< SSEllipse* >( subsurf );
            assert( ssell );

            m_SSEllTestToggleGroup.Update( ssell->m_TestType.GetID() );
            m_SSEllTessSlider.Update( ssell->m_Tess.GetID() );
            m_SSEllCentUSlider.Update( ssell->m_CenterU.GetID() );
            m_SSEllCentWSlider.Update( ssell->m_CenterW.GetID() );
            m_SSEllULenSlider.Update( ssell->m_ULength.GetID() );
            m_SSEllWLenSlider.Update( ssell->m_WLength.GetID() );
            m_SSEllThetaSlider.Update( ssell->m_Theta.GetID() );
            SubSurfDispGroup( & m_SSEllGroup );
        }
    }
    else
    {
        SubSurfDispGroup( 0 );
    }

    //==== SubSurfBrowser ====//
    auto headers = QStringList() << "NAME" << "TYPE";
    m_SubSurfBrowser->clear();
    m_SubSurfBrowser->setColumnCount( 2 );
    m_SubSurfBrowser->setHorizontalHeaderLabels( headers );

    string ss_name, ss_type;

    vector<SubSurface*> subsurf_vec = geom_ptr->GetSubSurfVec();
    m_SubSurfBrowser->setRowCount( subsurf_vec.size() );
    for ( int i = 0; i < ( int )subsurf_vec.size() ; i++ )
    {
        ss_name = subsurf_vec[i]->GetName();
        ss_type = SubSurface::GetTypeName( subsurf_vec[i]->GetType() );
        auto name_item = new QTableWidgetItem( ss_name.c_str() );
        auto type_item = new QTableWidgetItem( ss_type.c_str() );
        m_SubSurfBrowser->setItem( i, 0, name_item );
        m_SubSurfBrowser->setItem( i, 1, type_item );
    }

    if ( geom_ptr->ValidSubSurfInd( SubSurfaceMgr.GetCurrSurfInd() ) )
    {
        m_SubSurfBrowser->selectRow( SubSurfaceMgr.GetCurrSurfInd() );
    }

    return true;
}

void GeomScreenPrivate::UpdateMaterialNames()
{
    std::vector<std::string> matNames;
    matNames = MaterialMgr.GetNames();

    m_MaterialChoice.ClearItems();
    m_MaterialChoice.AddItem( "DEFAULT" );
    for( int i = 0; i < (int) matNames.size(); i++ )
    {
        m_MaterialChoice.AddItem( matNames[i] );
    }
}

void GeomScreenPrivate::GuiDeviceCallBack( GuiDevice* device )
{
    Geom* geom_ptr = geom();
    if ( !geom_ptr ) return;
    if ( device == &m_ColorPicker )
    {
        vec3d c = m_ColorPicker.GetColor();
        geom_ptr->SetColor( ( int )c.x(), ( int )c.y(), ( int )c.z() );
    }
    else if ( device == &m_MaterialChoice )
    {
        int index = m_MaterialChoice.GetVal() - 1;

        Material mat;

        if( MaterialMgr.FindMaterial( index, mat ) )
        {
            geom_ptr->SetMaterial( mat.m_Name, mat.m_Ambi, mat.m_Diff, mat.m_Spec, mat.m_Emis, mat.m_Shininess );
        }
        else
        {
            geom_ptr->SetMaterialToDefault();
        }
    }
    else if ( device == &m_CustomMaterialButton )
    {
        static_cast< MaterialEditScreen*> ( GetScreenMgr()->GetScreen( ScreenMgr::VSP_MATERIAL_EDIT_SCREEN ) )->m_OrigColor = geom_ptr->GetMaterial()->m_Name;
        geom_ptr->GetMaterial()->m_Name = "Custom";
        GetScreenMgr()->ShowScreen( ScreenMgr::VSP_MATERIAL_EDIT_SCREEN );
    }
    else if ( device == &m_ScaleAcceptButton )
    {
        geom_ptr->AcceptScale();
    }
    else if ( device == &m_ScaleResetButton )
    {
        geom_ptr->ResetScale();
    }
    else if ( device == &m_NameInput )
    {
        geom_ptr->SetName( m_NameInput.GetString() );
    }
    else if ( device == &m_AddSubSurfButton )
    {
        SubSurface* ssurf = NULL;
        if ( m_SubSurfChoice.GetVal() == SubSurface::SS_LINE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_LINE );
        }
        else if ( m_SubSurfChoice.GetVal() == SubSurface::SS_RECTANGLE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_RECTANGLE );
        }
        else if ( m_SubSurfChoice.GetVal() == SubSurface::SS_ELLIPSE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_ELLIPSE );
        }

        if ( ssurf )
        {
            ssurf->Update();
        }
        SubSurfaceMgr.SetCurrSubSurfInd( geom_ptr->NumSubSurfs() - 1 );
    }
    else if ( device == &m_DelSubSurfButton )
    {
        geom_ptr->DelSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        SubSurfaceMgr.SetCurrSubSurfInd( geom_ptr->NumSubSurfs() - 1 );
    }
    else if ( device == &m_SubNameInput )
    {
        SubSurface* sub_surf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        if ( sub_surf )
        {
            sub_surf->SetName( m_SubNameInput.GetString() );
        }
    }

    SetUpdateFlag();
}

void GeomScreenPrivate::SubSurfDispGroup( UiGroup* group )
{
    if ( m_CurSubDispGroup == group && group )
    {
        return;
    }

    m_SSLineGroup.Hide();
    m_SSRecGroup.Hide();
    m_SSCommonGroup.Hide();
    m_SSEllGroup.Hide();

    m_CurSubDispGroup = group;
    if ( group )
    {
        group->Show();
        m_SSCommonGroup.Show(); // Always show the Common Group if any other subsurface group is being displayed.
    }
}

GeomScreen::~GeomScreen() {}

//=====================================================================//
//=====================================================================//
//=====================================================================//

class SkinScreenPrivate : public GeomScreenPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC( SkinScreen )
public:

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

    SkinScreenPrivate( SkinScreen *, int w, int h, const string & title );
    bool Update() Q_DECL_OVERRIDE;
    void GuiDeviceCallBack( GuiDevice * ) Q_DECL_OVERRIDE;
};
VSP_DEFINE_PRIVATE( SkinScreen )

SkinScreenPrivate::SkinScreenPrivate( SkinScreen * q, int w, int h, const string &title ) :
    GeomScreenPrivate( q, w, h, title )
{
    QWidget* skin_tab = q->AddTab( "Skinning" );
    UiBuilder skin( skin_tab );

    skin.AddDividerBox( "Skin Cross Section" );
    skin.AddIndexSelector( m_SkinIndexSelector );
    skin.AddYGap();
    skin.AddYGap();

    skin.StartLine();
    skin.SetNextExpanding();
    skin.AddDividerBox( "Top Side" );
    skin.AddButton( m_AllSymButton, "All Sym" );
    skin.EndLine();
    skin.AddSkinHeader( m_TopHeader );
    skin.AddSkinControl( m_TopAngleSkinControl, "Angle", 30, "%6.5f");
    skin.AddSkinControl( m_TopStrengthSkinControl, "Strength", 30, "%6.5f");
    skin.AddSkinControl( m_TopCurvatureSkinControl, "Curvature", 30, "%6.5f");
    skin.AddYGap();

    skin.AddDividerBox( "Right Side" );
    skin.AddSkinHeader( m_RightHeader );
    skin.AddSkinControl( m_RightAngleSkinControl, "Angle", 30, "%6.5f");
    skin.AddSkinControl( m_RightStrengthSkinControl, "Strength", 30, "%6.5f");
    skin.AddSkinControl( m_RightCurvatureSkinControl, "Curvature", 30, "%6.5f");
    skin.AddYGap();

    skin.StartLine();
    skin.SetNextExpanding();
    skin.AddDividerBox( "Bottom Side" );
    skin.AddButton( m_TBSymButton, "T/B Sym" );
    skin.EndLine();
    skin.AddSkinHeader( m_BottomHeader );
    skin.AddSkinControl( m_BottomAngleSkinControl, "Angle", 30, "%6.5f");
    skin.AddSkinControl( m_BottomStrengthSkinControl, "Strength", 30, "%6.5f");
    skin.AddSkinControl( m_BottomCurvatureSkinControl, "Curvature", 30, "%6.5f");
    skin.AddYGap();

    skin.StartLine();
    skin.SetNextExpanding();
    skin.AddDividerBox( "Left Side" );
    skin.AddButton( m_RLSymButton, "R/L Sym" );
    skin.EndLine();

    skin.AddSkinHeader( m_LeftHeader );
    skin.AddSkinControl( m_LeftAngleSkinControl, "Angle", 30, "%6.5f");
    skin.AddSkinControl( m_LeftStrengthSkinControl, "Strength", 30, "%6.5f");
    skin.AddSkinControl( m_LeftCurvatureSkinControl, "Curvature", 30, "%6.5f");
}

SkinScreen::SkinScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    GeomScreen( *new SkinScreenPrivate( this, w, h, title ), mgr )
{
}

SkinScreen::SkinScreen( SkinScreenPrivate & dd, ScreenMgr* mgr ) :
    GeomScreen( dd, mgr )
{
}

bool SkinScreenPrivate::Update()
{
    Geom* geom_ptr = geom();
    if ( !geom_ptr ) return false;

    GeomScreenPrivate::Update();

    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );

    //==== Skin & XSec Index Display ===//
    int xsid = geomxsec_ptr->GetActiveXSecIndex();
    m_SkinIndexSelector.SetIndex( xsid );

    SkinXSec* xs = ( SkinXSec* ) geomxsec_ptr->GetXSec( xsid );
    if ( xs )
    {
        //==== Skin ====//
        // Update Symmetry flags to Parms.
        m_AllSymButton.Update( xs->m_AllSymFlag.GetID() );
        m_TBSymButton.Update( xs->m_TBSymFlag.GetID() );
        m_RLSymButton.Update( xs->m_RLSymFlag.GetID() );

        // Update Controls to Parms.
        m_TopHeader.m_ContChoice->SetVal( xs->m_TopCont() );
        m_TopAngleSkinControl.Update( xs->m_TopLAngle.GetID(), xs->m_TopLAngleSet.GetID(), xs->m_TopLRAngleEq.GetID(), xs->m_TopRAngleSet.GetID(), xs->m_TopRAngle.GetID() );
        m_TopStrengthSkinControl.Update( xs->m_TopLStrength.GetID(), xs->m_TopLStrengthSet.GetID(), xs->m_TopLRStrengthEq.GetID(), xs->m_TopRStrengthSet.GetID(), xs->m_TopRStrength.GetID() );
        m_TopCurvatureSkinControl.Update( xs->m_TopLCurve.GetID(), xs->m_TopLCurveSet.GetID(), xs->m_TopLRCurveEq.GetID(), xs->m_TopRCurveSet.GetID(), xs->m_TopRCurve.GetID() );

        m_RightHeader.m_ContChoice->SetVal( xs->m_RightCont() );
        m_RightAngleSkinControl.Update( xs->m_RightLAngle.GetID(), xs->m_RightLAngleSet.GetID(), xs->m_RightLRAngleEq.GetID(), xs->m_RightRAngleSet.GetID(), xs->m_RightRAngle.GetID() );
        m_RightStrengthSkinControl.Update( xs->m_RightLStrength.GetID(), xs->m_RightLStrengthSet.GetID(), xs->m_RightLRStrengthEq.GetID(), xs->m_RightRStrengthSet.GetID(), xs->m_RightRStrength.GetID() );
        m_RightCurvatureSkinControl.Update( xs->m_RightLCurve.GetID(), xs->m_RightLCurveSet.GetID(), xs->m_RightLRCurveEq.GetID(), xs->m_RightRCurveSet.GetID(), xs->m_RightRCurve.GetID() );

        m_BottomHeader.m_ContChoice->SetVal( xs->m_BottomCont() );
        m_BottomAngleSkinControl.Update( xs->m_BottomLAngle.GetID(), xs->m_BottomLAngleSet.GetID(), xs->m_BottomLRAngleEq.GetID(), xs->m_BottomRAngleSet.GetID(), xs->m_BottomRAngle.GetID() );
        m_BottomStrengthSkinControl.Update( xs->m_BottomLStrength.GetID(), xs->m_BottomLStrengthSet.GetID(), xs->m_BottomLRStrengthEq.GetID(), xs->m_BottomRStrengthSet.GetID(), xs->m_BottomRStrength.GetID() );
        m_BottomCurvatureSkinControl.Update( xs->m_BottomLCurve.GetID(), xs->m_BottomLCurveSet.GetID(), xs->m_BottomLRCurveEq.GetID(), xs->m_BottomRCurveSet.GetID(), xs->m_BottomRCurve.GetID() );

        m_LeftHeader.m_ContChoice->SetVal( xs->m_LeftCont() );
        m_LeftAngleSkinControl.Update( xs->m_LeftLAngle.GetID(), xs->m_LeftLAngleSet.GetID(), xs->m_LeftLRAngleEq.GetID(), xs->m_LeftRAngleSet.GetID(), xs->m_LeftRAngle.GetID() );
        m_LeftStrengthSkinControl.Update( xs->m_LeftLStrength.GetID(), xs->m_LeftLStrengthSet.GetID(), xs->m_LeftLRStrengthEq.GetID(), xs->m_LeftRStrengthSet.GetID(), xs->m_LeftRStrength.GetID() );
        m_LeftCurvatureSkinControl.Update( xs->m_LeftLCurve.GetID(), xs->m_LeftLCurveSet.GetID(), xs->m_LeftLRCurveEq.GetID(), xs->m_LeftRCurveSet.GetID(), xs->m_LeftRCurve.GetID() );

        // Note that other Deactivate() calls are made in SkinControl::Update
        // to handle Set and Equal flag settings.

        // Deactivate GUI for symmetry
        if( xs->m_AllSymFlag() )
        {
            m_TBSymButton.Deactivate();
            m_RLSymButton.Deactivate();

            m_RightHeader.Deactivate();
            m_RightAngleSkinControl.Deactivate();
            m_RightStrengthSkinControl.Deactivate();
            m_RightCurvatureSkinControl.Deactivate();

            m_BottomHeader.Deactivate();
            m_BottomAngleSkinControl.Deactivate();
            m_BottomStrengthSkinControl.Deactivate();
            m_BottomCurvatureSkinControl.Deactivate();

            m_LeftHeader.Deactivate();
            m_LeftAngleSkinControl.Deactivate();
            m_LeftStrengthSkinControl.Deactivate();
            m_LeftCurvatureSkinControl.Deactivate();
        }

        if( xs->m_TBSymFlag() )
        {
            m_BottomHeader.Deactivate();
            m_BottomAngleSkinControl.Deactivate();
            m_BottomStrengthSkinControl.Deactivate();
            m_BottomCurvatureSkinControl.Deactivate();
        }

        if( xs->m_RLSymFlag() )
        {
            m_LeftHeader.Deactivate();
            m_LeftAngleSkinControl.Deactivate();
            m_LeftStrengthSkinControl.Deactivate();
            m_LeftCurvatureSkinControl.Deactivate();
        }

        // Deactivate GUI for Set Strength -- doesn't make sense unless
        // Angle set is enabled.
        m_TopStrengthSkinControl.DeactivateSet();
        m_RightStrengthSkinControl.DeactivateSet();
        m_BottomStrengthSkinControl.DeactivateSet();
        m_LeftStrengthSkinControl.DeactivateSet();

        // Deactivate continuity control for first/last section.
        m_TopHeader.Activate();
        if ( xsid == 0 || xsid == (geomxsec_ptr->GetXSecSurf( 0 )->NumXSec() - 1) )
        {
            m_TopHeader.DeactiveContChoice();
        }

        // Deactivate RSet and Equality control to match continuity settings.
        // The RBL versions of these are redundant given current Code-Eli limitations
        // However, they are coded up now so fewer changes will be required later.
        if ( xs->m_TopCont() >= 1)
        {
            m_TopAngleSkinControl.DeactivateRSet();
            m_TopAngleSkinControl.DeactivateEqual();
            m_TopStrengthSkinControl.DeactivateRSet();
            m_TopStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_TopCont() >= 2)
        {
            m_TopCurvatureSkinControl.DeactivateRSet();
            m_TopCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_RightCont() >= 1)
        {
            m_RightAngleSkinControl.DeactivateRSet();
            m_RightAngleSkinControl.DeactivateEqual();
            m_RightStrengthSkinControl.DeactivateRSet();
            m_RightStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_RightCont() >= 2)
        {
            m_RightCurvatureSkinControl.DeactivateRSet();
            m_RightCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_BottomCont() >= 1)
        {
            m_BottomAngleSkinControl.DeactivateRSet();
            m_BottomAngleSkinControl.DeactivateEqual();
            m_BottomStrengthSkinControl.DeactivateRSet();
            m_BottomStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_BottomCont() >= 2)
        {
            m_BottomCurvatureSkinControl.DeactivateRSet();
            m_BottomCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_LeftCont() >= 1)
        {
            m_LeftAngleSkinControl.DeactivateRSet();
            m_LeftAngleSkinControl.DeactivateEqual();
            m_LeftStrengthSkinControl.DeactivateRSet();
            m_LeftStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_LeftCont() >= 2)
        {
            m_LeftCurvatureSkinControl.DeactivateRSet();
            m_LeftCurvatureSkinControl.DeactivateEqual();
        }


        // Deactivate GUI for non-top curves.  Code-Eli right now requires
        // things to be set per cross section.  This restriction may someday
        // be lifted -- while the above Strength restriction will not.
        m_RightAngleSkinControl.DeactivateSet();
        m_RightStrengthSkinControl.DeactivateSet();
        m_RightCurvatureSkinControl.DeactivateSet();

        m_BottomAngleSkinControl.DeactivateSet();
        m_BottomStrengthSkinControl.DeactivateSet();
        m_BottomCurvatureSkinControl.DeactivateSet();

        m_LeftAngleSkinControl.DeactivateSet();
        m_LeftStrengthSkinControl.DeactivateSet();
        m_LeftCurvatureSkinControl.DeactivateSet();

        // Deactivate GUI for non-top continuity control.  Code-Eli currently
        // requires continuity to be enforced on per cross section basis.
        m_RightHeader.DeactiveContChoice();
        m_BottomHeader.DeactiveContChoice();
        m_LeftHeader.DeactiveContChoice();
    }
    return true;
}

void SkinScreenPrivate::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = geom();
    if ( !geom_ptr )
    {
        return;
    }
    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );

    if ( gui_device == &m_SkinIndexSelector )
    {
        geomxsec_ptr->SetActiveXSecIndex( m_SkinIndexSelector.GetIndex() );
        geomxsec_ptr->Update();
    }
    else if ( gui_device == m_TopHeader.m_ContChoice )
    {
        int t = m_TopHeader.m_ContChoice->GetVal();
        int xsid = geomxsec_ptr->GetActiveXSecIndex();
        SkinXSec* xs = (SkinXSec*) geomxsec_ptr->GetXSec( xsid );
        if ( xs )
        {
            xs->m_TopCont.Set( t );
            xs->Update();
            geomxsec_ptr->Update();
        }
    }

    GeomScreenPrivate::GuiDeviceCallBack( gui_device );
}

SkinScreen::~SkinScreen() {}

//=====================================================================//
//=====================================================================//
//=====================================================================//

class XSecViewScreenPrivate : public BasicScreenPrivate
{
    Q_DECLARE_PUBLIC( XSecViewScreen )
    VSPGUI::VspSubGlWindow glWin;

    XSecViewScreenPrivate( XSecViewScreen * );
    bool Update() Q_DECL_OVERRIDE;
};
VSP_DEFINE_PRIVATE( XSecViewScreen )

XSecViewScreenPrivate::XSecViewScreenPrivate( XSecViewScreen * q ) :
    BasicScreenPrivate( q, 300, 300, "XSec View" ),
    glWin( DrawObj::VSP_XSEC_SCREEN )
{
    layout.addWidget( &glWin );

    VSPGraphic::Display * disp = glWin.getGraphicEngine()->getDisplay();
    disp->changeView( VSPGraphic::Common::VSP_CAM_TOP );
    disp->getViewport()->showGridOverlay( false );
    disp->getCamera()->setZoomValue(.005);
}

XSecViewScreen::XSecViewScreen( ScreenMgr* mgr ) :
    BasicScreen( *new XSecViewScreenPrivate( this ), mgr )
{
}

bool XSecViewScreenPrivate::Update()
{
    glWin.update();
    return true;
}

XSecViewScreen::~XSecViewScreen() {}

#include "ScreenBase.moc"
