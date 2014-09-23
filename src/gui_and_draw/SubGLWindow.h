//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef _VSP_GUI_2D_GL_WINDOW_H
#define _VSP_GUI_2D_GL_WINDOW_H

#include "DrawObj.h"
#include "VspObj.h"
#include <QGLWidget>

namespace VSPGraphic
{
class GraphicEngine;
}

namespace VSPGUI
{

/// This class provides all functionalites of a 2D opengl window.
class VspSubGlWindow : public QGLWidget
{
    Q_OBJECT
    VSP_DECLARE_PRIVATE( VspSubGlWindow )
    QScopedPointer< Private > const d_ptr;
public:
    /*! \param drawObjScreen specific drawObj screen to link. */
    VspSubGlWindow( DrawObj::ScreenEnum drawObjScreen, QWidget * parent = 0 );
    ~VspSubGlWindow();

    virtual void update();
    virtual void setZoomValue( float value );

    VSPGraphic::GraphicEngine * getGraphicEngine();
protected:
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL( int w, int h ) Q_DECL_OVERRIDE;
};

}
#endif
