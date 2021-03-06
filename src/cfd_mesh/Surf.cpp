//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Surf
//
//////////////////////////////////////////////////////////////////////

#include "Surf.h"
#include "SCurve.h"
#include "ICurve.h"
#include "ISegChain.h"
#include "Tritri.h"
#include "CfdMeshMgr.h"
#include "StlHelper.h"
#include "SubSurface.h"
#include "SubSurfaceMgr.h"

Surf::Surf()
{
    m_NumU = m_NumW = 0;
    m_GridDensityPtr = 0;
    m_CompID = -1;
    m_UnmergedCompID = -1;
    m_SurfID = -1;
    m_FlipFlag = false;
    m_WakeFlag = false;
    m_TransFlag = false;
    m_SymPlaneFlag = false;
    m_FarFlag = false;
    m_WakeParentSurfID = -1;
    m_Mesh.SetSurfPtr( this );
    m_NumMap = 10;
    m_BaseTag = 1;
}

Surf::~Surf()
{
    int i;
    //==== Delete Patches ====//
    for ( i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        delete m_PatchVec[i];
    }

    //==== Delete SCurves ====//
    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        delete m_SCurveVec[i];
    }
}

void Surf::BuildClean()
{
    int i;
    //==== Delete SCurves ====//
    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        delete m_SCurveVec[i];
    }

    m_SCurveVec.clear();
}

//===== Read Surf From File  =====//
void Surf::ReadSurf( FILE* file_id )
{
    int i, j;
    if ( file_id )
    {
        char buff[256];

        int num_u, num_w, num_u_map, num_w_map, surf_ind;
        double x, y, z, u;
        fgets( buff, 256, file_id );
        sscanf( buff, "%d %d\n", &num_u, &num_w );
        fgets( buff, 256, file_id );
        sscanf( buff, "%d %d\n", &num_u_map, &num_w_map );
        fgets( buff, 256, file_id );
        sscanf( buff, "%d\n", &surf_ind );

        m_VspSurfInd = surf_ind;

        u_to_vspsurf.resize( num_u_map );
        w_to_vspsurf.resize( num_w_map );

        for ( i = 0 ; i < num_u_map; i++ )
        {
            fgets( buff, 256, file_id );
            sscanf( buff, "%lf\n", &u );
            u_to_vspsurf[i] = u;
            u_to_surf[u] = i;
        }

        for ( i = 0 ; i < num_w_map ; i++ )
        {
            fgets( buff, 256, file_id );
            sscanf( buff, "%lf\n", &u );
            w_to_vspsurf[i] = u;
            w_to_surf[u] = i;
        }

        vector< vector< vec3d > > pnts;
        pnts.resize( num_u );
        for ( i = 0 ; i < num_u ; i++ )
        {
            pnts[i].resize( num_w );
        }

        for ( i = 0 ; i < num_u ; i++ )
            for ( j = 0 ; j < num_w ; j++ )
            {
                fgets( buff, 256, file_id );
                sscanf( buff, "%lf %lf %lf\n", &x, &y, &z );
                pnts[i][j].set_xyz( x, y, z );
            }
        LoadControlPnts( pnts );
    }
}

void Surf::LoadControlPnts( vector< vector< vec3d > > & control_pnts )
{
    int i, j;
    assert( control_pnts.size() >= 4 );
    assert( control_pnts[0].size() >= 4 );
    m_Pnts = control_pnts;

    m_NumU = m_Pnts.size();
    m_NumW = m_Pnts[0].size();
    m_MaxU = ( m_NumU - 1 ) / 3;
    m_MaxW = ( m_NumW - 1 ) / 3;

    //==== Load Patch Vec ====//
    m_BBox.Reset();
    for ( i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        delete m_PatchVec[i];
    }
    m_PatchVec.clear();

    for ( i = 0 ; i < m_NumU - 1 ; i += 3 )
    {
        for ( j = 0 ; j < m_NumW - 1 ; j += 3 )
        {
            SurfPatch* patch = new SurfPatch();
            for ( int pi = 0 ; pi < 4 ; pi++ )
                for ( int pj = 0 ; pj < 4 ; pj++ )
                {
                    m_BBox.Update( m_Pnts[pi + i][pj + j] );
                    patch->put_pnt( pi, pj, m_Pnts[pi + i][pj + j] );

                    patch->set_u_min_max( i / 3, i / 3 + 1.0 );
                    patch->set_w_min_max( j / 3, j / 3 + 1.0 );
                }
            patch->set_surf_ptr( this );
            patch->compute_bnd_box();
            m_PatchVec.push_back( patch );
        }
    }
}

//===== Compute Blending Functions  =====//
void Surf::BlendFuncs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    //==== Compute All Blending Functions ====//
    double uu = u * u;
    double one_u = 1.0 - u;
    double one_u_sqr = one_u * one_u;

    F1 = one_u * one_u_sqr;
    F2 = 3.0 * u * one_u_sqr;
    F3 = 3.0 * uu * one_u;
    F4 = uu * u;
}

void Surf::BlendDerivFuncs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    double uu    = u * u;
    double one_u = 1.0 - u;
    F1 = -3.0 * one_u * one_u;
    F2 = 3.0 - 12.0 * u + 9.0 * uu;
    F3 = 6.0 * u - 9.0 * uu;
    F4 = 3.0 * uu;
}

void Surf::BlendDeriv2Funcs( double u, double& F1, double& F2, double& F3, double& F4 )
{
    F1 = 6.0 - 6.0 * u;
    F2 = -12.0 + 18.0 * u;
    F3 = 6.0 - 18.0 * u;
    F4 = 6.0 * u;
}

//===== Compute Point On Surf Given  U W (Between 0 1 ) =====//
vec3d Surf::CompPnt01( double u, double w )
{
    return CompPnt( u * m_MaxU, w * m_MaxW );
}


