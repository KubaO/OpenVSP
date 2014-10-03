#include "MainGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "ScreenMgr.h"
#include "CfdMeshScreen.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "LayoutMgr.h"
#include "CameraMgr.h"
#include "Camera.h"
#include "SceneObject.h"
#include "Renderable.h"
#include "Pickable.h"
#include "PickablePnts.h"
#include "Lighting.h"
#include "Image.h"
#include "Texture2D.h"
#include "TextureMgr.h"
#include "Entity.h"
#include "XSecEntity.h"
#include "Ruler.h"
#include "GraphicEngine.h"
#include "ScreenMgr.h"
#include "ManageLabelScreen.h"
#include "ManageLightingScreen.h"
#include "ManageGeomScreen.h"
#include "ManageCORScreen.h"
#include "Common.h"
#include "GraphicSingletons.h"
#include "Selectable.h"
#include "SelectedPnt.h"
#include "SelectedLoc.h"
#include "Material.h"
#include "ClippingScreen.h"
#include "Clipping.h"
#include "BndBox.h"
#include "Vec3d.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>

#include <QWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGuiApplication>

#define PRECISION_PAN_SPEED 0.005f
#define PAN_SPEED 0.025f
#define PRECISION_ZOOM_SPEED 0.00005f
#define ZOOM_SPEED 0.00025f

static const unsigned int kNullId = 0xFFFFFFFFu;
static const glm::vec3 kNullVec3(0xFFFFFFFF);

#pragma warning(disable:4244)

using namespace VSPGraphic;

namespace VSPGUI
{

class VspGlWindow::Private
{
    VSP_DECLARE_PUBLIC( VspGlWindow )

    Private( ScreenMgr * mgr, DrawObj::ScreenEnum drawObj, VspGlWindow * q );

    void initGLEW();

    void updateTextures( DrawObj * drawObj );

    void loadTrisData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void loadXSecData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );
    void loadMarkData( VSPGraphic::Renderable * destObj, DrawObj * drawObj );

    void update( std::vector<DrawObj *> objects );

    void setLighting( DrawObj * drawObj );

    void setClipping( DrawObj * drawObj );

    struct ID;

    ID & findID( std::string geomID );
    ID & findID( unsigned int bufferID );

    void updateBuffer( std::vector<DrawObj *> objects );

    void sendFeedback( VSPGraphic::Selectable * selected );
    void sendFeedback( std::vector<VSPGraphic::Selectable *> listOfSelected );

    std::vector<std::vector<vec3d>> generateTexCoordFromXSec( DrawObj * drawObj );
    double distance( vec3d pointA, vec3d pointB );

    VSPGraphic::Viewport * viewport() { return display()->getViewport(); }
    VSPGraphic::Display * display() { return gEngine.getDisplay(); }
    VSPGraphic::Scene * scene() { return gEngine.getScene(); }
    VspScreen * getScreen( int id ) { return screenMgr->GetScreen( id ); }
    void updateMouse();

    VspGlWindow * const q_ptr;
    ScreenMgr * screenMgr;
    DrawObj::ScreenEnum linkedScreen;
    VSPGraphic::GraphicEngine gEngine;
    bool initialized;

    struct TextureID
    {
        unsigned int bufferTexID;
        std::string geomTexID;
    };
    struct ID
    {
        unsigned int bufferID;
        std::string geomID;
        std::vector<TextureID> textureIDs;

        ID() : bufferID( kNullId ) {}
        ID( unsigned int bID, const std::string gID ) : bufferID( bID ), geomID( gID ) {}
        operator bool() const { return bufferID != kNullId; }

        TextureID * find( std::string geomTexID )
        {
            for( int i = 0; i < ( int )textureIDs.size(); i++ )
            {
                if( textureIDs[i].geomTexID == geomTexID )
                {
                    return &textureIDs[i];
                }
            }
            return NULL;
        }
    };
    std::vector<ID> ids;

    float lightAmb;
    float lightDiff;
    float lightSpec;

    QPoint mouse; ///< Last mouse event cooordinate with origin in the lower left corner.
    QPoint prevLB;
    QPoint prevMB;
    QPoint prevRB;
    QPoint prevAltLB;
    QPoint prevCtrlLB;
    QPoint prevMetaLB;
    QPoint prevLBRB;
};
VSP_DEFINE_PRIVATE( VspGlWindow )

VspGlWindow::Private::Private( ScreenMgr * mgr, DrawObj::ScreenEnum drawObjScreen, VspGlWindow * q ) :
    q_ptr( q ),
    screenMgr( mgr ),
    linkedScreen( drawObjScreen ), // Link this GUI to one of drawObj screens.
    initialized( false ),
    lightAmb( 0.5f ),
    lightSpec( 0.5f ),
    lightDiff( 0.5f )
{
    QGLFormat fmt( QGL::Rgba | QGL::AlphaChannel | QGL::DepthBuffer | QGL::DoubleBuffer | QGL::SampleBuffers );
    q->setFormat( fmt );
    q->setCursor( Qt::CrossCursor );
    q->setMouseTracking( true );

    // One Screen
    display()->setDisplayLayout( 1, 1 );
    display()->selectViewport( 0 );
}

VspGlWindow::VspGlWindow( ScreenMgr * mgr, DrawObj::ScreenEnum drawObjScreen, QWidget *parent ) :
    QGLWidget( parent ),
    d_ptr( new Private( mgr, drawObjScreen, this ) )
{}

VSPGraphic::GraphicEngine * VspGlWindow::getGraphicEngine()
{
    return &d_func()->gEngine;
}

