//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef UIBUILDER_H
#define UIBUILDER_H

#include "GuiDeviceQt.h"
#include <QScopedPointer>
#include <string>
#include <vector>

class UiGroupPrivate;
class UiBuilderPrivate;
class QListWidget;
class QStackedLayout;

/// A group of widgets that can be shown/hidden together.
class UiGroup {
    Q_DECLARE_PRIVATE( UiGroup )
    QScopedPointer< UiGroupPrivate > const d_ptr;
    friend class UiBuilder;
public:
    UiGroup();
    UiGroup( const UiGroup & );
    UiGroup & operator=( const UiGroup & );
    void Show();
    void Hide();
protected:
    UiGroup( const QList<QWidget*> & );
    UiGroup( QStackedLayout *, int );
};

/// Builds a user interface on an empty widget.
/// Manages the creation of layouts and control groups to provide a "nice" Ui.
class UiBuilder
{
    Q_DISABLE_COPY( UiBuilder )
    Q_DECLARE_PRIVATE( UiBuilder )
    QScopedPointer< UiBuilderPrivate > const d_ptr;
public:
    UiBuilder( QWidget * );
    ~UiBuilder();

    void SetNextExpanding();

    void StartLine();
    void EndLine();
    void StartColumn();
    void EndColumn();
    void StartLabelAlignment();
    void EndLabelAlignment();

    void StartGroup();
    UiGroup EndGroup();
    void StartStack();
    void StartStackPage();
    UiGroup EndStackPage();
    void EndStack();

    void AddYGap();
    void AddDividerBox( const std::string& text );
    void AddSlider(  SliderAdjRangeInput& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider(  SliderAdjRange2Input& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider(  FractParmSlider& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider( SliderInput & slider_input, const char* label, double range, const char* format );
    void AddButton(  CheckButton& check_button, const char* label );
    void AddButton(  ToggleButton& toggle_button, const char* label );
    void AddButton(  CheckButtonBit& check_bit_button, const char* label, int val );
    void AddButton(  TriggerButton& trigger_button, const char* label );
    void AddButton(  ParmButton& parm_button, const char* label );
    void AddInput(  StringInput& string_input, const char* label );
    void AddInput(  Input& input, const char* label, const char* format  );
    void AddOutput( StringOutput& string_output, const char* label );
    void AddIndexSelector( IndexSelector& selector );
    void AddColorPicker( ColorPicker& picker );
    void AddChoice( Choice & choice, const char* label );
    void AddCounter( Counter & count, const char* label );
    void AddLabel( const char* label );
    void AddParmPicker( ParmPicker & parm_picker );
    void AddDriverGroupBank( DriverGroupBank & driver_group, const vector < string > &labels,
                             double range, const char* format );
    void AddSkinControl( SkinControl & skin_control, const char* label, double range, const char* format );
    void AddSkinHeader( SkinHeader & skin_header );
    void AddGeomPicker( GeomPicker & geom_picker );

    void AddBrowser( QListWidget * browser );

#if 0
    //==== Add Another GroupLayout as a SubSet of This GroupLayout ====//
    //==== Subgroups can be Used To Create Multiple Column Layouts ====//
    void AddSubGroupLayout( GroupLayout& layout, int w, int h );
#endif

    //==== Standard Non-Parm Components ====//
    QListWidget* AddCheckBrowser();
};

#endif // UIBUILDER_H
