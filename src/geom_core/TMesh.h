//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Mesh Geometry Class
//
//
//   J.R. Gloudemans - 11/7/94
//   Sterling Software
//
//
//******************************************************************************

#ifndef TRI_TEST_H
#define TRI_TEST_H


#include "Vec2d.h"
#include "Vec3d.h"
#include "Matrix.h"
#include "BndBox.h"
#include "DragFactors.h"
#include "XmlUtil.h"

#include <vector>               //jrg windows?? 
#include <algorithm>            //jrg windows??
#include <string>
using namespace std;            //jrg windows??

extern "C"
{
    void triangulate( const char *, struct triangulateio *, struct triangulateio *,
                      struct triangulateio * );
}

class TEdge;
class TTri;
class TBndBox;
class NBndBox;
class TNodeGroup;

class TetraMassProp
{
public:
    TetraMassProp( string id, double den, vec3d& p0, vec3d& p1, vec3d& p2, vec3d& p3 );
    TetraMassProp()         {}
    ~TetraMassProp()        {}

    void SetPointMass( double massIn, vec3d& posIn );           // For Point Mass

    bool m_PointMassFlag;

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;
    vec3d m_v3;

    string m_CompId;

    vec3d m_CG;

    double m_Density;
    double m_Vol;
    double m_Mass;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};



class TriShellMassProp
{
public:
    TriShellMassProp( string id, double mass_area_in, vec3d& p0, vec3d& p1, vec3d& p2 );
    ~TriShellMassProp()     {}

    vec3d m_v0;
    vec3d m_v1;
    vec3d m_v2;

    vec3d m_CG;

    string m_CompId;

    double m_MassArea;
    double m_TriArea;
    double m_Mass;

    double m_Ixx;
    double m_Iyy;
    double m_Izz;

    double m_Ixy;
    double m_Ixz;
    double m_Iyz;
};

class TNode
{
public:
    /*
        TNode()         { static int cnt = 0;   cnt++;
                            printf("TNode Construct Cnt = %d \n", cnt); }
        ~TNode()        { static int cnt = 0;   cnt++;
                            printf("TNode Destruct Cnt = %d \n", cnt); }
    */
    TNode();
    virtual ~TNode();

    vec3d m_Pnt;
    vec3d m_Norm;
    int m_ID;

    vector< TTri* > m_TriVec;               // For WaterTight Check
    vector< TEdge* > m_EdgeVec;         // For WaterTight Check

    vector< TNode* > m_MergeVec;

    vector< TNode* > m_SplitNodeVec;

//  TNode* mapNode;

    int m_IsectFlag;
};

class TNodeGroup
{
public:
    vector< TNode* > nVec;
};

class TEdge
{
public:
    TEdge()
    {
        m_N0 = m_N1 = 0;
    }
    virtual ~TEdge()        {}

    TNode* m_N0;
    TNode* m_N1;

    TTri* m_Tri0;                           // For WaterTight Check
    TTri* m_Tri1;

};



class TTri
{
public:
    TTri();
    virtual ~TTri();

    TNode* m_N0;
    TNode* m_N1;
    TNode* m_N2;

    vec3d m_Norm;

    vector< TEdge* > m_ISectEdgeVec;        // List of Intersection Edges
    vector< TTri* > m_SplitVec;             // List of split tris
    vector< TNode* > m_NVec;                // Nodes for split tris
    vector< TEdge* > m_EVec;                // Edges for split tris

    virtual void SplitTri( int meshFlag = 0 );              // Split Tri to Fit ISect Edges
    virtual void TriangulateSplit( int flattenAxis );
    virtual void NiceTriSplit( int flattenAxis );
    virtual double ComputeArea()
    {
        return area( m_N0->m_Pnt, m_N1->m_Pnt, m_N2->m_Pnt );
    }
    virtual double ComputeAwArea()
    {
        vec3d t1, t2, t3;
        t1.set_xyz( 0.0, m_N0->m_Pnt.y(), m_N0->m_Pnt.z() );
        t2.set_xyz( 0.0, m_N1->m_Pnt.y(), m_N1->m_Pnt.z() );
        t3.set_xyz( 0.0, m_N2->m_Pnt.y(), m_N2->m_Pnt.z() );
        return area( t1, t2, t3 );
    }
    virtual void LoadNodesFromEdges();
    virtual void ComputeCosAngles( double* ang0, double* ang1, double* ang2 );

    virtual void SplitEdges( TNode* n01, TNode* n12, TNode* n20 );

    virtual vec3d CompNorm();
    virtual bool  ShareEdge( TTri* t );
    virtual bool MatchEdge( TNode* n0, TNode* n1, TNode* nA, TNode* nB, double tol );

    int m_InteriorFlag;
    string m_ID;
    double m_Mass;
    int m_InvalidFlag;

    TEdge* m_E0;
    TEdge* m_E1;
    TEdge* m_E2;

    TNode* m_cn0;                   // Nodes on Edge Centers for FEA Export
    TNode* m_cn1;
    TNode* m_cn2;


private:

    virtual int OnEdge( vec3d & p, TEdge* e, double onEdgeTol );
    virtual int DupEdge( TEdge* e0, TEdge* e1, double tol );

};