//===== Compute Tangent In U Direction   =====//
vec3d Surf::CompTanU01( double u01, double w01 )
{
    return CompTanU( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Tangent In W Direction   =====//
vec3d Surf::CompTanW01( double u01, double w01 )
{
    return CompTanW( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Second Derivative U,U   =====//
vec3d Surf::CompTanUU01( double u01, double w01 )
{
    return CompTanUU( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Second Derivative W,W   =====//
vec3d Surf::CompTanWW01( double u01, double w01 )
{
    return CompTanWW( u01 * m_MaxU, w01 * m_MaxW );
}

//===== Compute Second Derivative U,W   =====//
vec3d Surf::CompTanUW01( double u01, double w01 )
{
    return CompTanUW( u01 * m_MaxU, w01 * m_MaxW );
}


//===== Compute Second Derivative U,U   =====//
vec3d Surf::CompTanUU( double u, double w )
{
    return CompBez( u, w, &BlendDeriv2Funcs, &BlendFuncs );
}

//===== Compute Second Derivative W,W   =====//
vec3d Surf::CompTanWW( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendDeriv2Funcs );
}

//===== Compute Second Derivative U,W   =====//
vec3d Surf::CompTanUW( double u, double w )
{
    return CompBez( u, w, &BlendDerivFuncs, &BlendDerivFuncs );
}

//===== Compute Tangent In U Direction   =====//
vec3d Surf::CompTanU( double u, double w )
{
    return CompBez( u, w, &BlendDerivFuncs, &BlendFuncs );
}

//===== Compute Tangent In W Direction   =====//
vec3d Surf::CompTanW( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendDerivFuncs );
}

//===== Compute Point On Surf Given  U W =====//
vec3d Surf::CompPnt( double u, double w )
{
    return CompBez( u, w, &BlendFuncs, &BlendFuncs );
}

//===== Generic Bezier Surface Calculation  =====//
vec3d Surf::CompBez( double u, double w,
                     void ( *uBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ),
                     void ( *wBlendFun )( double u, double& F1, double& F2, double& F3, double& F4 ) )
{
    vec3d pnt;

    if ( m_NumU < 4 || m_NumW < 4 )
    {
        return pnt;
    }

    if ( u < 0.0 )
    {
        u = 0.0;
    }
    if ( w < 0.0 )
    {
        w = 0.0;
    }

    double F1u, F2u, F3u, F4u;
    double F1w, F2w, F3w, F4w;

    int trunc_u = ( int )u;
    int u_ind = trunc_u * 3;
    if ( u_ind >= m_NumU - 1 )
    {
        trunc_u = trunc_u - 1;
        u_ind = m_NumU - 4;
    }
    uBlendFun( u - ( double )trunc_u, F1u, F2u, F3u, F4u );

    int trunc_w = ( int )w;
    int w_ind = trunc_w * 3;
    if ( w_ind >= m_NumW - 1 )
    {
        trunc_w = trunc_w - 1;
        w_ind = m_NumW - 4;
    }
    wBlendFun( w - ( double )trunc_w, F1w, F2w, F3w, F4w );

    pnt =
        ( ( m_Pnts[u_ind][w_ind] * F1u     + m_Pnts[u_ind + 1][w_ind] * F2u +
            m_Pnts[u_ind + 2][w_ind] * F3u   + m_Pnts[u_ind + 3][w_ind] * F4u ) * F1w ) +
        ( ( m_Pnts[u_ind][w_ind + 1] * F1u   + m_Pnts[u_ind + 1][w_ind + 1] * F2u +
            m_Pnts[u_ind + 2][w_ind + 1] * F3u + m_Pnts[u_ind + 3][w_ind + 1] * F4u ) * F2w ) +
        ( ( m_Pnts[u_ind][w_ind + 2] * F1u  + m_Pnts[u_ind + 1][w_ind + 2] * F2u +
            m_Pnts[u_ind + 2][w_ind + 2] * F3u + m_Pnts[u_ind + 3][w_ind + 2] * F4u ) * F3w ) +
        ( ( m_Pnts[u_ind][w_ind + 3] * F1u  + m_Pnts[u_ind + 1][w_ind + 3] * F2u +
            m_Pnts[u_ind + 2][w_ind + 3] * F3u + m_Pnts[u_ind + 3][w_ind + 3] * F4u ) * F4w );

    return pnt;
}

//===== Compute Surface Curvature Metrics Given  U W =====//
void Surf::CompCurvature( double u, double w, double& k1, double& k2, double& ka, double& kg )
{
    double tol = 1e-10;

    double bump = 1e-3;

    // First derivative vectors
    vec3d S_u = CompTanU( u, w );
    vec3d S_w = CompTanW( u, w );

    double E = dot( S_u, S_u );
    double G = dot( S_w, S_w );

    if( E < tol && G < tol )
    {
        double umid = m_MaxU / 2.0;
        double wmid = m_MaxW / 2.0;

        u = u + ( umid - u ) * bump;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( E < tol ) // U direction degenerate
    {
        double wmid = m_MaxW / 2.0;
        w = w + ( wmid - w ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }
    else if( G < tol ) // W direction degenerate
    {
        double umid = m_MaxU / 2.0;
        u = u + ( umid - u ) * bump;

        S_u = CompTanU( u, w );
        S_w = CompTanW( u, w );

        E = dot( S_u, S_u );
        G = dot( S_w, S_w );
    }

    // Second derivative vectors
    vec3d S_uu = CompTanUU( u, w );
    vec3d S_uw = CompTanUW( u, w );
    vec3d S_ww = CompTanWW( u, w );

    // Unit normal vector
    vec3d Q = cross( S_u, S_w );
    Q.normalize();

    double F = dot( S_u, S_w );

    double L = dot( S_uu, Q );
    double M = dot( S_uw, Q );
    double N = dot( S_ww, Q );

    // Mean curvature
    ka = ( E * N + G * L - 2.0 * F * M ) / ( 2.0 * ( E * G - F * F ) );

    // Gaussian curvature
    kg = ( L * N - M * M ) / ( E * G - F * F );

    double b = sqrt( ka * ka - kg );

    // Principal curvatures
    double kmax = ka + b;
    double kmin = ka - b;

    // Ensure k1 has largest magnitude
    if( fabs( kmax ) > fabs( kmin ) )
    {
        k1 = kmax;
        k2 = kmin;
    }
    else
    {
        k1 = kmin;
        k2 = kmax;
    }
}

double Surf::TargetLen( double u, double w, double gap, double radfrac )
{
    double k1, k2, ka, kg;

    double tol = 1e-6;
    double len = numeric_limits<double>::max( );
    double r = -1.0;

    double glen = numeric_limits<double>::max( );
    double nlen = numeric_limits<double>::max( );

    CompCurvature( u, w, k1, k2, ka, kg );

    if( fabs( k1 ) < tol ) // If zero curvature
    {
        double du = -tol;
        if( u < tol )
        {
            du = tol;
        }
        double dw = -tol;
        if( w < tol )
        {
            dw = tol;
        }
        // Check point offset inside the surface.
        CompCurvature( u + du, w + dw, k1, k2, ka, kg );
    }

    if( fabs( k1 ) > tol )
    {
        // Tightest radius of curvature
        r = 1.0 / fabs( k1 );

        if( r > gap )
        {
            // Pythagorean thm. to calculate edge length to match gap given radius.
            glen = 2.0 * sqrt( 2.0 * r * gap - gap * gap );
        }
        else
        {
            glen = 2.0 * gap;
        }

        // Radius fraction calculated elsewhere based on desired number of circle segments.
        // This calculation can give unboundedly small edge lengths.  The minimum edge length
        // is a required control to prevent this.
        nlen = r * radfrac;

        len = min( glen, nlen );
    }
    return len;
}

void Surf::BuildTargetMap( vector< MapSource* > &sources, int sid )
{
    int npatchu = ( m_NumU - 1 ) / 3;
    int npatchw = ( m_NumW - 1 ) / 3;

    int nmapu = npatchu * ( m_NumMap - 1 ) + 1;
    int nmapw = npatchw * ( m_NumMap - 1 ) + 1;

    // Initialize map matrix dimensions
    m_SrcMap.resize( nmapu );
    for( int i = 0; i < nmapu ; i++ )
    {
        m_SrcMap[i].resize( nmapw );
    }

    bool limitFlag = false;
    if ( m_FarFlag )
    {
        limitFlag = true;
    }
    if ( m_SymPlaneFlag )
    {
        limitFlag = true;
    }

    // Loop over surface evaluating source strength and curvature
    for( int i = 0; i < nmapu ; i++ )
    {
        double u = ( 1.0 * i ) / ( m_NumMap - 1 );
        for( int j = 0; j < nmapw ; j++ )
        {
            double w = ( 1.0 * j ) / ( m_NumMap - 1 );

            double len = numeric_limits<double>::max( );

            // apply curvature based limits
            double curv_len = TargetLen( u, w, m_GridDensityPtr->GetMaxGap( limitFlag ), m_GridDensityPtr->GetRadFrac( limitFlag ) );
            len = min( len, curv_len );

            // apply minimum edge length as safety on curvature
            len = max( len, m_GridDensityPtr->m_MinLen() );

            // apply sources
            vec3d p = CompPnt( u, w );
            double grid_len = m_GridDensityPtr->GetTargetLen( p, limitFlag );
            len = min( len, grid_len );

            // finally check max size
            len = min( len, m_GridDensityPtr->GetBaseLen( limitFlag ) );

            MapSource ms = MapSource( p, len, sid );
            m_SrcMap[i][j] = ms;
            sources.push_back( &( m_SrcMap[i][j] ) );
        }
    }
}

void Surf::SetSymPlaneFlag( bool flag )
{
    m_SymPlaneFlag = flag;

    // Refine background map for symmetry plane.
    if( m_SymPlaneFlag )
    {
        m_NumMap = 100;
    }
    else
    {
        m_NumMap = 10;
    }
}


bool indxcompare( const pair < double, pair < int, int > > &a, const pair < double, pair < int, int > > &b )
{
    return ( a.first < b.first );
}

void Surf::WalkMap( pair< int, int > ijstart, int kstart, pair< int, int > ijcurrent )
{

    int iadd[] = { -1, 1,  0, 0 };
    int jadd[] = {  0, 0, -1, 1 };

    double grm1 = m_GridDensityPtr->m_GrowRatio() - 1.0;

    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int istart = ijstart.first;
    int jstart = ijstart.second;

    MapSource srcstart = m_SrcMap[istart][jstart];
    vec3d p = srcstart.m_pt;
    double str = srcstart.m_str;

    int icur = ijcurrent.first;
    int jcur = ijcurrent.second;

    for( int i = 0; i < 4; i++ )
    {
        int itarget = icur + iadd[i];
        int jtarget = jcur + jadd[i];

        if( itarget < nmapu && itarget >= 0 && jtarget < nmapw && jtarget >= 0 )
        {

            if( m_SrcMap[ itarget ][ jtarget ].m_maxvisited < kstart )
            {
                m_SrcMap[ itarget ][ jtarget ].m_maxvisited = kstart;
                vec3d p2 = m_SrcMap[ itarget ][ jtarget ].m_pt;
                double r = ( p2 - p ).mag();
                double targetstr = str + r * grm1;
                if( m_SrcMap[ itarget ][ jtarget ].m_str > targetstr )
                {
                    // Mark dominated as progress is made
                    m_SrcMap[ itarget ][ jtarget ].m_dominated = true;
                    m_SrcMap[ itarget ][ jtarget ].m_str = targetstr;
                    pair< int, int > ijtarget( itarget, jtarget );
                    WalkMap( ijstart, kstart, ijtarget );
                }
            }
        }
    }
}

void Surf::WalkMap( int istart, int jstart, int icurrent, int jcurrent )
{
    static const int iadd[] = { -1, 1,  0, 0 };
    static const int jadd[] = {  0, 0, -1, 1 };

    for( int i = 0; i < 4; i++ )
    {
        static int itarget;
        itarget = icurrent + iadd[i];
        static int jtarget;
        jtarget = jcurrent + jadd[i];

        if( itarget < m_SrcMap.size() && itarget >= 0 && jtarget < m_SrcMap[0].size() && jtarget >= 0 )
        {
            static double targetstr;
            targetstr = m_SrcMap[istart][jstart].m_str +
                    ( m_SrcMap[ itarget ][ jtarget ].m_pt - m_SrcMap[istart][jstart].m_pt ).mag() *
                    (m_GridDensityPtr->m_GrowRatio() - 1.0);
            if( m_SrcMap[ itarget ][ jtarget ].m_str > targetstr )
            {
                m_SrcMap[ itarget ][ jtarget ].m_str = targetstr;
                WalkMap( istart, jstart, itarget, jtarget );
            }
        }
    }
}

void Surf::LimitTargetMap()
{
    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int nmap = nmapu * nmapw;

    // Create size sortable index of array i,j coordinates
    vector< pair < double, pair < int, int > > > index;
    index.resize( nmap );

    int k = 0;
    for( int i = 0; i < nmapu ; i++ )
    {
        for( int j = 0; j < nmapw ; j++ )
        {
            pair< int, int > ij( i, j );
            pair < double, pair < int, int > > id( m_SrcMap[i][j].m_str, ij );
            index[k] = id;
            k++;
            m_SrcMap[i][j].m_maxvisited = -1;  // Reset traversal limiter.
        }
    }

    // Sort index
    std::sort( index.begin(), index.end(), indxcompare );

    // Start from smallest
    for( int k = 0; k < nmap; k++ )
    {
        pair< int, int > ij = index[k].second;
        int i = ij.first;
        int j = ij.second;

        MapSource src = m_SrcMap[i][j];

        // Recursively limit from small to large (skip if dominated)
        if( !src.m_dominated )
        {
            WalkMap( ij, k, ij );
        }
    }
}

void Surf::LimitTargetMap( MSCloud &es_cloud, MSTree &es_tree, double minmap )
{
    double grm1 = m_GridDensityPtr->m_GrowRatio() - 1.0;

    double tmin = min( minmap, es_cloud.sources[0]->m_str );

    SearchParams params;
    params.sorted = false;

    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    // Loop over surface evaluating source strength and curvature
    for( int i = 0; i < nmapu ; i++ )
    {
//      double u = ( 1.0 * i ) / ( m_NumMap - 1 );
        for( int j = 0; j < nmapw ; j++ )
        {
//          double w = ( 1.0 * j ) / ( m_NumMap - 1 );

            double *query_pt = m_SrcMap[i][j].m_pt.v;

            double t = m_SrcMap[i][j].m_str;
            double torig = t;

            double rmax = ( t - tmin ) / grm1;
            if( rmax > 0.0 )
            {
                double r2max = rmax * rmax;

                MSTreeResults es_matches;

                int nMatches = es_tree.radiusSearch( query_pt, r2max, es_matches, params );

                for ( int k = 0; k < nMatches; k++ )
                {
                    int imatch = es_matches[k].first;
                    double r = sqrt( es_matches[k].second );

                    double str = es_cloud.sources[imatch]->m_str;

                    double ts = str + grm1 * r;
                    t = min( t, ts );
                }
                if( t < torig )
                {
                    m_SrcMap[i][j].m_str = t;
                    WalkMap( i, j, i, j );
                }
            }
        }
    }
}

double Surf::InterpTargetMap( double u, double w )
{
    int i, j;
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );

    double ti = m_SrcMap[i][j].m_str + fracj * ( m_SrcMap[i][j + 1].m_str - m_SrcMap[i][j].m_str );
    double tip1 = m_SrcMap[i + 1][j].m_str + fracj * ( m_SrcMap[i + 1][j + 1].m_str - m_SrcMap[i + 1][j].m_str );

    double t = ti + fraci * ( tip1 - ti );
    return t;
}

void Surf::UWtoTargetMapij( double u, double w, int &i, int &j, double &fraci, double &fracj )
{
    int imax = m_SrcMap.size() - 1;
    double di = u * ( m_NumMap - 1 );
    i = ( int ) di;
    fraci = di - i;
    if( i >= imax )
    {
        i = imax - 1;
        fraci = 1.0;
    }

    int jmax = m_SrcMap[0].size() - 1;
    double dj = w * ( m_NumMap - 1 );
    j = ( int ) dj;
    fracj = dj - j;
    if( j >= jmax )
    {
        j = jmax - 1;
        fracj = 1.0;
    }
}

void Surf::UWtoTargetMapij( double u, double w, int &i, int &j )
{
    double fraci, fracj;
    UWtoTargetMapij( u, w, i, j, fraci, fracj );
}

void Surf::ApplyES( vec3d uw, double t )
{
    double grm1 = m_GridDensityPtr->m_GrowRatio() - 1.0;
    int nmapu = m_SrcMap.size();
    int nmapw = m_SrcMap[0].size();

    int ibase, jbase;
    double u = uw.x();
    double w = uw.y();
    UWtoTargetMapij( u, w, ibase, jbase );

    vec3d p = CompPnt( u, w );

    int iadd[] = { 0, 1, 0, 1 };
    int jadd[] = { 0, 0, 1, 1 };

    for( int i = 0; i < 4; i++ )
    {
        int itarget = ibase + iadd[i];
        int jtarget = jbase + jadd[i];

        if( itarget < nmapu && itarget >= 0 && jtarget < nmapw && jtarget >= 0 )
        {
            vec3d p2 = m_SrcMap[ itarget ][ jtarget ].m_pt;
            double r = ( p2 - p ).mag();
            double targetstr = t + r * grm1;
            if( m_SrcMap[ itarget ][ jtarget ].m_str > targetstr )
            {
                m_SrcMap[ itarget ][ jtarget ].m_str = targetstr;
                WalkMap( itarget, jtarget, itarget, jtarget );
            }
        }
    }
}

vec2d Surf::ClosestUW( vec3d & pnt, double guess_u, double guess_w, double guess_del_u, double guess_del_w, double tol )
{
    double u = guess_u;
    double w = guess_w;
    double del_u = guess_del_u;
    double del_w = guess_del_w;
    double dist = dist_squared( pnt, CompPnt( u, w ) );

    if ( dist < tol )
    {
        return vec2d( u, w );
    }

    int iter = 20;
    while ( iter > 0 )
    {
        double u_plus  = u + del_u;
        double u_minus = u - del_u;
        if ( u_plus  > m_MaxU )
        {
            u_plus  = m_MaxU;
            del_u *= 0.25;
        }
        if ( u_minus < 0 )
        {
            u_minus = 0.0;
            del_u *= 0.25;
        }

        double dist_plus_u  = dist_squared( pnt, CompPnt( u_plus, w ) );
        double dist_minus_u = dist_squared( pnt, CompPnt( u_minus, w ) );
        if ( dist_plus_u < dist )
        {
            u = u_plus;
            del_u *= 2.0;
            dist = dist_plus_u;
        }
        else if ( dist_minus_u < dist )
        {
            u = u_minus;
            del_u *= 2.0;
            dist = dist_minus_u;
        }
        else
        {
            del_u *= 0.5;
            iter--;
        }

        double w_plus = w + del_w;
        double w_minus = w - del_w;
        if ( w_plus > m_MaxW )
        {
            w_plus = m_MaxW;
            del_w *= 0.25;
        }
        if ( w_minus < 0 )
        {
            w_minus = 0;
            del_w *= 0.25;
        }

        double dist_plus_w  = dist_squared( pnt, CompPnt( u, w_plus ) );
        double dist_minus_w = dist_squared( pnt, CompPnt( u, w_minus ) );
        if ( dist_plus_w < dist )
        {
            w = w_plus;
            del_w *= 2.0;
            dist = dist_plus_w;
        }
        else if ( dist_minus_w < dist )
        {
            w = w_minus;
            del_w *= 2.0;
            dist = dist_minus_w;
        }
        else
        {
            del_w *= 0.5;
            iter--;
        }
    }

    return vec2d( u, w );
}

vec2d Surf::ClosestUW( vec3d & pnt_in, double guess_u, double guess_w )
{
    int Max_Iter  = 10;
    double UW_Tol = 1.0e-14;
    double norm_uw[2];
    double del_uw[2];
    vec3d guess_pnt;

//  double orig_d = dist_squared( pnt_in, CompPnt( guess_u, guess_w ) );

    //==== Normalize uw Values Between 0 and 1 ====//
    norm_uw[0] = guess_u / m_MaxU;
    norm_uw[1] = guess_w / m_MaxW;

    //===== Loop Until U and W Stops Changing or Max Iterations is Hit  =====//
    int cnt = 0;
    int stop_flag = false;
    while ( !stop_flag )
    {
        guess_pnt = CompPnt01( norm_uw[0], norm_uw[1] );

        //==== Find Delta U and W Values ====//
        CompDeltaUW( pnt_in, guess_pnt, norm_uw, del_uw );

        norm_uw[0] += del_uw[0] / m_MaxU;
        norm_uw[1] -= del_uw[1] / m_MaxW;

        if ( norm_uw[0] < 0.0 )
        {
            norm_uw[0] = 0.0;
        }
        else if ( norm_uw[0] > 1.0 )
        {
            norm_uw[0] = 1.0;
        }
        if ( norm_uw[1] < 0.0 )
        {
            norm_uw[1] = 0.0;
        }
        else if ( norm_uw[1] > 1.0 )
        {
            norm_uw[1] = 1.0;
        }

        //==== Test To Stop Iteration ====//
        if ( ( fabs( del_uw[0] ) + fabs( del_uw[1] ) ) < UW_Tol )
        {
            stop_flag = true;
        }
        else if ( cnt > Max_Iter )
        {
            stop_flag = true;
        }
        else
        {
            cnt++;
        }
    }

    //==== Convert uw Values Back to Original Space ====//
    vec2d uw;
    uw[0] = norm_uw[0] * m_MaxU;
    uw[1] = norm_uw[1] * m_MaxW;
//
//  double new_d = dist_squared( pnt_in, guess_pnt );
//  if ( new_d > orig_d )
//  {
//      //===== Brute Force ====//
//      uw = ClosestUW( pnt_in, guess_u, guess_w, 0.001, 0.001, 0.00001 );
//  }

    //==== Clamp Values At Bounds  ====//
    if      ( uw[0] < 0.0 )
    {
        uw[0] = 0.0;
    }
    else if ( uw[0] > m_MaxU )
    {
        uw[0] = m_MaxU;
    }

    if      ( uw[1] < 0.0 )
    {
        uw[1] = 0.0;
    }
    else if ( uw[1] > m_MaxW )
    {
        uw[1] = m_MaxW;
    }

    return uw;
}


//===== Compute Point On Patch  =====//
void Surf::CompDeltaUW( vec3d& pnt_in, vec3d& guess_pnt, double norm_uw[2], double delta_uw[2] )
{
    vec3d tan_u = CompTanU01( norm_uw[0], norm_uw[1] );
    vec3d tan_w = CompTanW01( norm_uw[0], norm_uw[1] );

    vec3d dist_vec = guess_pnt - pnt_in;

    vec3d A = cross( tan_w, dist_vec );
    vec3d B = cross( tan_u, dist_vec );
    vec3d norm = cross( tan_u, tan_w );

    double N = dot( norm, norm );

    if( fabs( N ) > DBL_EPSILON )
    {
        delta_uw[0] = dot( A, norm ) / N;
        delta_uw[1] = dot( B, norm ) / N;
    }
    else
    {
        delta_uw[0] = 0.0;
        delta_uw[1] = 0.0;
    }
}

bool Surf::LessThanY( double val )
{
    for ( int i = 0 ; i < m_NumU ; i++ )
        for ( int j = 0 ; j < m_NumW ; j++ )
        {
            if ( m_Pnts[i][j][1] > val )
            {
                return false;
            }
        }
    return true;


}

bool Surf::OnYZeroPlane()
{
    double tol = 0.0000001;
    bool onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve 1 w = 0
    {
        if ( fabs( m_Pnts[i][0][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve 2 w = max
    {
        if ( fabs( m_Pnts[i][m_NumW - 1][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve 3 u = 0
    {
        if ( fabs( m_Pnts[0][i][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    onPlaneFlag = true;
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve 4 u = max
    {
        if ( fabs( m_Pnts[m_NumU - 1][i][1] ) > tol )
        {
            onPlaneFlag = false;
        }
    }
    if ( onPlaneFlag )
    {
        return onPlaneFlag;
    }

    return false;
}

bool Surf::PlaneAtYZero()
{
    double tol = 0.000001;
    for ( int i = 0 ; i < m_NumU ; i++ )
        for ( int j = 0 ; j < m_NumW ; j++ )
        {
            double yval = m_Pnts[i][j][1];
            if ( fabs( yval ) > tol )
            {
                return false;
            }
        }
    return true;
}

void Surf::FindBorderCurves()
{
    double degen_tol = 0.000001;

    //==== Load 4 Border Curves if Not Degenerate ====//
    SCurve* scrv;
    double max_u = ( m_NumU - 1 ) / 3.0;
    double max_w = ( m_NumW - 1 ) / 3.0;

    vector< vec3d > pnts;
    pnts.resize( 4 );

    pnts[0].set_xyz( 0,         0, 0 );         // Inc U
    pnts[1].set_xyz( 0.25 * max_u, 0, 0 );
    pnts[2].set_xyz( 0.75 * max_u, 0, 0 );
    pnts[3].set_xyz( max_u,     0, 0 );

    scrv = new SCurve( this );
    scrv->SetBezierControlPnts( pnts );

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( max_u,       0, 0 );       // Inc W
    pnts[1].set_xyz( max_u, 0.25 * max_w, 0 );
    pnts[2].set_xyz( max_u, 0.75 * max_w, 0 );
    pnts[3].set_xyz( max_u,     max_w, 0 );

    scrv = new SCurve( this );
    scrv->SetBezierControlPnts( pnts );

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( max_u,     max_w, 0 );         // Dec U
    pnts[1].set_xyz( 0.75 * max_u, max_w, 0 );
    pnts[2].set_xyz( 0.25 * max_u, max_w, 0 );
    pnts[3].set_xyz( 0,         max_w, 0 );

    scrv = new SCurve( this );
    scrv->SetBezierControlPnts( pnts );

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }

    pnts[0].set_xyz( 0, max_w,   0 );           // Dec W
    pnts[1].set_xyz( 0, 0.75 * max_w, 0 );
    pnts[2].set_xyz( 0, 0.25 * max_w, 0 );
    pnts[3].set_xyz( 0, 0,       0 );

    scrv = new SCurve( this );
    scrv->SetBezierControlPnts( pnts );

    if ( scrv->Length( 10 ) > degen_tol )
    {
        m_SCurveVec.push_back( scrv );
    }
    else
    {
        delete scrv;
    }
}

void Surf::LoadSCurves( vector< SCurve* > & scurve_vec )
{
    for ( int i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        scurve_vec.push_back( m_SCurveVec[i] );
    }
}

void Surf::BuildGrid()
{
    int i;
    vector< vec3d > uw_border;

    for ( i = 0 ; i < ( int )m_SCurveVec.size() ; i++ )
    {
        vector< vec3d > suw_vec = m_SCurveVec[i]->GetUWTessPnts();

        for ( int j = 0 ; j < ( int )suw_vec.size() ; j++ )
        {
            if ( uw_border.size() )         // Check For Duplicate Points
            {
                double d = dist( uw_border.back(), suw_vec[j] );
                if ( d > 0.0000001 )
                {
                    uw_border.push_back( suw_vec[j] );
                }
            }
            else
            {
                uw_border.push_back( suw_vec[j] );
            }
        }
    }

    m_Mesh.TriangulateBorder( uw_border );

    //for ( i = 0 ; i < 10 ; i++ )
    //{
    //  m_Mesh.Remesh(100);

    //}

}

void Surf::WriteSTL( const char* filename )
{
    m_Mesh.WriteSTL( filename );
}

void Surf::Intersect( Surf* surfPtr )
{
    int i;

    if ( surfPtr->GetCompID() == m_CompID )
    {
        return;
    }

    if ( !Compare( m_BBox, surfPtr->GetBBox() ) )
    {
        return;
    }
    if ( BorderCurveOnSurface( surfPtr ) )
    {
        return;
    }
    if ( surfPtr->BorderCurveOnSurface( this ) )
    {
        return;
    }

    vector< SurfPatch* > otherPatchVec = surfPtr->GetPatchVec();
    for ( i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
        if ( Compare( *m_PatchVec[i]->get_bbox(), surfPtr->GetBBox() ) )
        {
            for ( int j = 0 ; j < ( int )otherPatchVec.size() ; j++ )
            {
                if ( Compare( *m_PatchVec[i]->get_bbox(), *otherPatchVec[j]->get_bbox() ) )
                {
                    intersect( *m_PatchVec[i], *otherPatchVec[j], 0 );
                    m_PatchVec[i]->draw_flag = true;
                    otherPatchVec[j]->draw_flag = true;
                }
            }
        }
}

void Surf::IntersectLineSeg( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
    BndBox line_box;
    line_box.Update( p0 );
    line_box.Update( p1 );

    if ( !Compare( line_box, m_BBox ) )
    {
        return;
    }

    for ( int i = 0 ; i < ( int )m_PatchVec.size() ; i++ )
    {
        m_PatchVec[i]->IntersectLineSeg( p0, p1, line_box, t_vals );
    }
}

void Surf::IntersectLineSegMesh( vec3d & p0, vec3d & p1, vector< double > & t_vals )
{
    BndBox line_box;
    line_box.Update( p0 );
    line_box.Update( p1 );

    if ( !Compare( line_box, m_BBox ) )
    {
        return;
    }

    double tparm, uparm, vparm;
    list< Tri* >::iterator t;
    list <Tri*> triList = m_Mesh.GetTriList();

    vec3d dir = p1 - p0;

    for ( t = triList.begin() ; t != triList.end(); t++ )
    {
        int iFlag = intersect_triangle( p0.v, dir.v,
                                        ( *t )->n0->pnt.v, ( *t )->n1->pnt.v, ( *t )->n2->pnt.v, &tparm, &uparm, &vparm );

        if ( iFlag && tparm > 0.0 )
        {
            //==== Find If T is Already Included ====//
            int dupFlag = 0;
            for ( int j = 0 ; j < ( int )t_vals.size() ; j++ )
            {
                if ( fabs( tparm - t_vals[j] ) < 0.0000001 )
                {
                    dupFlag = 1;
                    break;
                }
            }

            if ( !dupFlag )
            {
                t_vals.push_back( tparm );
            }
        }
    }
}

bool Surf::BorderCurveOnSurface( Surf* surfPtr )
{
    bool retFlag = false;
    double tol = 1.0e-05;

    vector< SCurve* > border_curves;
    surfPtr->LoadSCurves( border_curves );

    for ( int i = 0 ; i < ( int )border_curves.size() ; i++ )
    {
        vector< vec3d > control_pnts;
        border_curves[i]->ExtractBorderControlPnts( control_pnts );

        int num_pnts_on_surf = 0;
        for ( int c = 0 ; c < ( int )control_pnts.size() ; c++ )
        {
            vec2d uw = ClosestUW( control_pnts[c], m_MaxU / 2.0, m_MaxW / 2.0 );

            vec3d p = CompPnt( uw[0], uw[1] );

            double d = dist( control_pnts[c], p );

            if ( d < tol )
            {
                num_pnts_on_surf++;
                retFlag = true;
            }
        }
        if ( num_pnts_on_surf > 2 )
        {
            //==== If Surface Add To List ====//
            CfdMeshMgr.AddPossCoPlanarSurf( this, surfPtr );
            PlaneBorderCurveIntersect( surfPtr, border_curves[i] );
        }
    }

    return retFlag;
}

void Surf::PlaneBorderCurveIntersect( Surf* surfPtr, SCurve* brdPtr )
{
    bool repeat_curve = false;
    bool null_ICurve = false;
    if ( brdPtr->GetICurve() != NULL )
    {
        for ( int j = 0 ; j < (int)m_SCurveVec.size() ; j++ )
        {
            if ( brdPtr->GetICurve() == m_SCurveVec[j]->GetICurve() )
            {
                repeat_curve = true;
            }
        }
    }
    else
    {
        null_ICurve = true;
    }

    if ( !repeat_curve )
    {
        SCurve* pSCurve = new SCurve;
        SCurve* bSCurve = new SCurve;
        ICurve* pICurve = new ICurve;

        ICurve* bICurve = pICurve;
        ICurve* obICurve = brdPtr->GetICurve();

        vector< vec3d > UWCrv_bordercurve;
        vector< vec3d > UWCrv_plane;

        UWCrv_plane.resize( 4 );

        vector< ICurve* > ICurves = CfdMeshMgr.GetICurveVec();
        int ICurveVecIndex;

        for ( int i = 0 ; i < brdPtr->GetUWCrv().get_num_control_pnts() ; i++ )
        {
            UWCrv_bordercurve.push_back( brdPtr->GetUWCrv().get_pnt( i ) );
        }

        for ( int i = 0 ; i < (int)UWCrv_bordercurve.size() ; i++ )
        {
            vec3d pnt = surfPtr->CompPnt( UWCrv_bordercurve[i].x(), UWCrv_bordercurve[i].y() );
            vec2d uw = ClosestUW( pnt, m_MaxU / 2, m_MaxW / 2 );
            UWCrv_plane[i].set_xyz( uw.x(), uw.y(), 0 );
        }

        pSCurve->SetBezierControlPnts( UWCrv_plane );

        pICurve->m_SCurve_A = brdPtr;
        pICurve->m_SCurve_B = pSCurve;
        pICurve->m_PlaneBorderIntersectFlag = true;
        pSCurve->SetSurf( this );
        pSCurve->SetICurve( pICurve );

        bICurve->m_SCurve_A = pSCurve;
        bICurve->m_SCurve_B = brdPtr;
        bICurve->m_PlaneBorderIntersectFlag = true;
        bSCurve->SetSurf( surfPtr );
        bSCurve->SetICurve( bICurve );

        brdPtr->SetICurve( bICurve );

        if ( !null_ICurve )
        {
            ICurveVecIndex = distance( ICurves.begin(), find( ICurves.begin(), ICurves.end(), obICurve ) );
            if ( ICurveVecIndex < (int)ICurves.size() )
            {
                CfdMeshMgr.SetICurveVec( brdPtr->GetICurve(), ICurveVecIndex );
            }
        }
        else
        {
            for ( int i = 0 ; i < (int)ICurves.size() ; i++ )
            {
                if ( ICurves[i]->m_SCurve_A == brdPtr && ICurves[i]->m_SCurve_B == NULL )
                {
                    ICurves[i]->m_SCurve_B = pSCurve;
                    ICurves[i]->m_PlaneBorderIntersectFlag = true;
                }
            }
        }

        m_SCurveVec.push_back( pSCurve );
    }
}


void Surf::InitMesh( vector< ISegChain* > chains )
{

//static int name_cnt = 0;
//char str[256];
//sprintf( str, "Surf_UW%d.dat", name_cnt );
//name_cnt++;
//FILE* fp = fopen( str, "w" );
//int ccnt = 0;
//for ( int i = 0 ; i < (int)chains.size() ; i++ )
//{
//  if ( ccnt%5 == 0 ) fprintf( fp, "COLOR RED\n" );
//  else if ( ccnt % 5 == 1 ) fprintf( fp, "COLOR BLUE\n" );
//  else if ( ccnt % 5 == 2 ) fprintf( fp, "COLOR GREEN\n" );
//  else if ( ccnt % 5 == 3 ) fprintf( fp, "COLOR PURPLE\n" );
//  else if ( ccnt % 5 == 4 ) fprintf( fp, "COLOR YELLOW\n" );
//  fprintf( fp, "MOVE \n" );
//  ccnt++;
//      for ( int j = 1 ; j < (int)chains[i]->m_TessVec.size() ; j++ )
//      {
//          vec2d uw0 = chains[i]->m_TessVec[j-1]->GetPuw( this )->m_UW;
//          vec2d uw1 = chains[i]->m_TessVec[j]->GetPuw( this )->m_UW;
//          fprintf( fp, "%f %f\n", uw0[0], uw0[1] );
//          fprintf( fp, "%f %f\n", uw1[0], uw1[1] );
//      }
//}
//fclose(fp);

    //==== Store Only One Instance of each IPnt ====//
    set< IPnt* > ipntSet;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
        for ( int j = 0 ; j < ( int )chains[i]->m_TessVec.size() ; j++ )
        {
            ipntSet.insert( chains[i]->m_TessVec[j] );
        }

    vector < vec2d > uwPntVec;

    set< IPnt* >::iterator ip;
    for ( ip = ipntSet.begin() ; ip != ipntSet.end() ; ip++ )
    {
        vec2d uw = ( *ip )->GetPuw( this )->m_UW;

        int min_id;
        double min_dist = 1.0;
        for ( int i = 0 ; i < ( int )uwPntVec.size() ; i++ )
        {
            double d = dist( uwPntVec[i], uw );
            if ( d < min_dist )
            {
                min_dist = d;
                min_id = i;
            }
        }

        if ( min_dist < 0.0001 )
        {
            ( *ip )->m_Index = min_id;
        }
        else
        {
            uwPntVec.push_back( uw );
            ( *ip )->m_Index = uwPntVec.size() - 1;
        }
    }


    MeshSeg seg;
    vector< MeshSeg > isegVec;
    for ( int i = 0 ; i < ( int )chains.size() ; i++ )
        for ( int j = 1 ; j < ( int )chains[i]->m_TessVec.size() ; j++ )
        {
            seg.m_Index[0] = chains[i]->m_TessVec[j - 1]->m_Index;
            seg.m_Index[1] = chains[i]->m_TessVec[j]->m_Index;
            isegVec.push_back( seg );
        }

//  ////jrg Check For Duplicate Segs
//  vector< MeshSeg > dupMeshSegVec;
//  for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//  {
//      int iind0 = isegVec[i].m_Index[0];
//      int iind1 = isegVec[i].m_Index[1];
//      bool dup = false;
//      for ( int j = i+1 ; j < (int)isegVec.size() ; j++ )
//      {
//          int jind0 = isegVec[j].m_Index[0];
//          int jind1 = isegVec[j].m_Index[1];
//          if ( (iind0 == jind0 && iind1 == jind1) ||
//               (iind0 == jind1 && iind1 == jind0) )
//          {
//              dup = true;
//              //printf("Surf: Duplicate Seg \n" );
//          }
//      }
//      if ( !dup )
//          dupMeshSegVec.push_back( isegVec[i] );
//  }
//  isegVec = dupMeshSegVec;


////jrg - Check For Point Close to Other Segs
//vector< MeshSeg > newMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//  int iind0 = isegVec[i].m_Index[0];
//  int iind1 = isegVec[i].m_Index[1];
//
//  for ( int j = 0 ; j < (int)uwPntVec.size() ; j++ )
//  {
//      if ( j != iind0 && j != iind1 )
//      {
//          vec2d proj = proj_pnt_on_line_seg(uwPntVec[iind0], uwPntVec[iind1], uwPntVec[j] );
//          double d = dist( proj, uwPntVec[j] );
//
//          if ( d < 0.000001 )
//          {
//              MeshSeg addseg1, addseg2;
//              addseg1.m_Index[0] = iind0;
//              addseg1.m_Index[1] = j;
//              addseg2.m_Index[0] = j;
//              addseg2.m_Index[1] = iind1;
//              newMeshSegVec.push_back( addseg1 );
//              newMeshSegVec.push_back( addseg2 );
//
//              //printf("Surf: %d Proj Pnt Dist = %f\n", (int)this, d );
//              //printf("  Seg = %f %f   %f %f\n",
//              //  uwPntVec[iind0].x(),  uwPntVec[iind0].y(),
//              //  uwPntVec[iind1].x(),  uwPntVec[iind1].y());
//              //vec3d p = CompPnt( uwPntVec[j].x(), uwPntVec[j].y() );
//              //printf("  Pnt uv = %f %f    %f %f %f\n", uwPntVec[j].x(), uwPntVec[j].y(), p.x(), p.y(), p.z() );
//          }
//          else
//          {
//              newMeshSegVec.push_back( isegVec[i] );
//          }
//      }
//  }
//}

//vector< MeshSeg > smallMeshSegVec;
//for ( int i = 0 ; i < (int)isegVec.size() ; i++ )
//{
//  int ind0 = isegVec[i].m_Index[0];
//  int ind1 = isegVec[i].m_Index[1];
//  double d = dist( uwPntVec[ind0], uwPntVec[ind1] );
//  if ( d > 0.00000001 )
//  {
//      smallMeshSegVec.push_back( isegVec[i] );
//  }
//  else
//      printf("Surf: %d Small Seg Dist = %12.12f\n", (int)this, d );
//}
//isegVec = smallMeshSegVec;


    m_Mesh.InitMesh( uwPntVec, isegVec );
}


void Surf::BuildDistMap()
{
    int i, j;
    int nump = 101;

    vec2d VspMinUW = Convert2VspSurf( 0.0, 0.0 );
    double VspMinU = VspMinUW.v[0];
    double VspMinW = VspMinUW.v[1];

    vec2d VspMaxUW = Convert2VspSurf( m_MaxU, m_MaxW );
    double VspMaxU = VspMaxUW.v[0];
    double VspMaxW = VspMaxUW.v[1];

    double VspdU = VspMaxU - VspMinU;
    double VspdW = VspMaxW - VspMinW;

    //==== Load Point Vec ====//
    vector< vector< vec3d > > pvec;
    pvec.resize( nump );
    for ( i = 0 ; i < nump ; i++ )
    {
        pvec[i].resize( nump );
        double u = VspMinU + VspdU * ( double )i / ( double )( nump - 1 );
        for ( j = 0 ; j < nump ; j++ )
        {
            double w = VspMinW + VspdW * ( double )j / ( double )( nump - 1 );
            vec2d uw = Convert2Surf( u, w );
            pvec[i][j] = CompPnt( uw.v[0], uw.v[1] );
        }
    }

    //==== Find U Dists ====//
    double maxUDist = 0.0;
    vector< double > uDistVec;
    for ( i = 0 ; i < nump ; i++ )
    {
        double sum_d = 0.0;
        for ( j = 1 ; j < nump ; j++  )
        {
            sum_d += dist( pvec[j - 1][i], pvec[j][i] );
        }
        uDistVec.push_back( sum_d );

        if ( sum_d > maxUDist )
        {
            maxUDist = sum_d;
        }
    }
    if ( maxUDist < DBL_EPSILON )
    {
        maxUDist = 0.000000001;
    }

    //==== Find W Dists ====//
    double maxWDist = 0.0;
    vector< double > wDistVec;
    for ( i = 0 ; i < nump ; i++ )
    {
        double sum_d = 0.0;
        for ( j = 1 ; j < nump ; j++  )
        {
            sum_d += dist( pvec[i][j - 1], pvec[i][j] );
        }
        wDistVec.push_back( sum_d );

        if ( sum_d > maxWDist )
        {
            maxWDist = sum_d;
        }
    }

    if ( maxWDist < DBL_EPSILON )
    {
        maxWDist = 0.000000001;
    }


    //==== Scale U Dists ====//
    double wu_ratio = VspdW / VspdU;
    m_UScaleMap.resize( uDistVec.size() );
    for ( i = 0 ; i < ( int )uDistVec.size() ; i++ )
    {
        m_UScaleMap[i] = wu_ratio * ( uDistVec[i] / maxWDist );

        if ( m_UScaleMap[i] < 0.00001 )
        {
            m_UScaleMap[i] = 0.00001;
        }
    }

    //==== Scale W Dists ====//
    double uw_ratio = VspdU / VspdW;
    m_WScaleMap.resize( wDistVec.size() );
    for ( i = 0 ; i < ( int )wDistVec.size() ; i++ )
    {
        m_WScaleMap[i] = uw_ratio * ( wDistVec[i] / maxUDist );

        if ( m_WScaleMap[i] < 0.00001 )
        {
            m_WScaleMap[i] = 0.00001;
        }
    }

    //==== Figure Out Which to Scale ====//
    double min_u_scale = 1.0e12;
    double max_u_scale = 0.0;
    for ( i = 0 ; i < ( int )m_UScaleMap.size() ; i++ )
    {
        if ( m_UScaleMap[i] < min_u_scale )
        {
            min_u_scale = m_UScaleMap[i];
        }
        if ( m_UScaleMap[i] > max_u_scale )
        {
            max_u_scale = m_UScaleMap[i];
        }
    }
    double u_ratio = max_u_scale / min_u_scale;

    double min_w_scale = 1.0e12;
    double max_w_scale = 0.0;
    for ( i = 0 ; i < ( int )m_WScaleMap.size() ; i++ )
    {
        if ( m_WScaleMap[i] < min_w_scale )
        {
            min_w_scale = m_WScaleMap[i];
        }
        if ( m_WScaleMap[i] > max_w_scale )
        {
            max_w_scale = m_WScaleMap[i];
        }
    }
    double w_ratio = max_w_scale / min_w_scale;

    if ( u_ratio > w_ratio )
    {
        m_ScaleUFlag = true;
    }
    else
    {
        m_ScaleUFlag = false;
    }



//char str[256];
//static int cnt = 0;
//sprintf( str, "uwscale_%d.dat", cnt );
//cnt++;
//  FILE* fp = fopen(str, "w");
//  fprintf( fp, "ws 1 2\n" );
//  fprintf( fp, "color green\n" );
//  for ( i = 0 ; i < (int)m_WScaleMap.size() ; i++ )
//  {
//      double u = (double)i/(double)(m_WScaleMap.size()-1);
//      fprintf( fp, "%f %f \n", u, m_WScaleMap[i]  );
//  }
//  fprintf( fp, "color blue\n" );
//  for ( i = 0 ; i < 1001 ; i++ )
//  {
//      double u = (double)i/(double)(1000);
//      fprintf( fp, "%f %f \n", u, GetWScale(u)  );
//  }
//  fclose( fp );

}

double Surf::GetUScale( double w )      // w 0->1
{
    if ( !m_ScaleUFlag )
    {
        return 1.0;
    }

    int num = m_UScaleMap.size();
    int ind = ( int )( w * ( double )( num - 1 ) );
    if ( ind < 0 )
    {
        ind = 0;
    }
    if ( ind > num - 2 )
    {
        ind = num - 2;
    }

    double fract = w * ( double )( num - 1 ) - ( double )ind;
    if ( fract < 0.0 )
    {
        fract = 0.0;
    }
    if ( fract > 1.0 )
    {
        fract = 1.0;
    }

    double uscale = m_UScaleMap[ind] + fract * ( m_UScaleMap[ind + 1] - m_UScaleMap[ind] );
    return uscale;
}

double Surf::GetWScale( double u )      // u 0->1
{
    if ( m_ScaleUFlag )
    {
        return 1.0;
    }

    int num = m_WScaleMap.size();
    int ind = ( int )( u * ( double )( num - 1 ) );
    if ( ind < 0 )
    {
        ind = 0;
    }
    if ( ind > num - 2 )
    {
        ind = num - 2;
    }

    double fract = u * ( double )( num - 1 ) - ( double )ind;
    if ( fract < 0.0 )
    {
        fract = 0.0;
    }
    if ( fract > 1.0 )
    {
        fract = 1.0;
    }

    double wscale = m_WScaleMap[ind] + fract * ( m_WScaleMap[ind + 1] - m_WScaleMap[ind] );
    return wscale;
}

bool Surf::ValidUW( vec2d & uw )
{
    //return true;
    double slop = 0.0001;
    if ( uw[0] < -slop )
    {
        return false;
    }
    if ( uw[1] < -slop )
    {
        return false;
    }
    if ( uw[0] > m_MaxU + slop )
    {
        return false;
    }
    if ( uw[1] > m_MaxW + slop )
    {
        return false;
    }

    if ( uw[0] < 0.0 )
    {
        uw[0] = 0.0;
    }
    if ( uw[1] < 0.0 )
    {
        uw[1] = 0.0;
    }
    if ( uw[0] > m_MaxU )
    {
        uw[0] = m_MaxU;
    }
    if ( uw[1] > m_MaxW )
    {
        uw[1] = m_MaxW;
    }

    return true;
}

void Surf::LoadBorderCurves( vector< vector <vec3d> > & borderCurves )
{
    vector< vec3d > borderPnts;

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = 0
    {
        borderPnts.push_back( m_Pnts[i][0] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumU ; i++ )                // Border Curve w = max
    {
        borderPnts.push_back( m_Pnts[i][m_NumW - 1] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = 0
    {
        borderPnts.push_back( m_Pnts[0][i] );
    }
    borderCurves.push_back( borderPnts );

    borderPnts.clear();
    for ( int i = 0 ; i < m_NumW ; i++ )                // Border Curve u = max
    {
        borderPnts.push_back( m_Pnts[m_NumU - 1][i] );
    }
    borderCurves.push_back( borderPnts );
}

bool Surf::BorderCurveMatch( vector< vec3d > & curveA, vector< vec3d > & curveB )
{
    double tol = 0.00000001;

    if ( curveA.size() != curveB.size() )
    {
        return false;
    }

    bool match = true;

    //==== Check Same Way ====//
    for ( int i = 0 ; i < ( int )curveA.size() ; i++ )
    {
        if ( dist_squared( curveA[i], curveB[i] ) > tol )
        {
            match = false;
            break;
        }
    }

    if ( match )
    {
        return true;
    }

    match = true;
    int max_ind = ( int )curveB.size() - 1;
    //==== Check Opposite Way ====//
    for ( int i = 0 ; i < ( int )curveA.size() ; i++ )
    {
        if ( dist_squared( curveA[i], curveB[max_ind - i] ) > tol )
        {
            match = false;
            break;
        }
    }

    if ( match )
    {
        return true;
    }

    return false;
}

bool Surf::BorderMatch( Surf* otherSurf )
{
    vector< vector< vec3d > > borderCurvesA;
    LoadBorderCurves( borderCurvesA );

    vector< vector< vec3d > > borderCurvesB;
    otherSurf->LoadBorderCurves( borderCurvesB );

    for ( int i = 0 ; i < ( int )borderCurvesA.size() ; i++ )
    {
        for ( int j = 0 ; j < ( int )borderCurvesB.size() ; j++ )
        {
            if ( BorderCurveMatch( borderCurvesA[i], borderCurvesB[j] ) )
            {
                return true;
            }
        }
    }
    return false;
}

bool Surf::SurfMatch( Surf* otherSurf )
{
    double tol = 0.00000001;

    if ( otherSurf )
    {
        vector< vector< vec3d > > oPnts = otherSurf->GetControlPnts();
        int oNumU = oPnts.size();
        int oNumW = oPnts[0].size();

        if ( oNumU == m_NumU && oNumW == m_NumW ) // Possible match
        {
            bool match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ i ][ j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }


            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ i ][ m_NumW - 1 - j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }


            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumU - 1 - i ][ m_NumW - 1 - j ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }
        }

        if ( oNumU == m_NumW && oNumW == m_NumU ) // Possible flipped match
        {
            bool match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ j ][ i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ j ][ m_NumU - 1 - i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }

            match = true;
            for ( int i = 0; i < m_NumU; i++ )
            {
                for ( int j = 0; j < m_NumW; j++ )
                {
                    if ( dist_squared( m_Pnts[ i ][ j ], oPnts[ m_NumW - 1 - j ][ m_NumU - 1 - i ] ) > tol )
                    {
                        match = false;
                        break;
                    }
                }
                if ( !match )
                {
                    break;
                }
            }

            if ( match )
            {
                return true;
            }
        }
    }
    return false;
}

//==== Functions to Map Between Surf and VspSurf Parameterizations ====//
vec2d Surf::Convert2VspSurf( double u, double w )
{
    double u_vsp = InterpolateToVspSurf( u_to_vspsurf, u );
    double w_vsp = InterpolateToVspSurf( w_to_vspsurf, w );

    return vec2d( u_vsp, w_vsp );
}

vec2d Surf::Convert2Surf( double u, double w )
{
    bool in_range;
    double u_surf, w_surf;
    vec2d ret_vec;

    u_surf = interpolate( u_to_surf, u, &in_range );
    if ( in_range )
    {
        ret_vec.set_x( u_surf );
    }
    else
    {
        ret_vec.set_x( -1 );
    }

    w_surf = interpolate( w_to_surf, w, &in_range );
    if ( in_range )
    {
        ret_vec.set_y( w_surf );
    }
    else
    {
        ret_vec.set_y( -1 );
    }

    return ret_vec;
}

double Surf::InterpolateToVspSurf( const vector< double> & vec, const double & surf_val ) const
{
    int x0 = (int)floor( surf_val );
    int x1 = x0 + 1;

    if ( vec.size() == 0 )
    {
        return 0.0;
    }

    if ( x0 < 0 )
    {
        return vec.front();
    }
    if ( x1 > ( ( int )vec.size() - 1 ) )
    {
        return vec.back();
    }
    double y0 = vec[x0];
    double y1 = vec[x1];

    double denom = ( double )( x1 - x0 );

    if ( denom == 0.0 )
    {
        return 0.0;
    }

    return ( y1 - y0 ) / denom * ( surf_val - ( double )x0 ) + y0;
}

void Surf::Subtag( bool tag_subs )
{
    vector< SimpTri >& tri_vec = m_Mesh.GetSimpTriVec();
    vector< vec2d >& pnts = m_Mesh.GetSimpUWPntVec();
    vector< SubSurface* > s_surfs;

    if ( tag_subs ) s_surfs = SubSurfaceMgr.GetSubSurfs( m_GeomID );

    for ( int t = 0 ; t < ( int ) tri_vec.size() ; t++ )
    {
        SimpTri& tri = tri_vec[t];
        tri.m_Tags.push_back( m_BaseTag );
        vec2d center = ( pnts[tri.ind0] + pnts[tri.ind1] + pnts[tri.ind2] ) * 1 / 3.0;
        vec2d cent2d = Convert2VspSurf( center.x(), center.y() );

        for ( int s = 0 ; s < ( int ) s_surfs.size() ; s++ )
        {
            if ( s_surfs[s]->Subtag( vec3d( cent2d.x(), cent2d.y(), 0 ) ) )
            {
                tri.m_Tags.push_back( s_surfs[s]->m_Tag );
            }
        }
        SubSurfaceMgr.m_TagCombos.insert( tri.m_Tags );
    }
}

/*
void Surf::Draw()
{
    ////==== Draw Control Hull ====//
    //glLineWidth( 2.0 );
    //glColor3ub( 0, 0, 255 );

    //for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
    //{
    //  glBegin( GL_LINE_STRIP );
    //  for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
    //  {
    //      glVertex3dv( m_Pnts[i][j].data() );
    //  }
    //  glEnd();
    //}
    //glPointSize( 3.0 );
    //glColor3ub( 255, 255, 255 );
    //glBegin( GL_POINTS );
    //for ( int i = 0 ; i < (int)m_Pnts.size() ; i++ )
    //{
    //  for ( int j = 0 ; j < (int)m_Pnts[i].size() ; j++ )
    //  {
    //      glVertex3dv( m_Pnts[i][j].data() );
    //  }
    //}
    //glEnd();



    //==== Draw Surface ====//
    int max_u = (m_NumU-1)/3;
    int max_w = (m_NumW-1)/3;

    glLineWidth( 1.0 );
    glColor3ub( 0, 255, 0 );

    int num_xsec = 10;
    int num_tess = 20;
    for ( int i = 0 ; i < num_xsec ; i++ )
    {
        double u = max_u*(double)i/(double)(num_xsec-1);
        glBegin( GL_LINE_STRIP );
        for ( int j = 0 ; j < num_tess ; j++ )
        {
            double w = max_w*(double)j/(double)(num_tess-1);
            vec3d p = CompPnt( u, w );
            glVertex3dv( p.data() );
        }
        glEnd();
    }

    for ( int j = 0 ; j < num_xsec ; j++ )
    {
        double w = max_w*(double)j/(double)(num_xsec-1);
        glBegin( GL_LINE_STRIP );
        for ( int i = 0 ; i < num_tess ; i++ )
        {
            double u = max_u*(double)i/(double)(num_tess-1);
            vec3d p = CompPnt( u, w );
            glVertex3dv( p.data() );

        }
        glEnd();
    }

    //for ( int i = 0 ; i < (int)m_SCurveVec.size() ; i++ )
    //{
    //  m_SCurveVec[i]->Draw();
    //}

    m_Mesh.Draw();

    //for ( int i = 0 ; i < (int)m_PatchVec.size() ; i++ )
    //{
    //  m_PatchVec[i]->Draw();
    //}

    //glLineWidth( 2.0 );
    //glColor3ub( 255, 0, 0 );
    //glBegin( GL_LINES );
    //for ( int i = 0 ; i < (int)ipnts.size() ; i++ )
    //{
    //  if ( i%4 > 1 )
    //      glColor3ub( 255, 0, 0 );
    //  else
    //      glColor3ub( 255, 255, 0 );


    //  vec3d uw = ipnts[i];
    //  vec3d p = CompPnt( uw[0], uw[1] );
    //  glVertex3dv( p.data() );
    //}
    //glEnd();



}
*/

void Surf::SetDefaultParmMap()
{
	int num_u_map = 1 + ( m_NumU - 1 ) / 3;
    int num_w_map = 1 + ( m_NumW - 1 ) / 3;

    u_to_vspsurf.resize( num_u_map );
    w_to_vspsurf.resize( num_w_map );

    for ( int i = 0 ; i < num_u_map; i++ )
    {
        u_to_vspsurf[ i ] = 1.0 * i;
        u_to_surf[ 1.0 * i ] = i;
    }

    for ( int i = 0 ; i < num_w_map ; i++ )
    {
        w_to_vspsurf[ i ] = 1.0 * i;
        w_to_surf[ 1.0 * i ] = i;
    }
}



