//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StackGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPSTACKGEOM__INCLUDED_)
#define VSPSTACKGEOM__INCLUDED_

#include "Geom.h"
#include "XSec.h"
#include "XSecCurve.h"
#include "XSecSurf.h"

//==== Stack Geom ====//
class StackGeom : public GeomXSec
{
public:
    enum {STACK_FREE, STACK_LOOP, NUM_STACK_POLICY};

    StackGeom( Vehicle* vehicle_ptr );
    virtual ~StackGeom();

    virtual void ComputeCenter();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual int NumXSec()
    {
        return m_XSecSurf.NumXSec();
    }

    virtual void SetActiveXSecType( int type );
    virtual void CutActiveXSec();
    virtual void CopyActiveXSec();
    virtual void PasteActiveXSec();
    virtual void InsertXSec( );
    virtual void InsertXSec( int type );

    virtual int GetNumXSecSurfs()
    {
        return 1;
    }
    virtual XSecSurf* GetXSecSurf( int index )
    {
        return &m_XSecSurf;
    }

    bool IsClosed() const;

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );
    virtual void Scale();

    //==== Set Drag Factors ====//
    virtual void LoadDragFactors( DragFactors& drag_factors );

    IntParm m_OrderPolicy;

protected:
    virtual void ChangeID( string id );

    virtual void UpdateSurf();
    virtual void UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts );

    virtual void EnforceOrder( StackXSec* xs, int indx, int policy );

    vector<int> m_TessUVec;

    bool m_Closed;

};
#endif // !defined(VSPSTACKGEOM__INCLUDED_)
