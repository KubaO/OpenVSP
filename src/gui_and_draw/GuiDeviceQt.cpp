//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GuiDeviceQt.h"
#include "AbstractDoubleSlider.h"
#include "ParmMgr.h"
#include "ScreenMgr.h"
#include "UiSignalBlocker.h"
#include "StlHelper.h"
#include <QObject>
#include <QDoubleSpinBox>
#include <QAbstractButton>

//
// GuiDeviceQt
//

class GuiDeviceQt::Private {
    VSP_DECLARE_PUBLIC( GuiDeviceQt )
public:
    GuiDeviceQt * const q_ptr;

    VspScreenQt* Screen;
    int ResizableWidgetIndex;
    vector< QWidget* > Widgets;

    Private( GuiDeviceQt * q ) :
        q_ptr( q ), Screen( 0 ), ResizableWidgetIndex( 0 ) {}
    virtual ~Private() {}
};
VSP_DEFINE_PRIVATE( GuiDeviceQt )

GuiDeviceQt::GuiDeviceQt( GuiDeviceQt::Private &d ) :
    d_ptr(&d)
{}

void GuiDeviceQt::Init( VspScreenQt * screen )
{
    d_func()->Screen = screen;
}

void GuiDeviceQt::AddWidget( QWidget* w, bool resizable_flag )
{
    V_D(GuiDeviceQt);
    if ( w )
    {
        d->Widgets.push_back( w );
    }
    if ( resizable_flag )
    {
        d->ResizableWidgetIndex = (int)d->Widgets.size() - 1;
    }
}

void GuiDeviceQt::ClearAllWidgets()
{
    d_func()->Widgets.clear();
}

void GuiDeviceQt::Activate()
{
    const_foreach ( auto widget, d_func()->Widgets )
    {
        widget->setEnabled( true );
    }
}

void GuiDeviceQt::Deactivate()
{
    const_foreach ( auto widget, d_func()->Widgets )
    {
        widget->setEnabled( false );
    }
}

int GuiDeviceQt::GetX() const
{
    Q_ASSERT( false );
}

void GuiDeviceQt::SetX(int)
{
    Q_ASSERT( false );
}

int GuiDeviceQt::GetWidth() const
{
    Q_ASSERT( false );
}

void GuiDeviceQt::SetWidth( int )
{
    Q_ASSERT( false );
}

GuiDeviceQt::~GuiDeviceQt() {}

//
// ToggleButton
//

class ToggleButton::Private : public QObject, public GuiDeviceQt::Private {
    Q_OBJECT
    VSP_DECLARE_PUBLIC( ToggleButton )
    QAbstractButton* Button;

    Private( ToggleButton* q ) :
        GuiDeviceQt::Private( q ), Button( 0 ) {}

    Q_SLOT void on_toggled( bool );
};
VSP_DEFINE_PRIVATE( ToggleButton )

ToggleButton::ToggleButton() :
    GuiDeviceQt( * new ToggleButton::Private( this ) )
{
    m_Type = GDEV_TOGGLE_BUTTON;
}

void ToggleButton::Init( VspScreenQt * screen, QAbstractButton * button )
{
    Q_ASSERT( button );
    V_D( ToggleButton );
    GuiDeviceQt::Init( screen );
    AddWidget( button );
    d->Button = button;
    d->Button->setCheckable( true );
    d->connect( d->Button, SIGNAL( toggled(bool) ), SLOT( on_toggled(bool) ) );
}

/// Set Button Value
void ToggleButton::SetValAndLimits( Parm* p )
{
    V_D( ToggleButton );
    Q_ASSERT( d->Button );
    if ( !p ) return;

    BoolParm* bool_p = dynamic_cast<BoolParm*>( p );
    Q_ASSERT( bool_p );

    UiSignalBlocker block( d->Button );
    d->Button->setChecked( bool_p->Get() );
}

void ToggleButton::Private::on_toggled( bool val )
{
    V_Q( ToggleButton );
    Parm* parm_ptr = q->SetParmID( q->m_ParmID );
    if ( !parm_ptr ) return;
    parm_ptr->SetFromDevice( val );
    Screen->GuiDeviceCallBack( q );
}

ToggleButton::~ToggleButton() {}

//
// Slider
//

class Slider::Private : public QObject, public GuiDeviceQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( Slider )
protected:
    AbstractDoubleSlider* slider;
    double Range;
    double MinBound;
    double MaxBound;

    Private( Slider * q );
    Q_SLOT virtual void on_valueChanged( double );
};
VSP_DEFINE_PRIVATE( Slider )

Slider::Private::Private( Slider * q ) :
    GuiDeviceQt::Private( q ),
    slider(0),
    Range(10.0),
    MinBound(0.0),
    MaxBound(0.0)
{}

Slider::Slider() :
    GuiDeviceQt( * new Slider::Private( this ) )
{
    m_Type = GDEV_SLIDER;
}

Slider::Slider(Private &q ) :
    GuiDeviceQt( q )
{
    m_Type = GDEV_SLIDER;
}

