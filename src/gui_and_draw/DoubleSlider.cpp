//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "DoubleSlider.h"
#include <QSlider>
#include <QResizeEvent>
#include <QDebug>

class DoubleSlider::Private {
    VSP_DECLARE_PUBLIC( DoubleSlider )
    DoubleSlider * const q_ptr;
    QSlider slider;
    double minimum;
    double maximum;
    double span;
    double sourceSpan;

    Private( DoubleSlider * q );
    void on_valueChanged( int );
};
VSP_DEFINE_PRIVATE( DoubleSlider )

DoubleSlider::Private::Private( DoubleSlider * q ) :
    q_ptr( q ),
    slider( q ),
    minimum( 0.0 ),
    maximum( 1.0 ),
    span( maximum - minimum )
{
    slider.setMinimum( 0 );
    slider.setMaximum( 1000 );
    sourceSpan = slider.maximum() - slider.minimum();
    q->setSizePolicy( slider.sizePolicy() );
    q->connect( &slider, SIGNAL( valueChanged(int) ), SLOT( on_valueChanged(int) ) );
}

void DoubleSlider::Private::on_valueChanged( int val )
{
    V_Q( DoubleSlider );
    emit q->valueChanged( q->value() );
}

DoubleSlider::DoubleSlider( QWidget *parent ) :
    AbstractDoubleSlider( parent ),
    d_ptr( new DoubleSlider::Private( this ))
{
}

Qt::Orientation DoubleSlider::orientation() const
{
    return d_func()->slider.orientation();
}

void DoubleSlider::setOrientation( Qt::Orientation o )
{
    V_D( DoubleSlider );
    d->slider.setOrientation( o );
    setSizePolicy( d->slider.sizePolicy() );
}

double DoubleSlider::minimum() const
{
    return d_func()->minimum;
}

double DoubleSlider::maximum() const
{
    return d_func()->maximum;
}

void DoubleSlider::setMinimum( double min )
{
    setRange( min, maximum() );
}

void DoubleSlider::setMaximum( double max )
{
    setRange( minimum(), max );
}

void DoubleSlider::setRange( double min, double max )
{
    V_D( DoubleSlider );
    if ( min == d->minimum && max == d->maximum ) return;
    double val = value();
    d->minimum = qMin( min, max );
    d->maximum = qMax( min, max );
    d->span = qMax( d->maximum - d->minimum, 1.0 );
    setValue( val );
}

double DoubleSlider::value() const
{
    V_D( const DoubleSlider );
    return d->minimum + d->span * d->slider.value() / d->sourceSpan;
}

void DoubleSlider::setValue( double val )
{
    V_D( DoubleSlider );
    val = qBound( d->minimum, val, d->maximum );
    int sourceVal = qRound( (val - d->minimum) * d->sourceSpan / d->span );
    if ( sourceVal != d->slider.value() ) {
        d->slider.setValue( sourceVal );
    }
}

QSize DoubleSlider::minimumSizeHint() const
{
    return d_func()->slider.minimumSizeHint();
}

QSize DoubleSlider::sizeHint() const
{
    return d_func()->slider.sizeHint();
}

void DoubleSlider::resizeEvent( QResizeEvent * ev )
{
    d_func()->slider.resize( ev->size() );
}

DoubleSlider::~DoubleSlider() {}

#include "moc_DoubleSlider.cpp"
