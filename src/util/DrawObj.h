//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// DrawObj.h: 
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_DRAWOBJ_H
#define VSP_DRAWOBJ_H

#include "Vec3d.h"

#include <vector>
#include <string>

#define PICKGEOMHEADER "GPICKING_"
#define PICKVERTEXHEADER "VPICKING_"
#define PICKLOCHEADER "LPICKING_"
#define BBOXHEADER "BBOX_"
#define XSECHEADER "XSEC_"

class DrawObj
{
public:
    DrawObj();
    virtual ~DrawObj();

    vec3d ColorWheel( double angle ); ///< Returns rgb for an angle given in degrees
    /*! Render Type */
    enum TypeEnum
    {
        VSP_POINTS, ///< Render to points.
        VSP_LINES, ///< Render to lines.
        VSP_LINE_STRIP, ///< Render to line strip.
        VSP_LINE_LOOP, ///< Render to line loop.
        VSP_WIRE_MESH, ///< Render to wire frame.
        VSP_HIDDEN_MESH, ///< Render to solid wire frame.
        VSP_SHADED_MESH, ///< Render to mesh with lighting.
        VSP_TEXTURED_MESH, ///< Render to texture mapped mesh with lighting.
        VSP_WIRE_TRIS, ///< Render Triangles to wire frame.
        VSP_HIDDEN_TRIS, ///< Render Triangles to solid wire frame.
        VSP_SHADED_TRIS, ///< Render Triangles to mesh with lighting.
        VSP_HIDDEN_TRIS_CFD,
        VSP_SETTING, ///< This drawObj provides Global Setting Info(lighting, etc...).  Does not render anything.
        VSP_RULER, ///< Render Ruler.
        VSP_PICK_VERTEX, ///< This drawObj enables vertex picking for a specific geometry.
        VSP_PICK_VERTEX_SELECT_ALL, ///< Same as VSP_PICK_VERTEX, but also select all vertices.
        VSP_PICK_VERTEX_UNSELECT_ALL, ///< Send signal to clear all existing selections. Does not contain render information.
        VSP_PICK_VERTEX_HIDE_SELECTION, ///< Send signal to hide all selections for this one update.  Does not contain render information.
        VSP_PICK_GEOM, ///< This drawObj enables geometry picking.
        VSP_PICK_LOCATION, ///< This type drawObj enables location picking.
        VSP_CLIP, ///< Add clipping plane.
    };

    /*!
    * \brief Target Screen
    * This is not Screen Type, each screen represent an actual screen
    * in the GUI.
    */
    enum ScreenEnum
    {
        VSP_MAIN_SCREEN, ///< Main VSP display window.
        VSP_XSEC_SCREEN, ///< 2D display window in XSec Panel.
        VSP_TEX_PREVIEW, ///< Texture preview window.
    };

    /*!
    * \brief Ruler Step
    * A ruler requires three steps to complete.  On first
    * step, only start point is given.  Ruler is drawn
    * between start point and mouse location.  On second
    * step, both start and end point are given.  Ruler is
    * drawn between those points.  Lastly, offset is set and
    * ruler is set to appropriate height.
    */
    enum RulerEnum
    {
        VSP_RULER_STEP_ZERO,
        VSP_RULER_STEP_ONE,
        VSP_RULER_STEP_TWO,
        VSP_RULER_STEP_COMPLETE,
    };

    /*! \brief Ruler Information. */
    struct Ruler
    {
        RulerEnum Step; ///< Current step of building a ruler.
        vec3d Start; ///<  Vertex Information of starting point.
        vec3d End; ///< Vertex Information of ending point.
        vec3d Offset; ///< Placement of the ruler.
    };

    /*! \brief Information needed to map a single texture. */
    struct TextureInfo
    {
        std::string FileName; ///< Texture File Path + File Name.
        std::string ID; ///< Texture ID, this ID is only unique to this drawObj.

        float U; ///< Texture's position on U direction.
        float W; ///< Texture's position on W direction.

        float UScale; ///< Texture's scale on U direction.
        float WScale; ///< Texture's scale on W direction.

        float Transparency; ///< Texture's Alpha value.

        bool UFlip; ///< The U coordinate is flipped.
        bool WFlip; ///< The W coordinate is flipped.
    };

    /*! \brief Information needed to define a material. */
    struct MaterialInfo
    {
        float Ambient[4];
        float Diffuse[4];
        float Specular[4];
        float Emission[4];

        float Shininess; ///< Shine value, the value is between 0 and 128.
    };

    /*! \brief Light Source Information. */
    struct LightSourceInfo
    {
        bool Active; ///< The light is active.