void Slider::Init(VspScreenQt* screen, AbstractDoubleSlider *slider_widget, double range )
{
    V_D( Slider );
    GuiDeviceQt::Init( screen );
    AddWidget( slider_widget );
    SetRange( range );
    d->slider = slider_widget;
    assert( d->slider );
    d->connect( d->slider, SIGNAL( valueChanged(double) ), SLOT( on_valueChanged(double) ) );
}

void Slider::SetValAndLimits( Parm* parm_ptr )
{
    V_D( Slider );
    assert( d->slider );
    UiSignalBlocker block( d->slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < d->MinBound || new_val > d->MaxBound )
    {
        d->MinBound = max( new_val - d->Range, parm_ptr->GetLowerLimit() );
        d->MaxBound = min( new_val + d->Range, parm_ptr->GetUpperLimit() );
        d->slider->setRange( d->MinBound, d->MaxBound );
    }

    if ( CheckValUpdate( new_val ) )
    {
        d->slider->setValue( new_val );
    }

    m_LastVal = new_val;
}

void Slider::SetRange( double range )
{
    V_D( Slider );
    UiSignalBlocker block( d->slider );
    d_func()->Range = range;
}

void Slider::Private::on_valueChanged( double new_val )
{
    V_Q( Slider );
    Parm* parm_ptr = q->SetParmID( q->m_ParmID );
    if ( !parm_ptr ) return;

    parm_ptr->SetFromDevice( new_val );

    Screen->GuiDeviceCallBack( q );
}

Slider::~Slider() {}

//
// LogSlider
//

class LogSlider::Private : public Slider::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( LogSlider )

    Private( LogSlider * q ) : Slider::Private( q ) {}
    void on_valueChanged( double ) Q_DECL_OVERRIDE;
};
VSP_DEFINE_PRIVATE( LogSlider )

LogSlider::LogSlider() : Slider( *new LogSlider::Private( this ))
{
    m_Type = GDEV_LOG_SLIDER;
}

void LogSlider::SetValAndLimits( Parm* parm_ptr )
{
    V_D( LogSlider );
    double m_Tol = 0.000001;
    assert( d->slider );
    UiSignalBlocker block( d->slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < ( d->MinBound - m_Tol ) || new_val > ( d->MaxBound + m_Tol ) )
    {
        d->MinBound = max( new_val - d->Range, parm_ptr->GetLowerLimit() );
        d->MaxBound = min( new_val + d->Range, parm_ptr->GetUpperLimit() );
        d->slider->setRange( log10( d->MinBound ), log10( d->MaxBound ) );
    }

    if ( CheckValUpdate( new_val ) )
    {
        d->slider->setValue( log10( new_val ) );
    }

    m_LastVal = new_val;
}


void LogSlider::Private::on_valueChanged( double val )
{
    V_Q( LogSlider );
    Parm* parm_ptr = q->SetParmID( q->m_ParmID );
    if ( !parm_ptr ) return;

    double new_val = pow( 10, val );
    parm_ptr->SetFromDevice( new_val );

    Screen->GuiDeviceCallBack( q );
}

LogSlider::~LogSlider() {}

//
// Input
//

class Input::Private : public QObject, public GuiDeviceQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( Input )
    QDoubleSpinBox* input;
    bool ParmButtonFlag;
    ParmButton ParmButton;

    Private( Input * q );
    Q_SLOT void on_valueChanged( double );
};
VSP_DEFINE_PRIVATE( Input )

Input::Private::Private( Input * q ) :
    GuiDeviceQt::Private( q ),
    input( 0 ),
    ParmButtonFlag( false )
{}

Input::Input() :
    GuiDeviceQt( *new Input::Private( this ) )
{
    m_Type = GDEV_INPUT;
}

void Input::Init( VspScreenQt* screen, QDoubleSpinBox* input, int decimals, QAbstractButton* parm_button )
{
    V_D( Input );
    assert( input );
    GuiDeviceQt::Init( screen );
    AddWidget( parm_button );
    AddWidget( input, true );

    d->input = input;
    d->input->setKeyboardTracking( false ); // FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE
    SetDecimals( decimals );
    d->connect( d->input, SIGNAL( valueChanged(double) ), SLOT( on_valueChanged(double) ) );

    d->ParmButtonFlag = false;
    if ( parm_button )
    {
        d->ParmButtonFlag = true;
        d->ParmButton.Init( screen, parm_button );
    }
}

void Input::SetDecimals( int decimals )
{
    V_D( Input );
    UiSignalBlocker block( d->input );
    if ( d->input ) d->input->setDecimals( decimals );
}

void Input::SetButtonNameUpdate( bool flag )
{
    d_func()->ParmButton.SetButtonNameUpdate( flag );
}

void Input::SetValAndLimits( Parm* parm_ptr )
{
    V_D( Input );
    assert( d->input );
    UiSignalBlocker block( d->input );
    double new_val = parm_ptr->Get();

    if ( CheckValUpdate( new_val ) )
    {
        d->input->setValue( new_val );
        d->input->setRange( parm_ptr->GetLowerLimit(), parm_ptr->GetUpperLimit() );
    }
    m_LastVal = new_val;

    if ( d->ParmButtonFlag )
    {
        d->ParmButton.Update( parm_ptr->GetID() );
    }
}

