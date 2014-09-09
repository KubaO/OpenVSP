//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef DOUBLESLIDER_H
#define DOUBLESLIDER_H

#include "AbstractDoubleSlider.h"

class DoubleSliderPrivate;
class DoubleSlider : public AbstractDoubleSlider
{
    Q_OBJECT
    Q_DECLARE_PRIVATE( DoubleSlider )
    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_PRIVATE_SLOT( d_func(), void on_valueChanged( int ) )
    QScopedPointer<DoubleSliderPrivate> const d_ptr;
public:
    explicit DoubleSlider(QWidget *parent = 0);
    ~DoubleSlider();

    Qt::Orientation orientation() const;
    Q_SLOT void setOrientation( Qt::Orientation );
    double minimum() const Q_DECL_OVERRIDE;
    double maximum() const Q_DECL_OVERRIDE;
    void setMinimum( double ) Q_DECL_OVERRIDE;
    void setMaximum( double ) Q_DECL_OVERRIDE;
    void setRange( double, double ) Q_DECL_OVERRIDE;
    double value() const Q_DECL_OVERRIDE;
    void setValue( double ) Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
protected:
    void resizeEvent( QResizeEvent * ) Q_DECL_OVERRIDE;
};

#endif // DOUBLESLIDER_H
