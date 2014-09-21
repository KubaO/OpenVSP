//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef GUIDEVICEQT_H
#define GUIDEVICEQT_H

#include "VspScreenQt.h"
#include "GuiDevice.h"
#include <QObject>

class QWidget;
class QAbstractButton;
class AbstractDoubleSlider;
class QDoubleSpinBox;

class GuiDeviceQtPrivate;
class GuiDeviceQt : public GuiDevice
{
    Q_DECLARE_PRIVATE( GuiDeviceQt )
protected:
    QScopedPointer<GuiDeviceQtPrivate> const d_ptr;
public:
    void Init( VspScreenQt* screen );
    void Activate() Q_DECL_OVERRIDE;
    void Deactivate() Q_DECL_OVERRIDE;
    void SetWidth( int ) Q_DECL_OVERRIDE;
    int GetWidth() const Q_DECL_OVERRIDE;
    void SetX( int ) Q_DECL_OVERRIDE;
    int GetX() const Q_DECL_OVERRIDE;
#if 0
    virtual void OffsetX( int x );
#endif
    ~GuiDeviceQt();

protected:
    GuiDeviceQt( GuiDeviceQtPrivate & );

    //==== First Widget Is Assumed Resizable For Set Width =====//
    virtual void AddWidget( QWidget* w, bool resizable_flag = false );
    virtual void ClearAllWidgets();

    /// \todo Temporary for development purposes
    void SetValAndLimits( Parm* ) Q_DECL_OVERRIDE {}
};

class ToggleButtonPrivate;
/// Toggle type button with light. Linked to a (BoolParm).
class ToggleButton : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( ToggleButton )
public:
    ToggleButton();
    void Init( VspScreenQt* screen, QAbstractButton* button );
    QAbstractButton * GetButton();
    ~ToggleButton();
protected:
    void SetValAndLimits( Parm* p ) Q_DECL_OVERRIDE;
};

class SliderPrivate;
/// A linear slider. Linked to a (Parm/IntParm).
class Slider : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( Slider )
public:
    Slider();
    void Init( VspScreenQt* screen, AbstractDoubleSlider* slider_widget, double range );
    void SetRange( double range );
    ~Slider();
protected:
    Slider( SliderPrivate & );
    void SetValAndLimits( Parm* p ) Q_DECL_OVERRIDE;
};

class LogSliderPrivate;
/// A log10 slider. Linked to a (Parm/IntParm).
class LogSlider : public Slider
{
    VSP_DECLARE_PRIVATE( LogSlider )
public:
    LogSlider();
    ~LogSlider();
protected:
    void SetValAndLimits( Parm* p ) Q_DECL_OVERRIDE;
};

class InputPrivate;
class Input : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( Input )
public:
    Input();
    void Init( VspScreenQt* screen, QDoubleSpinBox* input, int decimals, QAbstractButton* parm_button = NULL );
    void SetDecimals( int );
    void SetButtonNameUpdate( bool flag );
    ~Input();
protected:
    void SetValAndLimits( Parm* p ) Q_DECL_OVERRIDE;
};

class ParmButtonPrivate;
/// A button with a label. Linked to a (Parm).
class ParmButton : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( ParmButton )
public:
    ParmButton();
    void Init( VspScreenQt* screen, QAbstractButton* button );
    void Update( const std::string& parm_id ) Q_DECL_OVERRIDE;
    void SetButtonNameUpdate( bool flag );
    ~ParmButton();
protected:
    void SetValAndLimits( Parm* p ) Q_DECL_OVERRIDE;
};

class SliderInputPrivate;
/// Combo of a Slider (or LogSlider) and Input and optional Parm Button. Linked to a (Parm).
class SliderInput : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( SliderInput )
public:
    SliderInput();
    void Init( VspScreenQt* screen, AbstractDoubleSlider*, QDoubleSpinBox*,
               double range, int decimals, QAbstractButton* parm_button = 0,
               bool log_slider = false );
    void Update( const std::string& parm_id ) Q_DECL_OVERRIDE;
    void SetRange( double range );
    void SetDecimals( int decimals );
    void SetButtonNameUpdate( bool flag );
    ~SliderInput();
protected:
    virtual void SetValAndLimits( Parm* ) Q_DECL_OVERRIDE;
};


class StringInputPrivate;
class StringInput : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( StringInput )
public:
    StringInput();
    void Update( const string & val );
    string GetString();
};


class ColorPickerPrivate;
class ColorPicker : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( ColorPicker )
public:
    ColorPicker();
    void Update( const vec3d & rgb );
    vec3d GetColor();
    vec3d GetIndexRGB( int index );
};


class TriggerButtonPrivate;
class TriggerButton : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( TriggerButton )
public:
    TriggerButton();
};


class ChoicePrivate;
class Choice : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( Choice )
public:
    Choice();

    void SetVal( int val );
    int  GetVal() const;
    void ClearItems();
    void AddItem( const string& item );
    vector< string > GetItems() const;
    void UpdateItems();
    void SetButtonNameUpdate( bool flag );
};


class CounterPrivate;
class Counter : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( Counter )
public:
    Counter();
};


class SliderAdjRangeInputPrivate;
class SliderAdjRangeInput : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( SliderAdjRangeInput )
public:
    SliderAdjRangeInput();
};


class SliderAdjRange2InputPrivate;
class SliderAdjRange2Input : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( SliderAdjRange2Input )
public:
    SliderAdjRange2Input();

    void Update( int slider_id, const string& parm_id_in1, const string& parm_id_in2 );
};


class ToggleRadioGroupPrivate;
class ToggleRadioGroup : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( ToggleRadioGroup )
public:
    ToggleRadioGroup();
    void AddButton( QAbstractButton* button );
    void SetValMapVec( vector< int > & val_map_vec );
};


class CheckButtonBitPrivate;
class CheckButtonBit : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( CheckButtonBit )
public:
    CheckButtonBit();
};


class IndexSelectorPrivate;
class IndexSelector : public GuiDeviceQt
{
    VSP_DECLARE_PRIVATE( IndexSelector )
public:
    IndexSelector();
    void SetIndex( int index );
    int  GetIndex() const;

    void SetMinMaxLimits( int min, int max );
    void SetBigSmallIncrements( int big_inc, int small_inc );
};

#endif // GUIDEVICEQT_H
