#ifndef ABSTRACTDOUBLESLIDER_H
#define ABSTRACTDOUBLESLIDER_H

#include <QWidget>

class AbstractDoubleSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double value READ value WRITE setValue NOTIFY valueChanged USER true )
public:
    explicit AbstractDoubleSlider(QWidget *parent = 0);

    virtual double minimum() const = 0;
    virtual double maximum() const = 0;
    virtual void setMinimum( double ) = 0;
    virtual void setMaximum( double ) = 0;
    Q_SLOT virtual void setRange( double, double ) = 0;
    virtual double value() const = 0;
    Q_SLOT void virtual setValue( double ) = 0;
    Q_SIGNAL void valueChanged( double );
};


#endif // ABSTRACTDOUBLESLIDER_H