void VspGlWindow::Private::updateMouse()
{
    V_Q( VspGlWindow );
    auto screen = q->window()->windowHandle()->screen();
    auto pos = q->mapFromGlobal( QCursor::pos( screen ) );
    pos.setY( q->height() - pos.y() );
    mouse = pos;
}

QSize VspGlWindow::minimumSizeHint() const
{
    return QSize( 200, 200 );
}

void VspGlWindow::setWindowLayout( int row, int column )
{
    V_D( VspGlWindow );
    d->display()->setDisplayLayout( row, column );
    d->display()->resize( width(), height() );
    d->display()->selectViewport( 0 );
    QGLWidget::update();
}

void VspGlWindow::setView( VSPGraphic::Common::VSPenum type )
{
    d_func()->display()->changeView( type );
    QGLWidget::update();
}

void VspGlWindow::pan( int dx, int dy, bool precisionOn )
{
    auto speed = precisionOn ? PRECISION_PAN_SPEED : PAN_SPEED;
    float x = 0.0;
    float y = 0.0;

    if( dx != 0 )
    {
        x = speed * ( dx < 0 ? -1 : 1 );
    }
    if( dy != 0 )
    {
        y = speed * ( dy < 0 ? -1 : 1 );
    }
    d_func()->display()->pan( x, y );
}

void VspGlWindow::zoom( int delta, bool precisionOn )
{
    auto speed = precisionOn ? PRECISION_ZOOM_SPEED : ZOOM_SPEED;
    float zoomvalue = 0;

    if( delta != 0 )
    {
        zoomvalue = speed * ( delta < 0 ? -1 : 1 );
    }
    d_func()->display()->zoom( zoomvalue );
}

void VspGlWindow::initializeGL()
{
    d_func()->initGLEW();
}

void VspGlWindow::paintGL()
{
    V_D( VspGlWindow );
    d->gEngine.draw( d->mouse.x(), d->mouse.y() );
}

void VspGlWindow::resizeGL( int w, int h )
{
    d_func()->display()->resize( w, h );
}

