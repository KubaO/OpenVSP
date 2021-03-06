

//==== Init Is Called Once During Each Custom Geom Construction  ============================//
//==== Avoid Global Variables Unless You Want Shared With All Custom Geoms of This Type =====//
void Init()
{
	//==== Test Parm Template  =====//
	string length = AddParm( PARM_DOUBLE_TYPE, "Length", "Design" );
	SetParmValLimits( length, 5.0, 0.001, 1.0e12 );

	string width = AddParm( PARM_DOUBLE_TYPE, "Width", "Design" );
	SetParmValLimits( width, 7.0, 0.001, 1.0e12 );

	string height = AddParm( PARM_DOUBLE_TYPE, "Height", "Design" );
	SetParmValLimits( height, 3.0, 0.001, 1.0e12 );

	string square_flag = AddParm( PARM_BOOL_TYPE, "SquareFlag", "Design" );
	SetParmVal( square_flag, 0.0 );

	//==== Add Cross Sections  =====//
	string xsec_surf = AddXSecSurf();
	AppendXSec( xsec_surf, XS_POINT);
	AppendXSec( xsec_surf, XS_ROUNDED_RECTANGLE);
	AppendXSec( xsec_surf, XS_ROUNDED_RECTANGLE);
	AppendXSec( xsec_surf, XS_POINT);

	//==== Set Some Decent Tess Vals ====//
	string geom_id = GetCurrCustomGeom();
	SetParmVal( GetParm( geom_id, "Tess_U",  "Shape" ), 10 );
	SetParmVal( GetParm( geom_id, "Tess_W",  "Shape" ), 9 );

}

//==== Global Gui IDs - These Are Consistent For All Created Boxes And Used In UpdateGUI====// 
int DesignTab;
int LengthSlider;
int WidthSlider;
int HeightSlider;
int SquareButton;

//==== InitGui Is Called Once During Each Custom Geom Construction ====//
void InitGui()
{
	DesignTab = AddGui( GDEV_TAB, "Design"  );
	AddGui( GDEV_YGAP );
	AddGui( GDEV_DIVIDER_BOX, "Design" );
	LengthSlider = AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Length"  );
	WidthSlider = AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Width"  );
	HeightSlider = AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Height"  );
	AddGui( GDEV_YGAP );
	SquareButton = AddGui( GDEV_TOGGLE_BUTTON, "Square Box" );
} 

//==== UpdateGui Is Called Every Time The Gui is Updated ====//
void UpdateGui()
{
	string geom_id = GetCurrCustomGeom();
	UpdateGui( LengthSlider, GetParm( geom_id, "Length", "Design" ) ); 
	UpdateGui( WidthSlider, GetParm( geom_id,  "Width",  "Design" ) ); 
	UpdateGui( HeightSlider, GetParm( geom_id, "Height", "Design" ) ); 
	UpdateGui( SquareButton, GetParm( geom_id, "SquareFlag", "Design" ) ); 

	//==== Check For Square Flag ====//
	bool square_flag = GetBoolParmVal( GetParm( geom_id, "SquareFlag", "Design" ) );
	if ( square_flag )
	{
		//==== Disconnect Width/Height Sliders From Parms ====//
		UpdateGui( WidthSlider, GetParm( geom_id,  "",  "Design" ) ); 
		UpdateGui( HeightSlider, GetParm( geom_id, "", "Design" ) ); 
	}
} 

//==== UpdateSurf Is Called Every Time The Geom is Updated ====//
void UpdateSurf()
{
	string geom_id = GetCurrCustomGeom();


	//==== Get Curr Vals ====//
	double length_val = GetParmVal( GetParm( geom_id, "Length", "Design" ) );
	double width_val  = GetParmVal( GetParm( geom_id, "Width",  "Design" ) );
	double height_val = GetParmVal( GetParm( geom_id, "Height", "Design" ) );

	//==== Check For Square Flag ====//
	bool square_flag = GetBoolParmVal( GetParm( geom_id, "SquareFlag", "Design" ) );
	if ( square_flag )
	{
		width_val = height_val = length_val;
		SetParmVal( GetParm( geom_id, "Width",  "Design" ),  width_val);
		SetParmVal( GetParm( geom_id, "Height",  "Design" ), height_val);
	}

	//==== Get The XSec Surf ====//
	string xsec_surf = GetXSecSurf( geom_id, 0 );

	//==== Define The First/Last XSec Placement ====//
	string xsec3 = GetXSec( xsec_surf, 3 );
	SetCustomXSecLoc( xsec3, vec3d( length_val, 0, 0 ) );

	//==== Define The Middle XSecs ====//
	string xsec1 = GetXSec( xsec_surf, 1 );
	SetParmVal( GetXSecParm( xsec1, "RoundedRect_Height" ), height_val );
	SetParmVal( GetXSecParm( xsec1, "RoundedRect_Width" ),  width_val );
	SetParmVal( GetXSecParm( xsec1, "RoundRectXSec_Radius" ), 0.0 );

	string xsec2 = GetXSec( xsec_surf, 2 );
	SetParmVal( GetXSecParm( xsec2, "RoundedRect_Height" ), height_val );
	SetParmVal( GetXSecParm( xsec2, "RoundedRect_Width" ),  width_val );
	SetParmVal( GetXSecParm( xsec2, "RoundRectXSec_Radius" ), 0.0 );
	SetCustomXSecLoc( xsec2, vec3d( length_val, 0, 0 ) );

	SkinXSecSurf();
}
