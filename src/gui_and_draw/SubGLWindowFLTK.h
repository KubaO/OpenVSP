#ifndef _VSP_GUI_2D_GL_WINDOW_H
#define _VSP_GUI_2D_GL_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include "DrawObj.h"

namespace VSPGraphic
{
class GraphicEngine;
class Renderable;
}

class FuselageGeom;

namespace VSPGUI
{
/*
* This class provides all functionalites of a 2D opengl window.
*/
class VspSubGlWindowFLTK : public Fl_Gl_Window
{
public:
    /*
    * Constructor.
    * Parmeters are left upper corner position of the window (x, y) and
    * window's width and height (w, h).
    *
    * drawObjScreen - specific drawObj screen to link.
    */
    VspSubGlWindowFLTK( int x, int y, int w, int h, DrawObj::ScreenEnum drawObjScreen );
    /*
    * Destructor.
    */
    virtual ~VspSubGlWindowFLTK();

public:
    /*
    * Initialize window on first call.
    */
    virtual void show();
    /*
    * Draw 2D Scene.
    */
    virtual void draw();

public:
    /*
    * Update window.
    */
    virtual void update();

    /*
    * Set Zoom value.
    */
    virtual void setZoomValue( float value );

public:
    /*!
    * Get graphic engine of this window.
    */
    VSPGraphic::GraphicEngine * getGraphicEngine()
    {
        return m_GEngine;
    }

protected:
    /*
    * Initialize GLEW.
    */
    virtual void _initGLEW();
    /*
    * Update Scene.
    */
    virtual void _update( std::vector<DrawObj *> objects );

private:
    void _loadPointData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void _loadLineData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );

protected:
    /*
    * VSP Graphic Engine.
    */
    VSPGraphic::GraphicEngine * m_GEngine;
    unsigned int m_id;

private:
    DrawObj::ScreenEnum m_LinkedScreen;
    bool m_Initialized;
};
}
#endif