void VspGlWindow::update()
{
    V_D( VspGlWindow );
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        makeCurrent();

        // Get Render Objects from Vehicle.
        vector<DrawObj *> drawObjs = vPtr->GetDrawObjs();

        // Load Render Objects from CfdMeshScreen.
        CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
            ( d->getScreen( ScreenMgr::VSP_CFD_MESH_SCREEN ) );
        if( cfdScreen )
        {
            cfdScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from labelScreen.
        ManageLabelScreen * labelScreen = dynamic_cast< ManageLabelScreen* >
            ( d->getScreen( ScreenMgr::VSP_LABEL_SCREEN ) );
        if( labelScreen )
        {
            labelScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from lightScreen.
        ManageLightingScreen * lightScreen = dynamic_cast< ManageLightingScreen* >
            ( d->getScreen( ScreenMgr::VSP_LIGHTING_SCREEN ) );
        if( lightScreen )
        {
            lightScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from geomScreen.
        ManageGeomScreen * geomScreen = dynamic_cast< ManageGeomScreen* >
            ( d->getScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from corScreen ( Center of Rotation ).
        ManageCORScreen * corScreen = dynamic_cast< ManageCORScreen* >
            ( d->getScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->LoadDrawObjs( drawObjs );
        }

        // Load Render Objects from clipScreen ( Clipping ).
        ClippingScreen * clipScreen = dynamic_cast< ClippingScreen* >
            ( d->getScreen( ScreenMgr::VSP_CLIPPING_SCREEN ) );
        if( clipScreen )
        {
            clipScreen->LoadDrawObjs( drawObjs );
        }

        // Load Objects to Renderer.
        d->update( drawObjs );

        // Once updated data is stored in buffer, 
        // reset geometry changed flag to false.
        vPtr->ResetDrawObjsGeomChangedFlags();
    }
    QGLWidget::update();
}

void VspGlWindow::Private::initGLEW()
{
    if( !initialized )
    {
        q_func()->makeCurrent();
        VSPGraphic::GraphicEngine::initGlew();
        initialized = true;
    }
}

void VspGlWindow::Private::update( std::vector<DrawObj *> objects )
{
    // Check for changes in DrawObjs and adjust accordingly.
    updateBuffer( objects );

    // Process all geometry renderable first, then all labels, pickables, lastly the markers.  Order matters.
    for( int i = 0; i < (int)objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != linkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue;
        float lineWidth;

        red = ( float )objects[i]->m_LineColor.x();
        green = ( float )objects[i]->m_LineColor.y();
        blue = ( float )objects[i]->m_LineColor.z();

        lineWidth = ( float )objects[i]->m_LineWidth;

        unsigned int id = findID( objects[i]->m_GeomID ).bufferID;

        Renderable * rObj;
        VSPGraphic::Entity * eObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_SETTING:
            setLighting( objects[i] );
            break;

        case DrawObj::VSP_CLIP:
            setClipping( objects[i] );
            break;

        case DrawObj::VSP_POINTS:
            // Create new scene object if needed.
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }

            // Update scene object.
            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( objects[i]->m_PointColor.x(),
                    objects[i]->m_PointColor.y(), objects[i]->m_PointColor.z() );
                rObj->setPointSize( objects[i]->m_PointSize );

                // Update buffer data if needed.
                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINES:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }

            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_LOOP:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_LOOP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_STRIP:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_STRIP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_MESH:
            // Create new scene object if needed.
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                // Update scene object.
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_WIRE_TRIS:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_MESH:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_TRIS:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_HIDDEN_TRIS_CFD:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_CFD_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_WIRE_FRAME_SOLID );
                eObj->setLineColor( red, green, blue );
                eObj->setLineWidth( lineWidth );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_MESH:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadXSecData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_SHADED_TRIS:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_TRIANGLES );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_SHADED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadTrisData( eObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_TEXTURED_MESH:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_XSEC_ENTITY, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            eObj = dynamic_cast<VSPGraphic::Entity*> ( scene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_MESH_TEXTURED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                    objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                    objects[i]->m_MaterialInfo.Shininess );

                if( objects[i]->m_GeomChanged )
                {
                    loadXSecData( eObj, objects[i] );
                }
                updateTextures( objects[i] );
            }
            break;

        default:
            break;
        }
    }

    // Process Labels.  Order Matters.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != linkedScreen )
        {
            continue;
        }

        unsigned int id = findID( objects[i]->m_GeomID ).bufferID;

        VSPGraphic::Ruler * ruler;

        glm::vec3 start, end, offset;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_RULER:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_RULER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            ruler = dynamic_cast<VSPGraphic::Ruler*> ( scene()->getObject( id ) );
            if( ruler )
            {
                ruler->setTextColor( (float)objects[i]->m_TextColor.x(),
                    (float)objects[i]->m_TextColor.y(),
                    (float)objects[i]->m_TextColor.z() );

                ruler->setTextSize( (float)objects[i]->m_TextSize );

                switch( objects[i]->m_Ruler.Step )
                {
                case DrawObj::VSP_RULER_STEP_ZERO:
                    ruler->reset();
                    break;

                case DrawObj::VSP_RULER_STEP_ONE:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    ruler->placeRuler( start );
                    break;

                case DrawObj::VSP_RULER_STEP_TWO:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    end = glm::vec3( objects[i]->m_Ruler.End.x(),
                        objects[i]->m_Ruler.End.y(),
                        objects[i]->m_Ruler.End.z() );
                    ruler->placeRuler( start, end );
                    break;

                case DrawObj::VSP_RULER_STEP_COMPLETE:
                    start = glm::vec3( objects[i]->m_Ruler.Start.x(),
                        objects[i]->m_Ruler.Start.y(),
                        objects[i]->m_Ruler.Start.z() );
                    end = glm::vec3( objects[i]->m_Ruler.End.x(),
                        objects[i]->m_Ruler.End.y(),
                        objects[i]->m_Ruler.End.z() );
                    offset = glm::vec3( objects[i]->m_Ruler.Offset.x(),
                        objects[i]->m_Ruler.Offset.y(),
                        objects[i]->m_Ruler.Offset.z() );
                    ruler->placeRuler( start, end, offset );
                    break;

                default:
                    break;
                }
            }
            break;
 
        default:
            break;
       }
    }

    // Now process all pickables.  Order matters.

    // Reset flag to show all selections.  This prevents selections from permanently hidden
    // due to misuse of VSP_PICK_VERTEX_HIDE_SELECTION object.
    scene()->showSelection();

    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != linkedScreen )
        {
            continue;
        }

        unsigned int id = findID( objects[i]->m_GeomID ).bufferID;

        Pickable * pObj;
        PickablePnts * ppntObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_PICK_VERTEX_HIDE_SELECTION:
            scene()->hideSelection();
            break;

        case DrawObj::VSP_PICK_GEOM:
            if( id == kNullId )
            {
                ID & sourceId = findID( objects[i]->m_PickSourceID );
                if( sourceId )
                {
                    scene()->createObject( Common::VSP_OBJECT_PICK_GEOM, &id, sourceId.bufferID );
                    ids.push_back( ID( id, objects[i]->m_GeomID ) );
                }
            }
            pObj = dynamic_cast<Pickable*> ( scene()->getObject( id ) );
            if( pObj )
            {
                pObj->setVisibility( objects[i]->m_Visible );
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
            }
            break;

        case DrawObj::VSP_PICK_VERTEX:
            if( id == kNullId )
            {
                ID & sourceId = findID( objects[i]->m_PickSourceID );
                if( sourceId )
                {
                    scene()->createObject( Common::VSP_OBJECT_PICK_VERTEX, &id, sourceId.bufferID );
                    ids.push_back( ID( id, objects[i]->m_GeomID ) );
                }
            }
            ppntObj = dynamic_cast<PickablePnts*> ( scene()->getObject( id ) );
            if( ppntObj )
            {
                ppntObj->setVisibility( objects[i]->m_Visible );
                ppntObj->setGroup( objects[i]->m_FeedbackGroup );
                ppntObj->setPointSize( objects[i]->m_PointSize );
                ppntObj->update();
            }
            break;

        case DrawObj::VSP_PICK_VERTEX_SELECT_ALL:
            if( id == kNullId )
            {
                ID & sourceId = findID( objects[i]->m_PickSourceID );
                if( sourceId )
                {
                    scene()->createObject( Common::VSP_OBJECT_PICK_VERTEX, &id, sourceId.bufferID );
                    ids.push_back( ID( id, objects[i]->m_GeomID ) );
                }
            }
            ppntObj = dynamic_cast<PickablePnts*> ( scene()->getObject( id ) );
            if( ppntObj )
            {
                ppntObj->setVisibility( objects[i]->m_Visible );
                ppntObj->setGroup( objects[i]->m_FeedbackGroup );
                ppntObj->setPointSize( objects[i]->m_PointSize );
                ppntObj->update();

                // Select all points from Pickable.
                scene()->selectAll( ppntObj );

                // Send feedback back to GUI.
                std::string feedbackGroupName = scene()->getLastSelected()->getGroup();
                sendFeedback( scene()->getSelected( feedbackGroupName ) );
            }
            break;

        case DrawObj::VSP_PICK_VERTEX_UNSELECT_ALL:
            scene()->unselectAll();
            break;

        case DrawObj::VSP_PICK_LOCATION:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_PICK_LOCATION, &id, 0 );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            pObj = dynamic_cast<Pickable*> ( scene()->getObject( id ) );
            if( pObj )
            {
                pObj->setVisibility( objects[i]->m_Visible );
                pObj->setGroup( objects[i]->m_FeedbackGroup );
                pObj->update();
            }
            break;

        default:
            break;
        }
    }

    // Now process all markers.  Order matters.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != linkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue;
        float lineWidth;

        red = ( float )objects[i]->m_LineColor.x();
        green = ( float )objects[i]->m_LineColor.y();
        blue = ( float )objects[i]->m_LineColor.z();

        lineWidth = ( float )objects[i]->m_LineWidth;

        unsigned int id = findID( objects[i]->m_GeomID ).bufferID;

        Renderable * rObj;

        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_POINTS:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }

            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( objects[i]->m_PointColor.x(),
                    objects[i]->m_PointColor.y(), objects[i]->m_PointColor.z() );
                rObj->setPointSize( objects[i]->m_PointSize );

                // Update buffer data if needed.
                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINES:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }

            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_LOOP:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        case DrawObj::VSP_LINE_STRIP:
            if( id == kNullId )
            {
                scene()->createObject( Common::VSP_OBJECT_MARKER, &id );
                ids.push_back( ID( id, objects[i]->m_GeomID ) );
            }
            rObj = dynamic_cast<Renderable*> ( scene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINE_STRIP );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( lineWidth );

                if( objects[i]->m_GeomChanged )
                {
                    loadMarkData( rObj, objects[i] );
                }
            }
            break;

        default:
            break;
        }
    }
}

