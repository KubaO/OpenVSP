//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "XSecCurve.h"
#include "Geom.h"
#include "XSecSurf.h"
//#include "SuperEllipse.h"
#include "Parm.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include <float.h>
#include <stdio.h>
#include "APIDefines.h"

#include "Vehicle.h"

#include "eli/geom/curve/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_point_creator<double, 3, curve_tolerance_type> piecewise_point_creator;
typedef eli::geom::curve::piecewise_circle_creator<double, 3, curve_tolerance_type> piecewise_circle_creator;
typedef eli::geom::curve::piecewise_ellipse_creator<double, 3, curve_tolerance_type> piecewise_ellipse_creator;
typedef eli::geom::curve::piecewise_superellipse_creator<double, 3, curve_tolerance_type> piecewise_superellipse_creator;

using std::string;
using namespace vsp;

//==== Default Constructor ====//
XSecCurve::XSecCurve()
{
    m_GroupName = "XSecCurve";
    m_GroupSuffix = -1;
}

void XSecCurve::SetGroupDisplaySuffix( int num )
{
    m_GroupSuffix = num;
    //==== Assign Group Suffix To All Parms ====//
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );
        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

string XSecCurve::GetName()
{
    ParmContainer* pc = GetParentContainerPtr();

    if ( pc )
    {
        char str[256];
        sprintf( str, "_%d", m_GroupSuffix );
        return pc->GetParentContainerPtr()->GetName() + " " + m_GroupName + string(str);
    }
    return ParmContainer::GetName();
}

//==== Set Scale ====//
void XSecCurve::SetScale( double scale )
{
    SetWidthHeight( GetWidth()*scale, GetHeight()*scale );
}

//==== Parm Changed ====//
void XSecCurve::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;

        //==== Notify Parent Container (XSec) ====//
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            pc->ParmChanged( parm_ptr, type );
        }

        return;
    }

    Update();

    //==== Notify Parent Container (XSec) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}

//==== Update ====//
void XSecCurve::Update()
{
    m_LateUpdateFlag = false;
}

//==== Get Curve ====//
VspCurve& XSecCurve::GetCurve()
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve;
}

//==== Encode XML ====//
xmlNodePtr XSecCurve::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSecCurve", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
    }
    return xsec_node;
}

//==== Copy From ====//
void XSecCurve::CopyFrom( XSecCurve* from_crv )
{
    ParmMgr.ResetRemapID();
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );

    from_crv->EncodeXml( root );
    DecodeXml( root );

    xmlFreeNode( root );
    ParmMgr.ResetRemapID();
}