class TBndBox
{
public:
    TBndBox();
    virtual ~TBndBox();

    BndBox m_Box;
    vector< TTri* > m_TriVec;

    TBndBox* m_SBoxVec[8];      // Split Bnd Boxes

    void SplitBox();
    void AddTri( TTri* t );
    virtual void Intersect( TBndBox* iBox );
    virtual void NumCrossXRay( vec3d & orig, vector<double> & tParmVec );
    virtual void AddLeafNodes( vector< TBndBox* > & leafVec );

    virtual void SegIntersect( vec3d & p0, vec3d & p1, vector< vec3d > & ipntVec );

};

class NBndBox
{
public:
    NBndBox();
    virtual ~NBndBox();

    BndBox m_Box;
    vector< TNode* > m_NodeVec;

    NBndBox* m_SBoxVec[8];      // Split Bnd Boxes

    void SplitBox( double maxSize );
    void AddNode( TNode* n );
    virtual void AddLeafNodes( vector< NBndBox* > & leafVec );

};
class Geom;

class TMesh
{
public:
    TMesh();
    virtual ~TMesh();

    vector< TTri* >  m_TVec;
    vector< TNode* > m_NVec;
    vector< TEdge* > m_EVec;

    TBndBox m_TBox;

    void copy( TMesh* m );
    void CopyFlatten( TMesh* m );
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual void DecodeXml( xmlNodePtr & node );
    virtual xmlNodePtr EncodeTriList( xmlNodePtr & node );
    virtual void DecodeTriList( xmlNodePtr & node, int num_tris );

    //==== Stuff Copied From Geom That Created This Mesh ====//
    string m_PtrID;
    //bool reflected_flag;
    string m_NameStr;
    int m_MaterialID;
    vec3d m_Color;
    int m_MassPrior;
    double m_Density;
    double m_ShellMassArea;
    bool m_ShellFlag;
    DragFactors m_DragFactors;

    double m_TheoArea;
    double m_WetArea;
    double m_TheoVol;
    double m_GuessVol;
    double m_WetVol;

    void LoadGeomAttributes( Geom* geomPtr );
    int  RemoveDegenerate();
    void Intersect( TMesh* tm );
    void Split( int meshFlag = 0 );
    void DeterIntExt( vector< TMesh* >& meshVec );
    void DeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );
    void MassDeterIntExt( vector< TMesh* >& meshVec );
    void MassDeterIntExtTri( TTri* tri, vector< TMesh* >& meshVec );

    int DeterIntExtPnt( const vec3d& pnt, vector< TMesh* >& meshVec, TMesh* ignoreMesh = 0 );   // 1 Interior 0 Exterior

    void LoadBndBox();

    virtual double ComputeTheoArea();
    virtual double ComputeWetArea();
    virtual double ComputeAwaveArea();
    virtual double ComputeTheoVol();
    virtual double ComputeTrimVol();

    virtual void AddTri( const vec3d & v0, const vec3d & v1, const vec3d & v2, const vec3d & norm );
    virtual void AddTri( TNode* node0, TNode* node1, TNode* node2, const vec3d & norm );

    virtual void WriteSTLTris( FILE* file_id, Matrix4d XFormMat );

    virtual vec3d GetVertex( int index );
    virtual int   NumVerts();

    virtual void WaterTightCheck( FILE* fid, vector< TMesh* > & tMeshVec );
    virtual TNode* LowNode( TNode* node );
    virtual void FindEdge( TNode* node, TTri* tri0, TTri* tri1 );
    virtual void AddEdge( TTri* tri0, TTri* tri1, TNode* node0, TNode* node1 );
    virtual void SwapEdge( TEdge* edge );

    bool m_DeleteMeFlag;
    vector< TTri* > m_NonClosedTriVec;
    virtual void MergeNonClosed( TMesh* tm );
    virtual void CheckIfClosed();

    virtual void MatchNodes();
    virtual void CheckValid( FILE* fid );
    virtual void SwapEdges( double size );
    virtual void RelaxMesh( vector< TMesh* > & tMeshVec );
    virtual vec3d ProjectOnISectPairs( vec3d & offPnt, vector< vec3d > & pairVec );

    virtual void MeshStats( double* minEdgeLen, double* minTriAng, double* maxTriAng );
    virtual void TagNeedles( double minTriAng, double minAspectRatio, int delFlag );
    virtual void MoveNode( TNode* n0, TNode* n1 );
    virtual TTri* FindTriNodes( TTri* ignoreTri, TNode* n0, TNode* n1 );
    virtual TTri* FindTriPnts( TTri* ignoreTri, TNode* n0, TNode* n1 );

    virtual void MergeSplitNodes( vector< TTri* > & triVec, vector< TNode* > & nodeVec );
    static TNode* CheckDupOrAdd( TNode* node, vector< TNode* > & nodeVec, double tol = 0.00000001 );
    static TNode* CheckDupOrCreate( vec3d & p, vector< TNode* > & nodeVec, double tol = 0.00000001 );

    static void StressTest();
    static double Rand01();

    vector< vec3d > m_VertVec;
    vector< vec3d > m_ISectPairs;

    bool m_HalfBoxFlag;

protected:
    void CopyAttributes( TMesh* m );

};




#endif