VspGlWindow::Private::ID & VspGlWindow::Private::findID( std::string geomID )
{
    static const ID kNullID;
    for( int i = 0; i < ( int )ids.size(); i++ )
    {
        if( ids[i].geomID == geomID )
        {
            return ids[i];
        }
    }
    return const_cast<ID&>(kNullID);
}

VspGlWindow::Private::ID & VspGlWindow::Private::findID( unsigned int bufferID )
{
    static const ID kNullID;
    for( int i = 0; i < ( int )ids.size(); i++ )
    {
        if( ids[i].bufferID == bufferID )
        {
            return ids[i];
        }
    }
    return const_cast<ID&>(kNullID);
}

void VspGlWindow::Private::updateBuffer( std::vector<DrawObj *> objects )
{
    std::vector<ID> idsToRemove;
    std::vector<ID> idsToKeep;

    // Figure out if any buffer object no longer exists in DrawObjs.
    for( int i = 0; i < ( int )ids.size(); i++ )
    {
        bool exist = false;
        for( int j = 0; j < ( int )objects.size(); j++ )
        {
            if( ids[i].geomID == objects[j]->m_GeomID && ids[i].geomID != std::string( "Default" ) )
            {
                idsToKeep.push_back( ids[i] );
                exist = true;
                break;
            }
        }
        if( !exist )
        {
            idsToRemove.push_back( ids[i] );
        }
    }

    // Remove unnecessary buffers.
    for( int i = 0; i < ( int )idsToRemove.size(); i++ )
    {
        scene()->removeObject( idsToRemove[i].bufferID );
    }

    // Update m_ids.
    ids.clear();
    for( int i = 0; i < ( int )idsToKeep.size(); i++ )
    {
        ids.push_back( idsToKeep[i] );
    }
}

void VspGlWindow::Private::updateTextures( DrawObj * drawObj )
{
    ID & id = findID( drawObj->m_GeomID );
    if( !id )
    {
        // Can't find Object that is linked to this drawObj
        assert( false );
        return;
    }

    Renderable * renderable = dynamic_cast<Renderable*>( scene()->getObject( id.bufferID ) );
    if( renderable )
    {
        VSPGraphic::Entity * entity = dynamic_cast<VSPGraphic::Entity *>( renderable );
        if( entity )
        {
            // Check if any textureID no long exists in drawObj.
            // Remove Deleted Textures.

            std::vector<TextureID> texToRemove;
            std::vector<TextureID> texToKeep;

            for( int i = 0; i < ( int )id.textureIDs.size(); i++ )
            {
                bool exist = false;
                for( int j = 0; j < ( int )drawObj->m_TextureInfos.size(); j++ )
                {
                    if( id.textureIDs[i].geomTexID == drawObj->m_TextureInfos[j].ID )
                    {
                        texToKeep.push_back( id.textureIDs[i] );
                        exist = true;
                        break;
                    }
                }
                if( !exist )
                {
                    texToRemove.push_back( id.textureIDs[i] );
                }
            }

            for(int i = 0; i < ( int )texToRemove.size(); i++)
            {
                entity->getTextureMgr()->remove( texToRemove[i].bufferTexID );
            }

            id.textureIDs.clear();
            for( int i = 0; i < ( int )texToKeep.size(); i++ )
            {
                id.textureIDs.push_back( texToKeep[i] );
            }

            // Update / add textures.

            std::vector<DrawObj::TextureInfo> drawObjTexList = drawObj->m_TextureInfos;
            for( int i = 0; i < ( int )drawObjTexList.size(); i++ )
            {
                unsigned int texBufferID;

                TextureID * texID = id.find( drawObjTexList[i].ID );
                if( texID )
                {
                    texBufferID = texID->bufferTexID;
                }
                else
                {
                    // New texture.
                    TextureID newTex;
                    newTex.geomTexID = drawObjTexList[i].ID;
                    newTex.bufferTexID = entity->getTextureMgr()
                        ->add( VSPGraphic::GlobalTextureRepo()->get2DTexture(drawObjTexList[i].FileName.c_str() ) );
                    id.textureIDs.push_back( newTex );

                    texBufferID = newTex.bufferTexID;
                }

                float UScale = ( float )( 1 / drawObjTexList[i].UScale );
                float WScale = ( float )( 1 / drawObjTexList[i].WScale );
                float U = ( float )( drawObjTexList[i].U * -1.f * UScale );
                float W = ( float )( drawObjTexList[i].W * WScale );

                // Update Texture Properities.
                entity->getTextureMgr()->translate( texBufferID, U, W );
                entity->getTextureMgr()->scale( texBufferID, UScale, WScale );
                entity->getTextureMgr()->transparentize( texBufferID, drawObjTexList[i].Transparency );
                entity->getTextureMgr()->flipCoord( texBufferID, drawObjTexList[i].UFlip, drawObjTexList[i].WFlip );
            }
        }
        else
        {
            // Render Object is not type of Textureable Entity.
            assert( false );
        }
    }
    else
    {
        // Can't find Render Object.
        assert( false );
    }
}

