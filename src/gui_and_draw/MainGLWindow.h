//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_GL_WINDOW_H
#define VSP_GUI_GL_WINDOW_H

#include "Common.h"

#include "DrawObj.h"
#include "VspObj.h"
#include <QGLWidget>

QT_BEGIN_NAMESPACE
class QPoint;
QT_END_NAMESPACE

namespace VSPGraphic
{
class GraphicEngine;
class Renderable;
class Selectable;
}

class ScreenMgr;

namespace VSPGUI
{
class VspGlWindow : public QGLWidget
{
    Q_OBJECT
    VSP_DECLARE_PRIVATE( VspGlWindow )
    QScopedPointer< Private > const d_ptr;
public:
    /*! \param drawObjScreen - specific drawObj screen to link. */
    VspGlWindow( ScreenMgr * mgr, DrawObj::ScreenEnum drawObjScreen, QWidget * parent = 0 );
    ~VspGlWindow();

    /*!
    * Set Window Layout.
    * Row - Number of Rows.
    * Column - Number of Columns.
    */
    virtual void setWindowLayout( int row, int column );
    /*!
    * Set view for current selected viewport.
    * Acceptable type are VSP_CAM_TOP, VSP_CAM_FRONT, VSP_CAM_LEFT, VSP_CAM_LEFT_ISO,
    * VSP_CAM_BOTTOM, VSP_CAM_REAR, VSP_CAM_RIGHT, VSP_CAM_RIGHT_ISO, VSP_CAM_CENTER.
    */
    virtual void setView( VSPGraphic::Common::VSPenum type );

    /*!
    * Get graphic engine of this window.
    */
    VSPGraphic::GraphicEngine * getGraphicEngine();

    /*!
    * Pan current selected camera.
    *
    * dx - Direction on x-axis.  Positive moves right, negtive moves left.
    * dy - Direction on y-axis.  Positive moves up, negtive moves down.
    * precisionOn - pan camera slowly.
    */
    virtual void pan( int dx, int dy, bool precisionOn = false );

    /*!
    * Zoom current selected camera.
    *
    * delta - Zoom in or out.  Positive zooms out, negtive zooms in.
    * precisionOn - If true, zoom slowly.
    */
    virtual void zoom( int delta, bool precisionOn = false );

    virtual void update();

protected:
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL( int w, int h ) Q_DECL_OVERRIDE;

    void mousePressEvent( QMouseEvent * ) Q_DECL_OVERRIDE;
    void mouseMoveEvent( QMouseEvent * ) Q_DECL_OVERRIDE;
    void mouseReleaseEvent( QMouseEvent * ) Q_DECL_OVERRIDE;
    void keyPressEvent( QKeyEvent * ) Q_DECL_OVERRIDE;
    void keyReleaseEvent( QKeyEvent * ) Q_DECL_OVERRIDE;
};
}
#endif