//==== Compute Area ====//
double XSecCurve::ComputeArea( int num_pnts )
{
    VspCurve curve = GetCurve();
    vector<vec3d> pnts;
    curve.Tesselate( num_pnts, pnts );
    vec3d zero;
    return poly_area( pnts, zero );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
PointXSec::PointXSec( ) : XSecCurve( )
{
    m_Type = XS_POINT;
}

//==== Update Geometry ====//
void PointXSec::Update()
{
    piecewise_curve_type c;
    curve_point_type pt;
    pt << 0, 0, 0;

    // create point with 4 segments
    piecewise_point_creator ppc( 4 );

    // set point, make sure have 4 sections that go from 0 to 4
    ppc.set_point( pt );
    ppc.set_t0( 0 );
    ppc.set_segment_dt( 1, 0 );
    ppc.set_segment_dt( 1, 1 );
    ppc.set_segment_dt( 1, 2 );
    ppc.set_segment_dt( 1, 3 );
    if ( !ppc.create( c ) )
    {
        std::cerr << "Failed to create point XSec. " << __LINE__ << std::endl;
    }
    else
    {
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}
//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
CircleXSec::CircleXSec( ) : XSecCurve( )
{
    m_Type = XS_CIRCLE;
    m_Diameter.Init( "Circle_Diameter", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Diameter.SetDescript( "Diameter of Circle Cross-Section" );
}

//==== Set Width and Height ====//
void CircleXSec::SetWidthHeight( double w, double h )
{
    m_Diameter  = ( w + h ) / 2.0;
}

//==== Update Geometry ====//
void CircleXSec::Update()
{
    piecewise_curve_type c;
    piecewise_circle_creator pcc( 4 );
    curve_point_type origin, start, normal;

    origin << m_Diameter() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pcc.set_origin( origin );
    pcc.set_radius( m_Diameter() / 2 );
    pcc.set_t0( 0 );
    pcc.set_segment_dt( 1, 0 );
    pcc.set_segment_dt( 1, 1 );
    pcc.set_segment_dt( 1, 2 );
    pcc.set_segment_dt( 1, 3 );
    if ( !pcc.create( c ) )
    {
        std::cerr << "Failed to create circle XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
EllipseXSec::EllipseXSec( ) : XSecCurve( )
{
    m_Type = XS_ELLIPSE;

    m_Height.Init( "Ellipse_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Ellipse Cross-Section" );
    m_Width.Init( "Ellipse_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Ellipse Cross-Section" );
}

//==== Update Geometry ====//
void EllipseXSec::Update()
{
    piecewise_curve_type c;
    piecewise_ellipse_creator pec( 4 );
    curve_point_type origin, normal;

    origin << m_Width() / 2, 0, 0;
    normal << 0, 0, -1;

    // set circle params, make sure that entire curve goes from 0 to 4
    pec.set_origin( origin );
    pec.set_x_axis_radius( m_Width() / 2 );
    pec.set_y_axis_radius( m_Height() / 2 );
    pec.set_t0( 0 );
    pec.set_segment_dt( 1, 0 );
    pec.set_segment_dt( 1, 1 );
    pec.set_segment_dt( 1, 2 );
    pec.set_segment_dt( 1, 3 );
    if ( !pec.create( c ) )
    {
        std::cerr << "Failed to create ellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}

//==== Set Width and Height ====//
void EllipseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
SuperXSec::SuperXSec( ) : XSecCurve( )
{
    m_Type = XS_SUPER_ELLIPSE;

    m_Height.Init( "Super_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Height.SetDescript( "Height of the Super Ellipse Cross-Section" );
    m_Width.Init( "Super_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Width.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_M.Init( "Super_M", m_GroupName, this, 2.0, 0.2, 5.0 );
    m_M.SetDescript( "Width of the Super Ellipse Cross-Section" );
    m_N.Init( "Super_N", m_GroupName, this, 2.0, 0.2, 5.0 );
}

//==== Update Geometry ====//
void SuperXSec::Update()
{
    piecewise_curve_type c;
    piecewise_superellipse_creator psc( 16 );
    curve_point_type origin, x, y;

    origin << m_Width() / 2, 0, 0;

    // set hyperellipse params, make sure that entire curve goes from 0 to 4
    psc.set_axis( m_Width() / 2, m_Height() / 2 );
    psc.set_max_degree( 3 );
    psc.set_exponents( m_M(), m_N() );
    psc.set_origin( origin );

    psc.set_t0( 0 );
    for ( int i = 0; i < psc.get_number_segments(); ++i )
    {
        psc.set_segment_dt( 4.0 / psc.get_number_segments(), i );
    }

    if ( !psc.create( c ) )
    {
        std::cerr << "Failed to create superellipse XSec. " << __LINE__ << std::endl;
    }
    else
    {
        c.reverse();
        m_Curve.SetCurve( c );

        XSecCurve::Update();
    }
}

//==== Set Width and Height ====//
void SuperXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
RoundedRectXSec::RoundedRectXSec( ) : XSecCurve( )
{
    m_Type = XS_ROUNDED_RECTANGLE;

    m_Height.Init( "RoundedRect_Height", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "RoundedRect_Width", m_GroupName, this,  1.0, 0.0, 1.0e12 );
    m_Radius.Init( "RoundRectXSec_Radius", m_GroupName,  this,  0.2, 0.0, 1.0e12 );
}

//==== Update Geometry ====//
void RoundedRectXSec::Update()
{
    VspCurve edge;
    vector<vec3d> pt;
    vector<double> u;
    double w = m_Width(), h = m_Height();
    double w2 = 0.5 * w, h2 = 0.5 * h, r;
    bool round_curve( true );

    // do some parameter checking
    if ( m_Radius() > w2 )
    {
        m_Radius.Set( w2 );
    }
    if ( m_Radius() > h2 )
    {
        m_Radius.Set( h2 );
    }
    r = m_Radius();

    // catch special cases of degenerate cases
    if ( ( w2 == 0 ) || ( h2 == 0 ) )
    {
        pt.resize( 4 );
        u.resize( 5 );

        // set the segment points
        pt[0].set_xyz(  w,   0, 0 );
        pt[1].set_xyz( w2, -h2, 0 );
        pt[2].set_xyz(  0,   0, 0 );
        pt[3].set_xyz( w2,  h2, 0 );

        // set the corresponding parameters
        u[0] = 0;
        u[1] = 1;
        u[2] = 2;
        u[3] = 3;
        u[4] = 4;

        round_curve = false;
    }
    // create rectangle
    else
    {
        pt.resize( 8 );
        u.resize( 9 );

        // set the segment points
        pt[0].set_xyz(  w,   0, 0 );
        pt[1].set_xyz(  w, -h2, 0 );
        pt[2].set_xyz( w2, -h2, 0 );
        pt[3].set_xyz(  0, -h2, 0 );
        pt[4].set_xyz(  0,   0, 0 );
        pt[5].set_xyz(  0,  h2, 0 );
        pt[6].set_xyz( w2,  h2, 0 );
        pt[7].set_xyz(  w,  h2, 0 );

        // set the corresponding parameters
        u[0] = 0;
        u[1] = h2 / ( h2 + w2 );
        u[2] = 1;
        u[3] = 1 + w2 / ( h2 + w2 );
        u[4] = 2;
        u[5] = 2 + h2 / ( h2 + w2 );
        u[6] = 3;
        u[7] = 3 + w2 / ( h2 + w2 );
        u[8] = 4;
    }

    // build the polygon
    m_Curve.InterpolateLinear( pt, u, true );

    // round all joints if needed
    if ( round_curve )
    {
        m_Curve.RoundAllJoints( r );
    }

    XSecCurve::Update();
    return;
}

//==== Build Box of Even Spaced Points ====//
//==== Set Width and Height ====//
void RoundedRectXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
GeneralFuseXSec::GeneralFuseXSec( ) : XSecCurve( )
{
    m_Type = XS_GENERAL_FUSE;

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_MaxWidthLoc.Init( "MaxWidthLoc", m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
    m_CornerRad.Init( "CornerRad",     m_GroupName, this, 0.0, 0.0, 1.0e12 );
    m_TopTanAngle.Init( "TopTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_BotTanAngle.Init( "BotTanAngle", m_GroupName, this, 90.0, 0.0, 90.0 );
    m_TopStr.Init( "TopStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_BotStr.Init( "BotStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
    m_UpStr.Init( "UpStr",             m_GroupName, this, 0.83, 0.0, 2.0 );
    m_LowStr.Init( "LowStr",           m_GroupName, this, 0.83, 0.0, 2.0 );
}

//==== Set Width and Height ====//
void GeneralFuseXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void GeneralFuseXSec::Update()
{
    double x, y;
    //==== Top Control Points ====//
    vec3d tp0( 0.0, m_Height() / 2.0, 0.0 );
    vec3d tp1 = tp0 + vec3d( m_TopStr() * m_Width() / 3.0, 0.0, 0.0 );

    x = m_Width() / 2.0;
    y = m_MaxWidthLoc() * m_Height() / 2.0;
    vec3d tp3( x, y, 0.0 );

    x = ( m_UpStr() * m_Height() * cos( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    y = ( -m_UpStr() * m_Height() * sin( DEG_2_RAD * m_TopTanAngle() ) ) / 3.0;
    vec3d tp2 = tp3 - vec3d( x, y, 0.0 );

    //==== Bot Control Points ====//
    x = m_Width() / 2.0;
    y = m_MaxWidthLoc() * m_Height() / 2.0;
    vec3d bp0( x, y, 0.0 );

    x = ( double )( -m_LowStr() * m_Height() * cos( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    y = ( double )( -m_LowStr() * m_Height() * sin( DEG_2_RAD * m_BotTanAngle() ) ) / 3.0;
    vec3d bp1 = bp0 + vec3d( x, y, 0.0 );

    vec3d bp3( 0.0, -m_Height() / 2.0, 0.0 );
    vec3d bp2 = bp3 - vec3d( -m_BotStr() * m_Width() / 3.0, 0.0, 0.0 );

    //==== Load Bezier Control Points ====//
    vector< vec3d > bez_pnts;
    bez_pnts.push_back( tp0 );
    bez_pnts.push_back( tp1 );
    bez_pnts.push_back( tp2 );
    bez_pnts.push_back( tp3 );

    int ite = bez_pnts.size() - 1;

    bez_pnts.push_back( bp1 );
    bez_pnts.push_back( bp2 );
    bez_pnts.push_back( bp3 );

    vec3d offset;
    offset.set_x( m_Width() / 2.0 );

    //==== Reflect ====//
    int nrp = bez_pnts.size();
    for ( int i = 2 ; i <= nrp ; i++ )
    {
        vec3d p = bez_pnts[nrp - i];
        p.set_x( -p.x() );
        bez_pnts.push_back( p );
    }

    vector< vec3d > roll_pnts;
    for( int i = ite; i < (int)bez_pnts.size(); i++ )
    {
        roll_pnts.push_back( bez_pnts[i] + offset );
    }
    for ( int i = 1; i <= ite; i++ )
    {
        roll_pnts.push_back( bez_pnts[i] + offset );
    }

    m_Curve.SetCubicControlPoints( roll_pnts, true );

    //==== Corner Points ====//
    VspCurve corner_curve;
    if ( m_CornerRad() > 0.001 )
    {
        m_Curve.RoundJoint( m_CornerRad() * m_Height(), 2 );
        m_Curve.RoundJoint( m_CornerRad() * m_Height(), 0 );
    }

    XSecCurve::Update();
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FileXSec::FileXSec( ) : XSecCurve( )
{
    m_Type = XS_FILE_FUSE;

    // Initialize to closed circle.
    int n = 21;
    for ( int i = 0; i < n; i++ )
    {
        double theta = -2.0*PI*i/(n-1);
        m_UnityFilePnts.push_back( vec3d( 0.5*cos(theta), 0.5*sin(theta), 0.0 ) );
    }

    m_Height.Init( "Height",           m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_Width.Init( "Width",             m_GroupName, this, 1.0, 0.0, 1.0e12 );

}

//==== Set Width and Height ====//
void FileXSec::SetWidthHeight( double w, double h )
{
    m_Width  = w;
    m_Height = h;
}

//==== Update Geometry ====//
void FileXSec::Update()
{
    //==== Scale File Points ====//
    vector< vec3d > scaled_file_pnts;
    vector< double > arclen;

    // Point set is closed with last point repeating first.
    int npts = m_UnityFilePnts.size() - 1;

    for ( int i = 0 ; i < npts ; i++ )
    {
        double x = m_UnityFilePnts[i].x() * m_Width();
        double y = m_UnityFilePnts[i].y() * m_Height();
        scaled_file_pnts.push_back( vec3d( x + m_Width() / 2.0, y, 0.0 ) );

        if ( i > 0 )
        {
            double ds = dist( scaled_file_pnts[i], scaled_file_pnts[i-1] );
            if ( ds < 1e-8 )
            {
                ds = 1.0/npts;
            }
            arclen.push_back( arclen[i-1] + ds );
        }
        else
        {
            arclen.push_back( 0 );
        }

        // Calculate arclen to repeated final point.
        if ( i == npts - 1 )
        {
            x = m_UnityFilePnts[i+1].x() * m_Width();
            y = m_UnityFilePnts[i+1].y() * m_Height();
            double ds = dist( scaled_file_pnts[i], vec3d( x + m_Width() / 2.0, y, 0.0 )  );
            if ( ds < 1e-8 )
            {
                ds = 1.0/npts;
            }
            arclen.push_back( arclen[i] + ds );
        }
    }

    double lenscale = 4.0/arclen.back();

    for ( int i = 0; i < (int)arclen.size(); i++ )
    {
        arclen[i] = arclen[i] * lenscale;
    }

    m_Curve.InterpolatePCHIP( scaled_file_pnts, arclen, true );

    XSecCurve::Update();
}

//==== Encode XML ====//
xmlNodePtr FileXSec::EncodeXml(  xmlNodePtr & node  )
{
    XSecCurve::EncodeXml( node );
    xmlNodePtr child_node  = xmlNewChild( node, NULL, BAD_CAST "FileXSec", NULL );
    if ( child_node )
    {
        XmlUtil::AddVectorVec3dNode( child_node, "UnityFilePnts", m_UnityFilePnts );
    }
    return child_node;
}

//==== Decode XML ====//
xmlNodePtr FileXSec::DecodeXml(  xmlNodePtr & node  )
{
    XSecCurve::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "FileXSec", 0 );
    if ( child_node )
    {
        m_UnityFilePnts = XmlUtil::ExtractVectorVec3dNode( child_node, "UnityFilePnts" );
    }
    return child_node;
}

//==== Read Fuse XSec File ====//
bool FileXSec::ReadXsecFile( string file_name )
{
    FILE* file_id =  fopen( file_name.c_str(), "r" );
    if ( file_id == ( FILE * )NULL )
    {
        return false;
    }

    m_FileName = file_name;

//    if ( ReadOldXSecFile( file_id ) == true )
//    {
//        fclose( file_id );
//        return true;
//    }
//    else if ( ReadXSecFile( file_id ) == true )
    if ( ReadXSecFile( file_id ) == true )
    {
        fclose( file_id );
        return true;
    }

    fclose( file_id );
    return false;
}

/*
//==== Read Old Fuse XSec File ====//
bool FileXSec::ReadOldXSecFile( FILE* file_id )
{
    char buff[256];
    char name[256];
    string line;
    int num_pnts;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "FUSE XSEC FILE" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Find XSec Name ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%s", name );
    m_FileName = string( name );

    //==== Find Num Pnts ====//
    fgets( buff, 80, file_id );
    sscanf( buff, "%d", &num_pnts );
    if ( num_pnts < 3 )
    {
        return false;
    }

    //==== Find Points ====//
    float x, y;
    vector< vec3d > pnt_vec;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        if ( fgets( buff, 80, file_id ) )
        {
            sscanf( buff, "%f  %f", &x, &y );
            pnt_vec.push_back( vec3d( x, y, 0.0 ) );
        }
    }

    //==== Find Height & Width ====//
    vec3d p0 = pnt_vec[0];
    vec3d pn = pnt_vec[pnt_vec.size() - 1];
    m_Height.Set( max( fabs( p0.y() - pn.y() ), 1.0e-12 ) );

    double max_x = 0;
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x();
        if  ( fabs( y ) > max_x )
        {
            max_x = fabs( x );
        }
    }
    m_Width.Set( max( 2.0 * max_x, 1.0e-12 ) );

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( x, y, 0.0 ) );
    }
    //==== Reflected Pnts ====//
    for ( int i = ( int )pnt_vec.size() - 2 ; i >= 0 ; i-- )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( -x, y, 0.0 ) );
    }
    return true;
}
*/


//==== Read OpenVSP Fuse XSec File ====//
bool FileXSec::ReadXSecFile( FILE* file_id )
{
    char buff[256];
    string line;

    rewind( file_id );
    fgets( buff, 255, file_id );
    line.assign( buff );

    //==== Check For Valid File Type ====//
    size_t found = line.find( "OPENVSP_XSEC_FILE_V1" );
    if ( found == string::npos )
    {
        return false;
    }

    //==== Read Points ====//
    float x, y;
    vector< vec3d > pnt_vec;
    bool more_data = true;
    while( more_data )
    {
        more_data = false;
        if ( fgets( buff, 255, file_id ) )
        {
            sscanf( buff, "%f %f", &x, &y );
            if ( fabs( x ) < 1.0e12 && fabs( y ) <  1.0e12 )
            {
                pnt_vec.push_back( vec3d( x, y, 0.0 ) );
                more_data = true;
            }
        }
    }

    SetPnts( pnt_vec );

    //int num_pnts = (int)pnt_vec.size();

    ////==== Find Height & Width ====//
    //m_Width  = 1.0e-12;
    //m_Height = 1.0e-12;
    //for ( int i = 0 ; i < num_pnts ; i++ )
    //{
    //  for ( int j = 0 ; j < num_pnts ; j++ )
    //  {
    //      double w = fabs( pnt_vec[i].y() - pnt_vec[j].y() );
    //      if ( w > m_Width() )    m_Width = w;
    //      double h = fabs( pnt_vec[i].z() - pnt_vec[j].z() );
    //      if ( h > m_Height() )   m_Height = h;
    //  }
    //}

    ////==== Scale Point By Height & Width ====//
    //m_UnityFilePnts.clear();
    //for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
    //{
    //  double y = pnt_vec[i].y()/m_Width();
    //  double z = pnt_vec[i].z()/m_Height();
    //  m_UnityFilePnts.push_back( vec3d( 0.0, y,  z ) );
    //}

    return true;
}

//==== Set Pnt Vec ====//
void FileXSec::SetPnts( vector< vec3d > & pnt_vec )
{
    // Check for repeated first/last point and close curve.
    double gap = dist( pnt_vec[0], pnt_vec.back() );
    if ( gap > 1e-8 )
    {
        pnt_vec.push_back( pnt_vec[0] );
    }

    int num_pnts = ( int )pnt_vec.size();

    //==== Find Height & Width ====//
    m_Width  = 1.0e-12;
    m_Height = 1.0e-12;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            double w = fabs( pnt_vec[i].x() - pnt_vec[j].x() );
            if ( w > m_Width() )
            {
                m_Width = w;
            }
            double h = fabs( pnt_vec[i].y() - pnt_vec[j].y() );
            if ( h > m_Height() )
            {
                m_Height = h;
            }
        }
    }

    //==== Scale Point By Height & Width ====//
    m_UnityFilePnts.clear();
    for ( int i = 0 ; i < ( int )pnt_vec.size() ; i++ )
    {
        double x = pnt_vec[i].x() / m_Width();
        double y = pnt_vec[i].y() / m_Height();
        m_UnityFilePnts.push_back( vec3d( x, y, 0.0 ) );
    }
}