void VspGlWindow::Private::loadXSecData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;
    std::vector<unsigned int> edata;

    // Generate Texture Coordinate.
    std::vector<std::vector<vec3d>> textureCoords = generateTexCoordFromXSec( drawObj );

    int num_pnts = drawObj->m_PntMesh.size();
    int num_xsecs = 0;
    if ( num_pnts )
        num_xsecs = drawObj->m_PntMesh[0].size();

    // Vertex Buffer.
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        for ( int j = 0 ; j < num_xsecs ; j++ )
        {
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].x() );
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].y() );
            vdata.push_back( (float)drawObj->m_PntMesh[i][j].z() );

            vdata.push_back( (float)drawObj->m_NormMesh[i][j].x() );
            vdata.push_back( (float)drawObj->m_NormMesh[i][j].y() );
            vdata.push_back( (float)drawObj->m_NormMesh[i][j].z() );

            vdata.push_back( (float)textureCoords[i][j].x() );
            vdata.push_back( (float)textureCoords[i][j].y() );
        }
    }
    destObj->setFacingCW( drawObj->m_FlipNormals );

    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof(float) * vdata.size() );

    // Element Buffer.
    for( int i = 0; i < num_pnts - 1; i++ )
    {
        for( int j = 0; j < num_xsecs; j++ )
        {
            edata.push_back( i * num_xsecs + j );
            edata.push_back( ( i + 1 ) * num_xsecs + j );

            if( j == num_xsecs - 1 )
            {	
                edata.push_back( ( i + 1 ) * num_xsecs );
                edata.push_back( i * num_xsecs );
            }
            else
            {
                edata.push_back( ( i + 1 ) * num_xsecs + j + 1 );
                edata.push_back( i * num_xsecs + j + 1 );
            }
        }
    }
    destObj->emptyEBuffer();
    destObj->appendEBuffer( edata.data(), sizeof( unsigned int ) * edata.size() );
    destObj->enableEBuffer( true );

    // Update number of xsec and pnts.
    XSecEntity * xEntity = dynamic_cast<XSecEntity*>(destObj);
    if(xEntity)
    {
        xEntity->setNumXSec(num_xsecs);
        xEntity->setNumPnts(num_pnts);
    }
}

void VspGlWindow::Private::loadTrisData( Renderable * destObj, DrawObj * drawObj )
{
    assert( drawObj->m_PntVec.size() == drawObj->m_NormVec.size() );

    std::vector<float> data;

    for ( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        data.push_back( ( float )drawObj->m_PntVec[i].x() );
        data.push_back( ( float )drawObj->m_PntVec[i].y() );
        data.push_back( ( float )drawObj->m_PntVec[i].z() );

        data.push_back( ( float )drawObj->m_NormVec[i].x() );
        data.push_back( ( float )drawObj->m_NormVec[i].y() );
        data.push_back( ( float )drawObj->m_NormVec[i].z() );

        data.push_back( 0.0f );
        data.push_back( 0.0f );
    }
    destObj->setFacingCW( drawObj->m_FlipNormals );

    destObj->emptyVBuffer();
    destObj->appendVBuffer( data.data(), sizeof( float ) * data.size() );
}

void VspGlWindow::Private::loadMarkData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> data;

    for ( int i = 0; i < (int)drawObj->m_PntVec.size(); i++ )
    {
        data.push_back( ( float )drawObj->m_PntVec[i].x() );
        data.push_back( ( float )drawObj->m_PntVec[i].y() );
        data.push_back( ( float )drawObj->m_PntVec[i].z() );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
        data.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer(data.data(), sizeof( float ) * data.size());
}

