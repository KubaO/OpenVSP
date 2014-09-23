//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "SubGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "Camera.h"
#include "Renderable.h"
#include "Common.h"
#include <vector>
#include <cassert>

using namespace VSPGraphic;

namespace VSPGUI
{

class VspSubGlWindow::Private {
    VSP_DECLARE_PUBLIC( VspSubGlWindow )
    VspSubGlWindow * const q_ptr;
    VSPGraphic::GraphicEngine gEngine;
    unsigned int id;
    DrawObj::ScreenEnum linkedScreen;
    bool initialized;

    Private( VspSubGlWindow *, DrawObj::ScreenEnum );
    Scene * scene() {  return gEngine.getScene(); }
    virtual void initGLEW();
    virtual void update( std::vector<DrawObj *> objects );

    void loadPointData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void loadLineData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
};
VSP_DEFINE_PRIVATE( VspSubGlWindow )

VspSubGlWindow::Private::Private( VspSubGlWindow * q, DrawObj::ScreenEnum screen ) :
    q_ptr( q ),
    id( 0xFFFFFFFF ),
    linkedScreen( screen ),
    initialized( false )
{
    QGLFormat fmt( QGL::Rgba | QGL::AlphaChannel | QGL::DepthBuffer | QGL::DoubleBuffer | QGL::SampleBuffers );
    q->setFormat( fmt );

    gEngine.getDisplay()->setDisplayLayout( 1, 1 );
    gEngine.getDisplay()->selectViewport( 0 );
    gEngine.getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_FRONT );

    Viewport * viewport = gEngine.getDisplay()->getViewport();
    assert( viewport );
    // Disable border and arrows.
    viewport->showBorders( false );
    viewport->showXYZArrows( false );

    // Enable grid.
    viewport->showGridOverlay( true );
}

VspSubGlWindow::VspSubGlWindow( DrawObj::ScreenEnum drawObjScreen, QWidget * parent ) :
    QGLWidget( parent ),
    d_ptr( new VspSubGlWindow::Private( this, drawObjScreen ) )
{
    setWindowTitle( "VSP Sub GL Window" );
}

VSPGraphic::GraphicEngine * VspSubGlWindow::getGraphicEngine()
{
    return &d_func()->gEngine;
}

QSize VspSubGlWindow::minimumSizeHint() const
{
    return QSize(10, 10);
}

void VspSubGlWindow::initializeGL()
{
    d_func()->initGLEW();
}

void VspSubGlWindow::paintGL()
{
    d_func()->gEngine.draw();
}

void VspSubGlWindow::resizeGL( int w, int h )
{
    d_func()->gEngine.getDisplay()->resize( w, h );
}

void VspSubGlWindow::update()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();
    if ( vPtr && isValid() )
    {
        d_func()->update( vPtr->GetDrawObjs() );
    }
    QGLWidget::update();
}

void VspSubGlWindow::Private::update( std::vector<DrawObj *> objects )
{
    // Remove all Scene Objects.
    std::vector<unsigned int> currIds;
    currIds = scene()->getIds();
    for( int i = 0; i < ( int )currIds.size(); i++ )
    {
        scene()->removeObject( currIds[i] );
    }

    // Update Scene Objects.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != linkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue, size;

        Renderable * rObj;
        unsigned int id;
        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_POINTS:
            scene()->createObject( Common::VSP_OBJECT_MARKER, &id );

            // Update scene object.
            red = ( float )objects[i]->m_PointColor.x();
            green = ( float )objects[i]->m_PointColor.y();
            blue = ( float )objects[i]->m_PointColor.z();

            size = ( float )objects[i]->m_PointSize;

            rObj = dynamic_cast<Renderable*>( scene()->getObject(id) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( red, green, blue );
                rObj->setPointSize( size );

                loadPointData( rObj, objects[i] );
            }
            break;

        case DrawObj::VSP_LINES:
            scene()->createObject( Common::VSP_OBJECT_MARKER, &id );

            // Update scene object.
            red = ( float )objects[i]->m_LineColor.x();
            green = ( float )objects[i]->m_LineColor.y();
            blue = ( float )objects[i]->m_LineColor.z();

            size = ( float )objects[i]->m_LineWidth;

            rObj = dynamic_cast<Renderable*>( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( size );

                loadLineData( rObj, objects[i] );
            }
            break;

        default:
            break;
        }
    }
}

void VspSubGlWindow::setZoomValue( float value )
{
    Camera * camera = d_func()->gEngine.getDisplay()->getCamera();
    if( camera )
    {
        camera->setZoomValue( value );
    }
}

void VspSubGlWindow::Private::initGLEW()
{
    if( !initialized )
    {
        VSPGraphic::GraphicEngine::initGlew();
        initialized = true;
    }
}

void VspSubGlWindow::Private::loadPointData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}

void VspSubGlWindow::Private::loadLineData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 1; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}

VspSubGlWindow::~VspSubGlWindow() {}

}