void Input::Private::on_valueChanged( double new_val )
{
    V_Q( Input );
    Parm* parm_ptr = q->SetParmID( q->m_ParmID );
    if ( !parm_ptr ) return;

    parm_ptr->SetFromDevice( new_val );
    q->m_LastVal = new_val;

    Screen->GuiDeviceCallBack( q );
}

Input::~Input() {}

//
// ParmButton
//

class ParmButton::Private : public QObject, public GuiDeviceQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( ParmButton )
    QAbstractButton* Button;
    bool ButtonNameUpdate;

    Private( ParmButton * q );
    Q_SLOT void on_clicked();
};
VSP_DEFINE_PRIVATE( ParmButton )

ParmButton::Private::Private( ParmButton * q ) :
    GuiDeviceQt::Private( q ),
    Button( 0 ),
    ButtonNameUpdate( false )
{}

ParmButton::ParmButton() :
    GuiDeviceQt( *new ParmButton::Private( this ) )
{
    m_Type = GDEV_PARM_BUTTON;
}

void ParmButton::Init( VspScreenQt* screen, QAbstractButton* button )
{
    V_D( ParmButton );
    GuiDeviceQt::Init( screen );
    AddWidget( button );
    d->Button = button;
    assert( d->Button );
    d->connect( d->Button, SIGNAL( clicked() ), SLOT( on_clicked() ) );
}

void ParmButton::Update( const string& parm_id )
{
    V_D( ParmButton );
    GuiDeviceQt::Update( parm_id );

    if( d->ButtonNameUpdate )
    {
        Parm* parm_ptr = SetParmID( parm_id );
        if ( parm_ptr )
        {
            d->Button->setText( parm_ptr->GetName().c_str() );
        }
    }
}

void ParmButton::SetButtonNameUpdate( bool flag )
{
    d_func()->ButtonNameUpdate = flag;
}

void ParmButton::SetValAndLimits( Parm* )
{}

void ParmButton::Private::on_clicked()
{
    V_Q( ParmButton );
    ParmMgr.SetActiveParm( q->m_ParmID );

    Screen->GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PARM_SCREEN );
#if 0
//  Screen->GetScreenMgr()->ShowParmScreen(parm_ptr, Fl::event_x_root(), Fl::event_y_root());
#endif
    Screen->GuiDeviceCallBack( q );
}

ParmButton::~ParmButton() {}

//
// SliderInput
//

class SliderInput::Private : public GuiDeviceQt::Private
{
    VSP_DECLARE_PUBLIC( SliderInput )
    Slider Slider;
    Input Input;

    bool LogSliderFlag;
    LogSlider LogSlider;

    bool ParmButtonFlag;
    ParmButton ParmButton;

    Private( SliderInput* );
};
VSP_DEFINE_PRIVATE( SliderInput )

SliderInput::Private::Private( SliderInput * q ) :
    GuiDeviceQt::Private( q ),
    LogSliderFlag( false ),
    ParmButtonFlag( false )
{
}

SliderInput::SliderInput() :
    GuiDeviceQt( *new SliderInput::Private( this ) )
{
    m_Type = GDEV_SLIDER_INPUT;
}

void SliderInput::Init(VspScreenQt* screen, AbstractDoubleSlider *slider, QDoubleSpinBox* input,
                          double range, int decimals, QAbstractButton* parm_button,
                          bool log_slider )
{
    V_D( SliderInput );
    GuiDeviceQt::Init( screen );

    if ( parm_button )
    {
        d->ParmButtonFlag = true;
        d->ParmButton.Init( screen, parm_button );
    }

    d->LogSliderFlag = log_slider;
    if ( d->LogSliderFlag )
    {
        d->LogSlider.Init( screen, slider, range );
    }
    else
    {
        d->Slider.Init( screen, slider, range );
    }

    d->Input.Init( screen, input, decimals );

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( slider, true );
    AddWidget( input );
}

void SliderInput::Update( const string& parm_id )
{
    V_D( SliderInput );
    if ( d->LogSliderFlag )
    {
        d->LogSlider.Update( parm_id );
    }
    else
    {
        d->Slider.Update( parm_id );
    }
    d->Input.Update( parm_id );
    if ( d->ParmButtonFlag )
    {
        d->ParmButton.Update( parm_id );
    }
}

void SliderInput::SetRange( double range )
{
    d_func()->Slider.SetRange( range );
}

void SliderInput::SetDecimals( int decimals )
{
    d_func()->Input.SetDecimals( decimals );
}

void SliderInput::SetButtonNameUpdate( bool flag )
{
    d_func()->ParmButton.SetButtonNameUpdate( flag );
}

void SliderInput::SetValAndLimits( Parm* ) {}

SliderInput::~SliderInput() {}

#include "GuiDeviceQt.moc"