std::vector<std::vector<vec3d>> VspGlWindow::Private::generateTexCoordFromXSec( DrawObj * drawObj )
{
    int i, j;
    std::vector<std::vector<vec3d>> coordinates;

    int num_pnts = drawObj->m_PntMesh.size();
    int num_xsecs = 0;
    if ( num_pnts )
        num_xsecs = drawObj->m_PntMesh[0].size();

    // Initialize coordinates vector.
    coordinates.resize( num_pnts );
    for ( i = 0; i < num_pnts; i++ )
    {
        coordinates[i].resize( num_xsecs );
    }

    // Calculate Texture Coordinate.
    for ( i = 0 ; i < num_pnts ; i++ )
    {
        double totalDistance = 0.0;
        double currPos = 0.0;

        // Calculate the distance between each vertex and total distance.
        coordinates[i][0].set_y( 0.0 );
        for ( j = 1 ; j < num_xsecs ; j++ )
        {
            double distance = this->distance( drawObj->m_PntMesh[i][j - 1], drawObj->m_PntMesh[i][j] );
            totalDistance += distance;
            coordinates[i][j].set_y( distance );
        }

        // Normalize y.
        for ( j = 0; j < num_xsecs; j++ )
        {
            currPos += coordinates[i][j].y();

            // In case totalDistance equals 0 (pointy ends of pods), 
            // set normalized x to 0.0.
            coordinates[i][j].set_y( totalDistance <= 0.0 ? (j + 1) * (1.0 / num_xsecs) : currPos / totalDistance );
        }
    }

    for ( i = 0 ; i < num_xsecs ; i++ )
    {
        double totalDistance = 0.0;
        double currPos = 0.0;

        // Calculate the distance between each vertex and total distance.
        coordinates[0][i].set_x( 0.0 );
        for( j = 1; j < num_pnts ; j++ )
        {
            double distance = this->distance( drawObj->m_PntMesh[j - 1][i], drawObj->m_PntMesh[j][i] );
            totalDistance += distance;
            coordinates[j][i].set_x( distance );
        }

        // Normalize x.
        for ( j = 0; j < num_pnts; j++ )
        {
            currPos += coordinates[j][i].x();

            // In case totalDistance equals 0 (pointy ends of pods), 
            // set normalized y to 0.0.
            coordinates[j][i].set_x( totalDistance <= 0.0 ? ( j + 1 ) * ( 1.0 / num_pnts ) : currPos / totalDistance );
        }
    }
    return coordinates;
}

void VspGlWindow::Private::setLighting( DrawObj * drawObj )
{
    if( drawObj->m_Type != DrawObj::VSP_SETTING )
        return;

    // Currently only support up to eight light sources.
    assert( drawObj->m_LightingInfos.size() <= 8 );
    if( drawObj->m_LightingInfos.size() > 8 )
        return;

    DrawObj::LightSourceInfo lInfo;

    LightSource * lSource;

    Lighting * lights = scene()->getLights();

    for( int i = 0; i < (int)drawObj->m_LightingInfos.size(); i++ )
    {
        lInfo = drawObj->m_LightingInfos[i];

        lSource = lights->getLightSource( i );

        lSource->position( lInfo.X, lInfo.Y, lInfo.Z, 0.0f );
        lSource->ambient( lInfo.Amb, lInfo.Amb, lInfo.Amb, 1.0f );
        lSource->diffuse( lInfo.Diff, lInfo.Diff, lInfo.Diff, 1.0f );
        lSource->specular( lInfo.Spec, lInfo.Spec, lInfo.Spec, 1.0f );

        if( lInfo.Active )
            lSource->enable();
        else
            lSource->disable();
    }
}

void VspGlWindow::Private::setClipping( DrawObj * drawObj )
{
    if( drawObj->m_Type != DrawObj::VSP_CLIP )
        return;

    Clipping * clip = scene()->GetClipping();

    for( int i = 0; i < 6; i++ )
    {
        ClipPlane * cplane = clip->getPlane( i );

        if( drawObj->m_ClipFlag[ i ] )
        {
            cplane->enable();
        }
        else
        {
            cplane->disable();
        }

        vec3d n(0, 0, 0);
        vec3d p(0, 0, 0);

        int jaxis = i / 2;  // Integer division implies floor.

        if( i % 2 == 0 )
        {
            n.v[ jaxis ] = -1.0;
        }
        else
        {
            n.v[ jaxis ] = 1.0;
        }

        p.v[ jaxis ] = drawObj->m_ClipLoc[ i ];

        double e[4] = {n.v[0], n.v[1], n.v[2], -dot( n, p )};

        cplane->setEqn( e );
    }
}

double VspGlWindow::Private::distance( vec3d pointA, vec3d pointB )
{
    double x = pointB.x() - pointA.x();
    double y = pointB.y() - pointA.y();
    double z = pointB.z() - pointA.z();

    return std::sqrt( x * x + y * y + z * z );
}

void VspGlWindow::mousePressEvent( QMouseEvent * ev )
{
    V_D( VspGlWindow );
    VSPGraphic::Display * display = d->display();
    d->updateMouse();
    display->selectViewport( d->mouse );

    if ( ev->buttons() == ( Qt::LeftButton | Qt::RightButton ) )
    {
        d->prevLBRB = d->mouse;
    }
    else if( ev->buttons() == Qt::LeftButton )
    {
        if ( ev->modifiers() == Qt::AltModifier )
        {
            d->prevAltLB = d->mouse;
        }
        else if( ev->modifiers() == Qt::ControlModifier )
        {
            d->prevCtrlLB = d->mouse;
        }
        else if( ev->modifiers() == Qt::MetaModifier )
        {
            d->prevMetaLB = d->mouse;
        }
        else if( ev->modifiers() == Qt::NoModifier )
        {
            // LB
            glm::vec3 mouseInWorld = kNullVec3;

            // Getting mouse location in world space.  This is for selectLocation().
            Viewport * vp = d->viewport();
            if( vp )
            {
                mouseInWorld = vp->screenToWorld( glm::vec2( d->mouse.x(), d->mouse.y() ) );
            }

            // Select highlighted.
            if( d->scene()->selectHighlight() )
            {
                Selectable * selected = d->scene()->getLastSelected();
                d->sendFeedback( selected );
            }
            // Select location in world.
            else if( mouseInWorld != kNullVec3 &&
                d->scene()->selectLocation( mouseInWorld.x, mouseInWorld.y, mouseInWorld.z ) )
            {
                Selectable * selected = d->scene()->getLastSelected();
                d->sendFeedback( selected );
            }
            // Rotation.
            else
            {
                d->prevLB = d->mouse;
            }
        }
    }
    else if( Fl::event_button2() )
    {
        // MB
        d->prevMB = d->mouse;
    }
    else if( Fl::event_button3() )
    {
        // RB
        d->prevRB = d->mouse;
    }
    QGLWidget::update();
}

