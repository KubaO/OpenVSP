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

using namespace std;

class DrawObj
{
public:
    /*
    * Constructor.
    */
    DrawObj();
    /*
    * Destructor.
    */
    virtual ~DrawObj();

    /*
    * Render Type Enum.
    *
    * VSP_POINTS - Render to points.
    * VSP_LINES - Render to lines.
    * VSP_LINE_STRIP - Render to line strip.
    * VSP_LINE_LOOP - Render to line loop.
    * VSP_WIRE_MESH - Render to wire frame.
    * VSP_HIDDEN_MESH - Render to solid wire frame.
    * VSP_SHADED_MESH - Render to mesh with lighting.
    * VSP_TEXTURED_MESH - Render to texture mapped mesh with lighting.
    * VSP_WIRE_TRIS - Render Triangles to wire frame.
    * VSP_HIDDEN_TRIS - Render Triangles to solid wire frame.
    * VSP_SHADED_TRIS - Render Triangles to mesh with lighting.
    *
    * VSP_SETTING - This drawObj provides Global Setting Info(lighting, etc...).  Does not render anything.
    */
    enum TypeEnum
    {
        VSP_POINTS,
        VSP_LINES,
        VSP_LINE_STRIP,
        VSP_LINE_LOOP,
        VSP_WIRE_MESH,
        VSP_HIDDEN_MESH,
        VSP_SHADED_MESH,
        VSP_TEXTURED_MESH,
        VSP_WIRE_TRIS,
        VSP_HIDDEN_TRIS,
        VSP_SHADED_TRIS,
        VSP_SETTING,
    };

    /*
    * Target Screen Enum.
    * This is not Screen Type, each screen represent an actual screen
    * in GUI.
    *
    * VSP_MAIN_SCREEN - Main VSP display window.
    * VSP_XSEC_SCREEN - 2D display window in XSec Panel.
    */
    enum ScreenEnum
    {
        VSP_MAIN_SCREEN,
        VSP_XSEC_SCREEN,
        VSP_TEX_PREVIEW,
    };

    /*
    * Information needed to map a single texture.
    *
    * FileName - Texture File Path + File Name.
    * ID - Texture ID, this ID is only unique to this drawObj.
    *
    * U - Texture's position on U direction.
    * W - Texture's position on W direction.
    *
    * UScale - Texture's scale on U direction.
    * WScale - Texture's scale on W direction.
    *
    * Transparency - Texture's Alpha value.
    *
    * UFlip - Flag for if U coordinate is flipped.
    * WFlip - Flag for if W coordinate is flipped.
    */
    struct TextureInfo
    {
        string FileName;
        unsigned int ID;

        float U;
        float W;

        float UScale;
        float WScale;

        float Transparency;

        bool UFlip;
        bool WFlip;
    };

    /*
    * Light Source Information.
    *
    * Active - True if Light Source is active.
    *
    * X - Light Source's position on X-axis.
    * Y - Light Source's position on Y-axis.
    * Z - Light Source's position on Z-axis.
    *
    * Amb - Ambient Light Value.
    * Diff - Diffuse Light Value.
    * Spec - Specular Light Value.
    */
    struct LightSourceInfo
    {
        bool Active;

        float X;
        float Y;
        float Z;

        float Amb;
        float Diff;
        float Spec;
    };

    /*
    * Unique Geom ID.
    * Default ID = "Default".
    */
    string m_GeomID;

    /*
    * Visibility of Geom.
    * Geometry is visible on true and invisible on false.
    * Visibility is set to true by default.
    */
    bool m_Visible;

    /*
    * Geometry changed flag.
    * This flag is used for optimization.  Flag true if geometry data are changed and require
    * graphic buffer update.
    */
    bool m_GeomChanged;

    /*
    * Desired Render type.
    * m_Type is set to VSP_POINTS by default.
    */
    TypeEnum m_Type;

    /*
    * Screen to display this drawObj.
    * m_Screen is set to VSP_MAIN_SCREEN by default.
    */
    ScreenEnum m_Screen;

    /*
    * No Use for now.
    */
    int m_Priority;

    /*
    * Line thickness.
    * LineWidth is set to 1.0 by default.
    */
    double m_LineWidth;
    /*
    * RGB Line color.
    * LineColor is set to blue(0, 0, 1) by default.
    */
    vec3d m_LineColor;

    /*
    * Point Size.
    * PointSize is set to 5.0 by default.
    */
    double m_PointSize;
    /*
    * RGB Point color.
    * PointColor is set to red(1, 0, 0) by default.
    */
    vec3d m_PointColor;

    /*
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
    vector< vec3d > m_PntVec;
    /*
    * Array of normal data.
    */
    vector< vec3d > m_NormVec;
    /*
    * XSec data.
    * m_PntMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_PntMesh[pnts on xsec][xsec index]
    */
    vector< vector< vec3d > > m_PntMesh;
    /*
    * XSec normals.
    * m_NormMesh is available if m_Type is one of the following:
    * VSP_WIRE_MESH, VSP_HIDDEN_MESH, VSP_SHADED_MESH, VSP_TEXTURED_MESH
    *
    * Data format:
    * m_NormMesh[pnts on xsec][xsec index]
    */
    vector< vector< vec3d > > m_NormMesh;

    /*
    * List of attached textures to this drawobj.  Default is empty.
    */
    vector<TextureInfo> m_TextureInfos;

    /*
    * List of Light Source Information.  Lighting is global, there will be
    * only one drawObj that has m_LightingInfos to avoid redundant processes.
    * Otherwise m_LightingInfos is empty.
    */
    vector<LightSourceInfo> m_LightingInfos;

protected:

};



#endif

