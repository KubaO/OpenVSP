

//==== Init Is Called Once During Each Custom Geom Construction  ============================//
//==== Avoid Global Variables Unless You Want Shared With All Custom Geoms of This Type =====//
int NUM_XSECS = 11;
void Init()
{
	//==== Spine Parms  =====//
	string width = AddParm( PARM_DOUBLE_TYPE, "Width", "Design" );
	SetParmValLimits( width, 1.0, 0.001, 1.0e12 );

	string height = AddParm( PARM_DOUBLE_TYPE, "Height", "Design" );
	SetParmValLimits( height, 1.0, 0.001, 1.0e12 );

	string shape = AddParm( PARM_DOUBLE_TYPE, "Shape", "Design" );
	SetParmValLimits( shape, 0.0, -1.0, 1.0 );

	string chord = AddParm( PARM_DOUBLE_TYPE, "Chord", "Design" );
	SetParmValLimits( chord, 1.0, 0.001, 1.0e12 );

	string thick_chord = AddParm( PARM_DOUBLE_TYPE, "ThickChord", "Design" );
	SetParmValLimits( thick_chord, 0.2, 0.0, 0.5 );

	string camber = AddParm( PARM_DOUBLE_TYPE, "Camber", "Design" );
	SetParmValLimits( camber, 0.0, 0.0, 0.5 );

	string camber_loc = AddParm( PARM_DOUBLE_TYPE, "CamberLoc", "Design" );
	SetParmValLimits( camber_loc,  0.2, 0.0, 1.0 );

	string invert_flag = AddParm( PARM_BOOL_TYPE, "InvertFlag", "Design" );
	SetParmVal( invert_flag, 0.0 );


    //==== Add Cross Sections  =====//
    string xsec_surf = AddXSecSurf();
    for ( int i = 0 ; i < NUM_XSECS ; i++ )
    {
	    AppendXSec( xsec_surf, XS_FOUR_SERIES);
    }

	//==== Set Some Decent Tess Vals ====//
	string geom_id = GetCurrCustomGeom();
	SetParmVal( GetParm( geom_id, "Tess_U",  "Shape" ), 33 );
	SetParmVal( GetParm( geom_id, "Tess_W",  "Shape" ), 13 );

}

//==== InitGui Is Called Once During Each Custom Geom Construction ====//
void InitGui()
{
	AddGui( GDEV_TAB, "Design"  );
	AddGui( GDEV_YGAP );
	AddGui( GDEV_DIVIDER_BOX, "Spine" );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Width", "Width", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Height", "Height", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Shape", "Shape", "Design"  );
	AddGui( GDEV_YGAP );
	AddGui( GDEV_DIVIDER_BOX, "Airfoil Cross Section" );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Chord", "Chord", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "T/C", "ThickChord", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Camber", "Camber", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Camber Loc", "CamberLoc", "Design"  );
    AddGui( GDEV_TOGGLE_BUTTON, "Invert Airfoil", "InvertFlag", "Design" );

} 

//==== UpdateGui Is Called Every Time The Gui is Updated ====//
void UpdateGui()
{
} 

//==== UpdateSurf Is Called Every Time The Geom is Updated ====//
void UpdateSurf()
{
	string geom_id = GetCurrCustomGeom();

	//==== Get Curr Vals ====//
	double width_val   = GetParmVal( GetParm( geom_id, "Width", "Design" ) );
	double height_val  = GetParmVal( GetParm( geom_id, "Height",  "Design" ) );
	double shape_val   = GetParmVal( GetParm( geom_id, "Shape",  "Design" ) );

    double chord_val  = GetParmVal( GetParm( geom_id, "Chord",  "Design" ) );
    double tc_val  = GetParmVal( GetParm( geom_id, "ThickChord",  "Design" ) );
    double camber_val  = GetParmVal( GetParm( geom_id, "Camber",  "Design" ) );
    double camber_loc_val  = GetParmVal( GetParm( geom_id, "CamberLoc",  "Design" ) );
    bool invert_flag = GetBoolParmVal( GetParm( geom_id, "InvertFlag", "Design" ) );

    //==== Add Temp Spine Surf ====//
    string spine_surf = AddXSecSurf();
	string spine_xsec = AppendXSec( spine_surf, XS_GENERAL_FUSE );

	SetParmVal( GetXSecParm( spine_xsec, "Height" ), height_val );
	SetParmVal( GetXSecParm( spine_xsec, "Width" ),  width_val );
	SetParmVal( GetXSecParm( spine_xsec, "MaxWidthLoc" ),  shape_val );

    SetCustomXSecRot( spine_xsec, vec3d( 0, 0, 90 ) );
 
	//==== Get The XSec Surf ====//
	string xsec_surf = GetXSecSurf( geom_id, 0 );

	//==== Define XSecs ====//
    for ( int i = 0 ; i < NUM_XSECS ; i++ )
    {
	    string xsec = GetXSec( xsec_surf, i );
	    SetParmVal( GetXSecParm( xsec, "Chord" ), chord_val );
	    SetParmVal( GetXSecParm( xsec, "ThickChord" ), tc_val );
	    SetParmVal( GetXSecParm( xsec, "Camber" ), camber_val );
	    SetParmVal( GetXSecParm( xsec, "CamberLoc" ), camber_loc_val );
        if ( invert_flag )
	        SetParmVal( GetXSecParm( xsec, "Invert" ), 1.0 );
        else
            SetParmVal( GetXSecParm( xsec, "Invert" ), 0.0 );

        UpdateXSecLocRot( xsec, spine_xsec, i*0.10 );
    }

    //==== Align With X Axis ====//
    Matrix4d rot_mat;
    rot_mat.rotateZ( -90 );
    SetXSecSurfGlobalXForm( xsec_surf, rot_mat );

    //=== Remove Spine Surf ====//
    RemoveXSecSurf( spine_surf );

	SkinXSecSurf( true );
}


void UpdateXSecLocRot( string xsec, string spine_id, double fract  )
{
    vec3d pnt = ComputeXSecPnt( spine_id, fract );
	SetCustomXSecLoc( xsec, pnt );

    vec3d tan = ComputeXSecTan( spine_id, fract );
    double ang = signed_angle( tan, vec3d( 1, 0, 0 ), vec3d( 0, -1, 0 ) );
	SetCustomXSecRot( xsec, vec3d( 0, Rad2Deg(ang), 0) );
    SetXSecContinuity( xsec, 2 );
}