void VspGlWindow::mouseMoveEvent( QMouseEvent * ev )
{
    V_D( VspGlWindow );
    d->updateMouse();
    if ( ! ev->buttons() ) {
        if ( d->gEngine.getScene()->isPickingEnabled() ) {
            update();
        }
        return;
    }

    VSPGraphic::Display * display = d->display();
    if ( ev->buttons() == ( Qt::LeftButton | Qt::RightButton ) )
    {
        if( ! d->prevLBRB.isNull() )
        {
            display->zoom( d->prevLBRB, d->mouse );
        }
        d->prevLBRB = d->mouse;
    }
    else if ( ev->buttons() == Qt::LeftButton )
    {
        if ( ev->modifiers() == Qt::AltModifier )
        {
            if( ! d->prevAltLB.isNull() )
            {
                display->pan( d->prevAltLB, d->mouse );
            }
            d->prevAltLB = d->mouse;
        }
        else if ( ev->modifiers() == Qt::ControlModifier )
        {
            if( ! d->prevCtrlLB.isNull() )
            {
                display->zoom( d->prevCtrlLB, d->mouse );
            }
            d->prevCtrlLB = d->mouse;
        }
        else if ( ev->modifiers() == Qt::MetaModifier )
        {
            if( ! d->prevMetaLB.isNull() )
            {
                display->zoom( d->prevMetaLB, d->mouse );
            }
            d->prevMetaLB = d->mouse;
        }
        else if ( ev->modifiers() == Qt::NoModifier )
        {
            if( ! d->prevLB.isNull() )
            {
                display->rotate( d->prevLB, d->mouse );
            }
            d->prevLB = d->mouse;
        }
    }
    else if( ev->buttons() == Qt::MiddleButton )
    {
        if( ! d->prevMB.isNull() )
        {
            display->zoom( d->prevMB, d->mouse );
        }
        d->prevMB = d->mouse;
    }
    else if( ev->buttons() == Qt::RightButton )
    {
        if( ! d->prevRB.isNull() )
        {
            display->pan( d->prevRB, d->mouse );
        }
        d->prevRB = d->mouse;
    }
    QGLWidget::update();
}


/// Reset button positions.
void VspGlWindow::mouseReleaseEvent( QMouseEvent * ev )
{   
    V_D( VspGlWindow );
    d->updateMouse();
    if ( ! ev->buttons() & Qt::LeftButton )
    {
        d->prevLB = d->prevAltLB = d->prevCtrlLB = d->prevMetaLB = QPoint();
    }
    if ( ! ev->buttons() & Qt::RightButton )
    {
        d->prevRB = QPoint();
    }
    if (! ev->buttons() & Qt::MiddleButton )
    {
        d->prevMB = QPoint();
    }
    QGLWidget::update();
}

void VspGlWindow::keyReleaseEvent( QKeyEvent * ev )
{
    V_D( VspGlWindow );
    d->updateMouse();
    VSPGraphic::Display * display = d->display();

    switch( ev->key() )
    {
    case Qt::Key_C: // center
    {
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            BndBox bbox = vPtr->GetBndBox();
            vec3d p = bbox.GetCenter();
            display->setCOR( -p.x(), -p.y(), -p.z() );
        }
        display->center();
        break;
    }

    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
        if ( ev->modifiers() == Qt::ShiftModifier )
        {
            display->save( ev->key() - Qt::Key_F1 );
        }
        else if ( ev->modifiers() == Qt::NoModifier )
        {
            display->load( ev->key() - Qt::Key_F1 );
        }
        break;

    case Qt::Key_F5:
        display->changeView( Common::VSP_CAM_TOP );
        break;

    case Qt::Key_F6:
        display->changeView( Common::VSP_CAM_FRONT );
        break;

    case Qt::Key_F7:
        display->changeView( Common::VSP_CAM_LEFT );
        break;

    case Qt::Key_F8:
        display->changeView( Common::VSP_CAM_LEFT_ISO );
        break;

    case Qt::Key_F9:
        display->changeView( Common::VSP_CAM_BOTTOM );
        break;

    case Qt::Key_F10:
        display->changeView( Common::VSP_CAM_REAR );
        break;

    case Qt::Key_F11:
        display->changeView( Common::VSP_CAM_RIGHT );
        break;

    case Qt::Key_F12:
        display->changeView( Common::VSP_CAM_RIGHT_ISO );
        break;

    case Qt::Key_Alt:
    case Qt::Key_Control:
    case Qt::Key_Meta:
        // If mouse still pressed, start perform mouse action.
        if( QGuiApplication::mouseButtons() != Qt::NoButton) {
            d->updateMouse();
            if ( QGuiApplication::mouseButtons() & Qt::LeftButton )
            {
                d->prevLB = d->mouse;
                switch ( ev->key() ) {
                    // Stop modifier+button control
                case Qt::Key_Alt:
                    d->prevAltLB = QPoint();
                    break;
                case Qt::Key_Control:
                    d->prevCtrlLB = QPoint();
                    break;
                case Qt::Key_Meta:
                    d->prevMetaLB = QPoint();
                    break;
                }
            }
            if( QGuiApplication::mouseButtons() & Qt::MiddleButton )
            {
                d->prevMB = d->mouse;
            }
            if( QGuiApplication::mouseButtons() & Qt::RightButton )
            {
                d->prevRB = d->mouse;
            }
        }
        break;
    }
    QGLWidget::update();
}