        float X; ///< Light Source's position on X-axis.
        float Y; ///< Light Source's position on Y-axis.
        float Z; ///< Light Source's position on Z-axis.

        float Amb; ///< Ambient Light Value.
        float Diff; ///< Diffuse Light Value.
        float Spec; ///< Specular Light Value.
    };

    /*!
    * \brief Unique Geometry ID.
    * Default ID = "Default".
    */
    std::string m_GeomID;

    /*!
    * If m_Type is a picking type, m_PickSourceID stores GeomID of 
    * the "Host Geometry".  
    * Picking DrawObjs do not store any data, its job is to set a target
    * geometry to become selectable. The target geometry is the host,
    * and its id goes here.
    */
    std::string m_PickSourceID;

    /*!
    * If m_Type is a picking type, m_FeedbackGroup identifies where the
    * picking feedback is going.  With this, we can have multiple picking
    * sessions active at the same time.
    */
    std::string m_FeedbackGroup;

    /*!
    * \brief Visibility of Geom.
    * Geometry is visible on true and invisible on false.
    * Visibility is set to true by default.
    */
    bool m_Visible;

    /*!
    * \brief Geometry changed flag.
    * This flag is used for optimization.  Flag true if geometry data are changed and require 
    * graphic buffer update.
    */
    bool m_GeomChanged;

    /*!
    * \brief Flip Normals flag.
    * If true, Normals need to flip to the opposite direction.
    */
    bool m_FlipNormals;

    /*!
    * \brief Desired Render type.
    * m_Type is set to VSP_POINTS by default.
    */
    TypeEnum m_Type;

    /*!
    * \brief Screen to display this drawObj.
    * m_Screen is set to VSP_MAIN_SCREEN by default.
    */
    ScreenEnum m_Screen;

    /*!
    * \brief Holds ruler information.
    * A ruler requires three steps to complete.  On first
    * step, only start point is given.  Ruler is drawn
    * between start point and mouse location.  On second
    * step, both start and end point are given.  Ruler is
    * drawn between those points.  Lastly, offset is set and
    * ruler is set to appropriate height.
    */
    Ruler m_Ruler;

#if 0
    /*! Unused. Default value: 0. */
    int m_Priority;
#endif

    /*!
    * \brief Line thickness.
    * LineWidth is set to 1.0 by default.
    */
    double m_LineWidth;
    /*!
    * \brief RGB Line Color.
    * LineColor is set to blue(0, 0, 1) by default.
    */
    vec3d m_LineColor;

    /*!
    * \brief Point Size.
    * PointSize is set to 5.0 by default.
    */
    double m_PointSize;
    /*!
    * \brief RGB Point Color.
    * PointColor is set to red(1, 0, 0) by default.
    */
    vec3d m_PointColor;

    /*! \brief Text size for label. */
    double m_TextSize;
    /*! \brief RGB Text Color. */
    vec3d m_TextColor;

    /*!
    * Array of vertex data.
    * m_PntVec is available if m_Type is one of the following:
    * VSP_POINTS, VSP_LINES, VSP_WIRE_TRIS, VSP_HIDDEN_TRIS,
    * VSP_SHADED_TRIS.
    *
    * The data format is based on m_Type.
    * On VSP_POINTS, data are stored as v0 v1 v2...
    * On VSP_LINES, data are stored as v0 v1 v1 v2...
    * On VSP_WIRE_TRIS, VSP_HIDDEN_TRIS, and VSP_SHADED_TRIS,
    * data are stored as v0, v1, v2...
    */
    std::vector< vec3d > m_PntVec;
    /*!
    * XSec data.
    * m_PntMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_PntMesh[pnts on xsec][xsec index]
    */
    std::vector< std::vector< vec3d > > m_PntMesh;
    /*!
    * XSec normals.
    * m_NormMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_NormMesh[pnts on xsec][xsec index]
    */ 
    std::vector< std::vector< vec3d > > m_NormMesh;
    std::vector< vec3d > m_NormVec; // For triangles

    /// List of attached textures to this drawobj.  Default is empty.
    std::vector<TextureInfo> m_TextureInfos;

    /*!
    * List of Light Source Information.  Lighting is global, there will be
    * only one drawObj that has m_LightingInfos to avoid redundant processes.
    * Otherwise m_LightingInfos is empty.
    */
    std::vector<LightSourceInfo> m_LightingInfos;

    /*! Material information of this DrawObj. */
    MaterialInfo m_MaterialInfo;

    std::vector< double > m_ClipLoc;
    std::vector< bool > m_ClipFlag;
};



#endif


