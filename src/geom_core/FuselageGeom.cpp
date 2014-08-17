//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "FuselageGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"

#include "APIDefines.h"
using namespace vsp;

//==== Constructor ====//
FuselageGeom::FuselageGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "FuselageGeom";
    m_Type.m_Name = "Fuselage";
    m_Type.m_Type = FUSELAGE_GEOM_TYPE;

    m_Closed = false;

    m_XSecSurf.SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    m_XSecSurf.SetParentContainer( GetID() );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_Length.Init( "Length", "Design", this, 30.0, 1.0e-8, 1.0e12 );
    m_Length.SetDescript( "Length of fuselage" );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_FUSE );

    m_XSecSurf.AddXSec( XS_POINT );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_POINT );

    int j;
    FuseXSec* xs;

    j = 0;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.0;

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.25;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.5;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.75;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 1.0;

}

//==== Destructor ====//
FuselageGeom::~FuselageGeom()
{

}

void FuselageGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void FuselageGeom::UpdateSurf()
{
    m_TessUVec.clear();

    int nxsec = m_XSecSurf.NumXSec();

    //==== Cross Section Curves & joint info ====//
    vector< rib_data_type > rib_vec;
    rib_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

            //==== Set X Limits ====//
#if 0
            // NOTE: This code breaks cross section insertion and deletion because Update()
            //       gets called when a new cross section is created but before the cross section
            //       has been fully initialized. For example, trace the code through a cross section
            //       insert and see that XSecSurf::AddXSec() needs to call ParmChanged() to let
            //       geometry know that the number of u-curves changed because the cross section
            //       count changed and not the Gen tab changing the count. Also, this logic should
            //       probably be in the cross section code (perhaps XSecSurf) so that when the cross
            //       section x-location is changed this is checked.
            int policy = FUSE_MONOTONIC;
            int duct_ile = 2;  // Only needed for FUSE_DUCT
            EnforceOrder( xs, i, duct_ile, policy );
#endif

            xs->SetRefLength( m_Length() );

            bool first = false;
            bool last = false;

            if( i == 0 ) first = true;
            else if( i == (nxsec-1) ) last = true;

            rib_vec[i] = xs->GetRib( first, last );

            if ( i > 0 )
            {
                m_TessUVec.push_back( xs->m_SectTessU() );
            }
        }
    }

    m_MainSurfVec[0].SkinRibs( rib_vec, false );

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetUnsetParms( i, m_MainSurfVec[0] );
        }
    }

    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }
}

void FuselageGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts )
{
    m_SurfVec[indx].Tesselate( m_TessUVec, m_TessW(), pnts, norms, uw_pnts );
}

//==== Compute Rotation Center ====//
void FuselageGeom::ComputeCenter()
{
    m_Center.set_x( m_Length()*m_Origin() );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr FuselageGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr fuselage_node = xmlNewChild( node, NULL, BAD_CAST "FuselageGeom", NULL );
    if ( fuselage_node )
    {
        m_XSecSurf.EncodeXml( fuselage_node );
    }
    return fuselage_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr FuselageGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr fuselage_node = XmlUtil::GetNode( node, "FuselageGeom", 0 );
    if ( fuselage_node )
    {
        m_XSecSurf.DecodeXml( fuselage_node );
    }

    return fuselage_node;
}

//==== Set Active XSec Type ====//
void FuselageGeom::SetActiveXSecType( int type )
{
    XSec* xs = GetXSec( m_ActiveXSec );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetXSecCurve()->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape( m_ActiveXSec, type );

    Update();
}

//==== Cut Active XSec ====//
void FuselageGeom::CutActiveXSec()
{
    m_XSecSurf.CutXSec( m_ActiveXSec );
    Update();
}

//==== Copy Active XSec ====//
void FuselageGeom::CopyActiveXSec()
{
    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void FuselageGeom::PasteActiveXSec()
{
    m_XSecSurf.PasteXSec( m_ActiveXSec );

    Update();
}

//==== Insert XSec ====//
void FuselageGeom::InsertXSec( )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );
    if ( xs )
    {
        InsertXSec( xs->GetXSecCurve()->GetType() );
    }
}

//==== Insert XSec ====//
void FuselageGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    FuseXSec* xs = ( FuseXSec* ) GetXSec( m_ActiveXSec );
    FuseXSec* xs_1 = ( FuseXSec* ) GetXSec( m_ActiveXSec + 1 );

    double x_loc_0 = xs->m_XLocPercent();
    double x_loc_1 = xs_1->m_XLocPercent();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    FuseXSec* inserted_xs = ( FuseXSec* ) GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );
        inserted_xs->m_XLocPercent = ( x_loc_0 + x_loc_1 ) * 0.5;
    }

    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void FuselageGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void FuselageGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
    m_Length *= currentScale;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetScale( currentScale );
        }
    }

    m_LastScale = m_Scale();
}

//==== Drag Parameters ====//
void FuselageGeom::LoadDragFactors( DragFactors& drag_factors )
{
    double max_xsec_area = 0.000000000001;
    for ( int i = 0 ; i < ( int )m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        double a = xs->ComputeArea( 100 );
        if ( a > max_xsec_area )
        {
            max_xsec_area = a;
        }
    }

    double dia = 2.0 * sqrt( max_xsec_area / PI );

    drag_factors.m_Length = m_Length();
    drag_factors.m_MaxXSecArea = max_xsec_area;
    drag_factors.m_LengthToDia = m_Length() / dia;
}

bool FuselageGeom::IsClosed() const
{
    return m_Closed;
}

void FuselageGeom::EnforceOrder( FuseXSec* xs, int indx, int ile, int policy )
{
    if( policy == FUSE_MONOTONIC )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else
        {
            FuseXSec* priorxs = ( FuseXSec* ) m_XSecSurf.FindXSec( indx - 1 );
            FuseXSec* nextxs = ( FuseXSec* ) m_XSecSurf.FindXSec( indx + 1 );
            double lower = priorxs->m_XLocPercent();
            double upper = nextxs->m_XLocPercent();
            xs->m_XLocPercent.SetLowerUpperLimits( lower , upper );
        }
    }
    else if( policy == FUSE_DUCT )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx == ile )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
        }
        else
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
        }

    }
    else if( policy == FUSE_FREE )
    {
        xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
    }
}