void VspGlWindow::keyPressEvent( QKeyEvent * ev )
{
    V_D( VspGlWindow );
    ManageGeomScreen * geomScreen = NULL;
    ManageCORScreen * corScreen = NULL;
    d->updateMouse();

    switch( ev->key() )
    {
    case Qt::Key_P:
        geomScreen = dynamic_cast<ManageGeomScreen *>
            ( d->getScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );
        if( geomScreen )
        {
            geomScreen->TriggerPickSwitch();
        }
        break;
    case Qt::Key_R:
        corScreen = dynamic_cast<ManageCORScreen *> 
            ( d->getScreen( ScreenMgr::VSP_COR_SCREEN ) );
        if( corScreen )
        {
            corScreen->EnableSelection();
        }
        break;
    case Qt::Key_Escape:
        Vehicle* vPtr = VehicleMgr.GetVehicle();
        if ( vPtr )
        {
            vector< string > none;
            vPtr->SetActiveGeomVec( none );
            d->screenMgr->SetUpdateFlag( true );
        }
        break;
    }
    QGLWidget::update();
}

void VspGlWindow::Private::sendFeedback( Selectable * selected )
{
    // Find out where feedback is heading...
    std::string selectedFeedbackName = selected->getGroup();

    // Label Screen Feedback
    ManageLabelScreen * labelScreen = dynamic_cast<ManageLabelScreen*>
        ( getScreen( ScreenMgr::VSP_LABEL_SCREEN ) );

    if( labelScreen && labelScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        // Location feedback
        SelectedLoc * loc = dynamic_cast<SelectedLoc*> ( selected );
        if( loc )
        {
            glm::vec3 placement = loc->getLoc();
            labelScreen->Set( vec3d(placement.x, placement.y, placement.z ) );

            // Only one selection is needed for label, remove this 'selected' from selection list.
            scene()->removeSelected( selected );
            selected = NULL;
        }

        // Vertex feedback
        // Cast selectable to SelectedPnt object, so that we can get Render Source Ptr.
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            XSecEntity * xEntity = dynamic_cast<XSecEntity*>(pnt->getSource());
            if(xEntity)
            {
                ID & id = findID( xEntity->getID() );
                if( id )
                {
                    int index = id.geomID.find_last_of( '_' );
                    std::string baseId = id.geomID.substr( 0, index );
                    glm::vec3 placement = xEntity->getVertexVec(pnt->getIndex());
                    labelScreen->Set( vec3d( placement.x, placement.y, placement.z ), baseId );

                    // Only one selection is needed for label, remove this 'selected' from selection list.
                    scene()->removeSelected( selected );
                    selected = NULL;
                }
            }
        }
    }

    // Geom Screen Feedback
    ManageGeomScreen * geomScreen = dynamic_cast<ManageGeomScreen *>
        ( getScreen( ScreenMgr::VSP_MANAGE_GEOM_SCREEN ) );

    if( geomScreen && geomScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedGeom * geom = dynamic_cast<SelectedGeom*>( selected );
        if( geom )
        {
            ID & id = findID( geom->getSource()->getID() );
            if( id )
            {
                int index = id.geomID.find_last_of( '_' );
                std::string baseId = id.geomID.substr( 0, index );
                geomScreen->Set( baseId );

                // Only one selection is needed for Geom, remove this 'selected' from selection list.
                scene()->removeSelected( selected );
                selected = NULL;
            }
        }
        // Mac Fix:  On Mac, FLTK window is always focus on the last shown GUI.  In
        // this case where a geometry is selected, geometry's window becomes the new
        // focus.  The behavior locks up geometry selection process.  Set OpenGL
        // window back on focus so user can proceed without interruption.
        q_ptr->setFocus();
    }

    // Center of rotation Screen Feedback
    ManageCORScreen * corScreen = dynamic_cast<ManageCORScreen*>
        ( getScreen( ScreenMgr::VSP_COR_SCREEN ) );

    if( corScreen && corScreen->getFeedbackGroupName() == selectedFeedbackName )
    {
        SelectedPnt * pnt = dynamic_cast<SelectedPnt*>( selected );
        if( pnt )
        {
            VSPGraphic::Entity * entity = dynamic_cast<VSPGraphic::Entity*>(pnt->getSource());
            if(entity)
            {
                glm::vec3 placement = entity->getVertexVec(pnt->getIndex());

                display()->setCOR( -placement.x, -placement.y, -placement.z );
                display()->center();

                // This is a dummy call to let corScreen know the job is done.
                corScreen->Set( vec3d( placement.x, placement.y, placement.z ) );

                // Only one selection is needed for Center of Rotation, remove this 'selected' from selection list.
                scene()->removeSelected( selected );
                selected = NULL;
            }
        }
    }
}

void VspGlWindow::Private::sendFeedback( std::vector<Selectable *> listOfSelected )
{
    for ( int i = 0; i < (int) listOfSelected.size(); i++ )
    {
        sendFeedback( listOfSelected[i] );
    }
}

VspGlWindow::~VspGlWindow() {}
} // namespace VSPGUI
