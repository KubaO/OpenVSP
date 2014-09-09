//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef VALUESLIDER_H
#define VALUESLIDER_H

#include "AbstractDoubleSlider.h"
#include <QAbstractSpinBox>

class ValueSliderPrivate;
class ValueSlider : public AbstractDoubleSlider
{
    Q_OBJECT
    Q_DECLARE_PRIVATE( ValueSlider )
    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( int decimals READ decimals WRITE setDecimals )
    Q_PROPERTY( QAbstractSpinBox::ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols )
    Q_PROPERTY( QColor colorization READ colorization WRITE setColorization )
    Q_PRIVATE_SLOT( d_func(), void on_qt_slider_valueChanged( double ) )
    Q_PRIVATE_SLOT( d_func(), void on_qt_spinbox_valueChanged( double ) )
    QScopedPointer<ValueSliderPrivate> const d_ptr;
public:
    explicit ValueSlider(QWidget *parent = 0);
    ~ValueSlider();

    Qt::Orientation orientation() const;
    Q_SLOT void setOrientation( Qt::Orientation );
    double minimum() const Q_DECL_OVERRIDE;
    double maximum() const Q_DECL_OVERRIDE;
    void setMinimum( double ) Q_DECL_OVERRIDE;
    void setMaximum( double ) Q_DECL_OVERRIDE;
    void setRange( double, double ) Q_DECL_OVERRIDE;
    int decimals() const;
    void setDecimals( int );
    QAbstractSpinBox::ButtonSymbols buttonSymbols() const;
    void setButtonSymbols( QAbstractSpinBox::ButtonSymbols );
    QColor colorization() const;
    void setColorization( const QColor & );
    double value() const Q_DECL_OVERRIDE;
    void setValue( double ) Q_DECL_OVERRIDE;
};

#endif // VALUESLIDER_H
