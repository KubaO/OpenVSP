//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ScriptMgr.h: interface to AngelScript
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"

#include "Parm.h"
#include "Vec3d.h"
#include "Matrix.h"
#include "VSP_Geom_API.h"
#include "APIErrorMgr.h"
#include "CustomGeom.h"
#include "AdvLinkMgr.h"
#include "XSec.h"
#include "Vehicle.h"
#include "ResultsMgr.h"
#include "StringUtil.h"
#include "FileUtil.h"

using namespace vsp;

//==== Implement a simple message callback function ====//
void MessageCallback( const asSMessageInfo *msg, void *param )
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
    {
        type = "WARN";
    }
    else if( msg->type == asMSGTYPE_INFORMATION )
    {
        type = "INFO";
    }
    printf( "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message );
}

//==================================================================================================//
//========================================= ScriptMgr      =========================================//
//==================================================================================================//


//==== Constructor ====//
ScriptMgrSingleton::ScriptMgrSingleton()
{

}

//==== Set Up Script Engine, Script Error Callbacks ====//
void ScriptMgrSingleton::Init( )
{
    //==== Only Init Once ====//
    static bool init_flag = false;
    if ( init_flag )
        return;
    init_flag = true;

    //==== Create the Script Engine ====//
    m_ScriptEngine = asCreateScriptEngine( ANGELSCRIPT_VERSION );
    asIScriptEngine* se = m_ScriptEngine;

    //==== Set the message callback to receive information on errors in human readable form.  ====//
    int r = se->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
    assert( r >= 0 );

    //==== Register Addons ====//
    RegisterStdString( m_ScriptEngine );
    RegisterScriptArray( m_ScriptEngine, true );
    RegisterScriptFile( m_ScriptEngine );
    RegisterScriptMath( m_ScriptEngine );
    RegisterScriptAny( m_ScriptEngine );

    //==== Cache Some Commom Types ====//
    m_IntArrayType    = se->GetObjectTypeById( se->GetTypeIdByDecl( "array<int>" ) );
    assert( m_IntArrayType );
    m_DoubleArrayType = se->GetObjectTypeById( se->GetTypeIdByDecl( "array<double>" ) );
    assert( m_DoubleArrayType );
    m_StringArrayType = se->GetObjectTypeById( se->GetTypeIdByDecl( "array<string>" ) );
    assert( m_StringArrayType );

    //==== Register VSP Enums ====//
    RegisterEnums( m_ScriptEngine );

    //==== Register VSP Objects ====//
    RegisterVec3d( m_ScriptEngine );
    m_Vec3dArrayType  = se->GetObjectTypeById( se->GetTypeIdByDecl( "array<vec3d>" ) );
    assert( m_Vec3dArrayType );

    RegisterMatrix4d( m_ScriptEngine );
    RegisterCustomGeomMgr( m_ScriptEngine );
    RegisterAdvLinkMgr( m_ScriptEngine );
    RegisterAPIErrorObj( m_ScriptEngine );
    RegisterAPI( m_ScriptEngine );
    RegisterUtility(  m_ScriptEngine );


}

void ScriptMgrSingleton::RunTestScripts()
{
    //===== Run Test Scripts ====//
 //   ScriptMgr.ReadScript( "TestScript", "../../TestScript.as"  );
    //ScriptMgr.ReadScriptFromFile( "TestScript", "../../../TestScript.as"  );
    //ScriptMgr.ExecuteScript( "TestScript", "void main()" );
    //ScriptMgr.ExecuteScript( "TestScript", "void TestAPIScript()" );
}

//==== Read And Execute Script File  ====//
void ScriptMgrSingleton::ReadExecuteScriptFile( const string &  file_name, const string &  function_name )
{
    string module_name = ReadScriptFromFile( "ReadExecute", file_name );

    ExecuteScript( module_name.c_str(), function_name.c_str() );
}

vector< string > ScriptMgrSingleton::ReadScriptsFromDir( const string & dir_name )
{
    vector< string > mod_name_vec;

    vector< string > file_vec = ScanFolder( dir_name.c_str() );

    for ( int i = 0 ; i < ( int )file_vec.size() ; i++ )
    {
        if ( file_vec[i].compare( file_vec[i].size() - 3, 3, ".as" ) == 0 )
        {
            string sub = file_vec[i].substr( 0, file_vec[i].size() - 3 );
            string file_name = dir_name;
            file_name.append( file_vec[i] );
            string module_name = ScriptMgr.ReadScriptFromFile( sub, file_name );

            if ( module_name.size() )
                mod_name_vec.push_back( module_name );
        }
    }

    return mod_name_vec;
}

//==== Start A New Module And Read Script ====//
string ScriptMgrSingleton::ReadScriptFromFile( const string & module_name, const string &  file_name )
{
    string content = ExtractContent( file_name );

    if ( content.size() < 2 )
    {
        return string();
    }

    return ReadScriptFromMemory( module_name, content );
}

//==== Start A New Module And Read Script ====//
string ScriptMgrSingleton::ReadScriptFromMemory( const string &  module_name, const string & script_content )
{
    int r;
    string updated_module_name = module_name;
    map< string, string >::iterator iter;

    //==== Check If Module Name Already Exists ====//
    iter = m_ModuleContentMap.find(updated_module_name);
    if ( iter != m_ModuleContentMap.end() )
    {
        //==== Check If Content is Same ====//
        if ( iter->second == script_content )
            return iter->first;

        //==== Need To Change Module Name ====//
        static int dup_cnt = 0;
        updated_module_name.append( StringUtil::int_to_string( dup_cnt, "%d" ) );
        dup_cnt++;
    }

    //==== Make Sure Not Dupicate Of Any Other Module ====//
    for ( iter = m_ModuleContentMap.begin() ; iter != m_ModuleContentMap.end() ; iter++ )
    {
        if ( iter->second == script_content )
            return iter->first;
    }

    //==== Start A New Module ====//
    r = m_ScriptBuilder.StartNewModule( m_ScriptEngine, updated_module_name.c_str() );
    if( r < 0 )        return string();

    r = m_ScriptBuilder.AddSectionFromMemory( updated_module_name.c_str(), script_content.c_str(), script_content.size()  );
    if ( r < 0 )    return string();

    r = m_ScriptBuilder.BuildModule();
    if ( r < 0 )    return string();

    //==== Add To Map ====//
    m_ModuleContentMap[ updated_module_name ] = script_content;

    return updated_module_name;
}

//==== Extract Content From File Into String ====//
string ScriptMgrSingleton::ExtractContent( const string & file_name )
{
    string file_content;
    FILE* fp = fopen( file_name.c_str(), "r" );
    if ( fp )
    {
        char buff[512];
        while ( fgets( buff, 512, fp ) )
        {
            file_content.append( buff );
        }
        file_content.append( "\0" );
        fclose( fp );
    }
    return file_content;
}


//==== Execute Function in Module ====//
void ScriptMgrSingleton::ExecuteScript(  const char* module_name,  const char* function_name )
{
    int r;

    // Find the function that is to be called.
    asIScriptModule *mod = m_ScriptEngine->GetModule( module_name );

    if ( !mod )
    {
        printf( "Error ExecuteScript GetModule %s\n", module_name );
        return;
    }

    asIScriptFunction *func = mod->GetFunctionByDecl( function_name );
    if( func == 0 )
    {
        return;
    }

    // Create our context, prepare it, and then execute
    asIScriptContext *ctx = m_ScriptEngine->CreateContext();
    ctx->Prepare( func );
    r = ctx->Execute();
    if( r != asEXECUTION_FINISHED )
    {
        // The execution didn't complete as expected. Determine what happened.
        if( r == asEXECUTION_EXCEPTION )
        {
            // An exception occurred, let the script writer know what happened so it can be corrected.
            printf( "An exception '%s' occurred \n", ctx->GetExceptionString() );
        }
    }
}

//==== Return Script Content Given Module Name ====//
string ScriptMgrSingleton::FindModuleContent( const string &  module_name )
{
    map< string, string >::iterator iter;
    iter = m_ModuleContentMap.find( module_name );

    string file_string;
    if ( iter != m_ModuleContentMap.end() )
    {
        file_string = iter->second;
    }
    return file_string;
}

//==== Write Script Content To File ====//
int ScriptMgrSingleton::SaveScriptContentToFile( const string & module_name, const string & file_name )
{
    map< string, string >::iterator iter;
    iter = m_ModuleContentMap.find( module_name );

    if ( iter == m_ModuleContentMap.end() )
        return -1;

    FILE* fp = fopen( file_name.c_str(), "w" );
    if ( !fp )
        return -2;

    if ( iter->second.size() == 0 )
        return -3;


    fprintf( fp, "%s", iter->second.c_str() );
    fclose( fp );

    return 0;
}


//==== Register Enums ====//
void ScriptMgrSingleton::RegisterEnums( asIScriptEngine* se )
{
    int r = se->RegisterEnum( "PARM_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_DOUBLE_TYPE", PARM_DOUBLE_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_INT_TYPE",    PARM_INT_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_BOOL_TYPE",   PARM_BOOL_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_FRACTION_TYPE", PARM_FRACTION_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "PARM_TYPE", "PARM_STRING_TYPE", PARM_STRING_TYPE );
    assert( r >= 0 );

    r = se->RegisterEnum( "GDEV" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TAB", GDEV_TAB );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_GROUP", GDEV_GROUP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_PARM_BUTTON", GDEV_PARM_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INPUT", GDEV_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER", GDEV_SLIDER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_LOG_SLIDER", GDEV_LOG_SLIDER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE", GDEV_SLIDER_ADJ_RANGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHECK_BUTTON", GDEV_CHECK_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_RADIO_BUTTON", GDEV_RADIO_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_BUTTON", GDEV_TOGGLE_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TRIGGER_BUTTON", GDEV_TRIGGER_BUTTON );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_TOGGLE_RADIO_GROUP", GDEV_TOGGLE_RADIO_GROUP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COUNTER", GDEV_COUNTER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_CHOICE", GDEV_CHOICE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_ADD_CHOICE_ITEM", GDEV_ADD_CHOICE_ITEM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_INPUT", GDEV_SLIDER_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_INPUT", GDEV_SLIDER_ADJ_RANGE_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SLIDER_ADJ_RANGE_TWO_INPUT", GDEV_SLIDER_ADJ_RANGE_TWO_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FRACT_PARM_SLIDER", GDEV_FRACT_PARM_SLIDER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_INDEX_SELECTOR", GDEV_INDEX_SELECTOR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_STRING_INPUT", GDEV_STRING_INPUT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_COLOR_PICKER", GDEV_COLOR_PICKER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_YGAP", GDEV_YGAP );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_DIVIDER_BOX", GDEV_DIVIDER_BOX );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_BEGIN_SAME_LINE", GDEV_BEGIN_SAME_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_END_SAME_LINE", GDEV_END_SAME_LINE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_FORCE_WIDTH", GDEV_FORCE_WIDTH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "GDEV", "GDEV_SET_FORMAT", GDEV_SET_FORMAT );
    assert( r >= 0 );

    r = se->RegisterEnum( "XSEC_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_FUSE", XSEC_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_STACK", XSEC_STACK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_TYPE", "XSEC_NUM_TYPES", XSEC_NUM_TYPES );
    assert( r >= 0 );

    r = se->RegisterEnum( "DIR_INDEX" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "X_DIR", X_DIR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Y_DIR", Y_DIR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "DIR_INDEX", "Z_DIR", Z_DIR );
    assert( r >= 0 );

    r = se->RegisterEnum( "XSEC_WIDTH_SHIFT" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_LE", XS_SHIFT_LE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_MID", XS_SHIFT_MID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_WIDTH_SHIFT", "XS_SHIFT_TE", XS_SHIFT_TE );
    assert( r >= 0 );

    r = se->RegisterEnum( "XSEC_CRV_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_POINT", XS_POINT );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_CIRCLE", XS_CIRCLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ELLIPSE", XS_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SUPER_ELLIPSE", XS_SUPER_ELLIPSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_ROUNDED_RECTANGLE", XS_ROUNDED_RECTANGLE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_GENERAL_FUSE", XS_GENERAL_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_FUSE", XS_FILE_FUSE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FOUR_SERIES", XS_FOUR_SERIES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_SIX_SERIES", XS_SIX_SERIES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_BICONVEX", XS_BICONVEX );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_WEDGE", XS_WEDGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_BEZIER", XS_BEZIER );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_CRV_TYPE", "XS_FILE_AIRFOIL", XS_FILE_AIRFOIL );
    assert( r >= 0 );

    r = se->RegisterEnum( "XSEC_SIDES_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_BOTH_SIDES", XSEC_BOTH_SIDES );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_LEFT_SIDE", XSEC_LEFT_SIDE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "XSEC_SIDES_TYPE", "XSEC_RIGHT_SIDE", XSEC_RIGHT_SIDE );
    assert( r >= 0 );


    r = se->RegisterEnum( "SET_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_ALL", SET_ALL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_SHOWN", SET_SHOWN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_NOT_SHOWN", SET_NOT_SHOWN );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "SET_TYPE", "SET_FIRST_USER", SET_FIRST_USER );
    assert( r >= 0 );

    r = se->RegisterEnum( "IMPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_STL", IMPORT_STL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_NASCART", IMPORT_NASCART );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_CART3D_TRI", IMPORT_CART3D_TRI );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_SURF", IMPORT_XSEC_SURF );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "IMPORT_TYPE", "IMPORT_XSEC_MESH", IMPORT_XSEC_MESH );
    assert( r >= 0 );

    r = se->RegisterEnum( "EXPORT_TYPE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_FELISA", EXPORT_FELISA );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSEC", EXPORT_XSEC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_STL", EXPORT_STL );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_RHINO3D", EXPORT_RHINO3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_AWAVE", EXPORT_AWAVE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_NASCART", EXPORT_NASCART );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_POVRAY", EXPORT_POVRAY );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_CART3D", EXPORT_CART3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_VORXSEC", EXPORT_VORXSEC );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_XSECGEOM", EXPORT_XSECGEOM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_GMSH", EXPORT_GMSH );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_X3D", EXPORT_X3D );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "EXPORT_TYPE", "EXPORT_BEZ", EXPORT_BEZ );
    assert( r >= 0 );

    r = se->RegisterEnum( "ERROR_CODE" );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_OK", vsp::VSP_OK );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_PTR", vsp::VSP_INVALID_PTR );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_TYPE", vsp::VSP_CANT_FIND_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_PARM", vsp::VSP_CANT_FIND_PARM );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_CANT_FIND_NAME", vsp::VSP_CANT_FIND_NAME );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_GEOM_ID", vsp::VSP_INVALID_GEOM_ID );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_DOES_NOT_EXIST", vsp::VSP_FILE_DOES_NOT_EXIST );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_FILE_WRITE_FAILURE", vsp::VSP_FILE_WRITE_FAILURE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_XSEC_TYPE", vsp::VSP_WRONG_XSEC_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_WRONG_FILE_TYPE", vsp::VSP_WRONG_FILE_TYPE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INDEX_OUT_RANGE", vsp::VSP_INDEX_OUT_RANGE );
    assert( r >= 0 );
    r = se->RegisterEnumValue( "ERROR_CODE", "VSP_INVALID_XSEC_ID", vsp::VSP_INVALID_XSEC_ID );
    assert( r >= 0 );

}

//==== Vec3d Constructors ====//
static void Vec3dDefaultConstructor( vec3d *self )
{
    new( self ) vec3d();
}
static void Vec3dCopyConstructor( const vec3d &other, vec3d *self )
{
    new( self ) vec3d( other );
}
static void Vec3dInitConstructor( double x, double y, double z, vec3d *self )
{
    new( self ) vec3d( x, y, z );
}

//==== Register Vec3d Object ====//
void ScriptMgrSingleton::RegisterVec3d( asIScriptEngine* se )
{
    //==== Register vec3d Object =====//
    int r = se->RegisterObjectType( "vec3d", sizeof( vec3d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA );
    assert( r >= 0 );

    //==== Register the vec3d Constructors  ====//
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Vec3dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(double, double, double)", asFUNCTION( Vec3dInitConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );
    r = se->RegisterObjectBehaviour( "vec3d", asBEHAVE_CONSTRUCT, "void f(const vec3d &in)", asFUNCTION( Vec3dCopyConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    //==== Register the vec3d Methods  ====//
    r = se->RegisterObjectMethod( "vec3d", "double& opIndex(int) const", asMETHODPR( vec3d, operator[], ( int ), double& ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "double x() const", asMETHOD( vec3d, x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "double y() const", asMETHOD( vec3d, y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "double z() const", asMETHOD( vec3d, z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_xyz(double x, double y, double z)", asMETHOD( vec3d, set_xyz ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_x(double x)", asMETHOD( vec3d, set_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_y(double y)", asMETHOD( vec3d, set_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d& set_z(double z)", asMETHOD( vec3d, set_z ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "void rotate_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void scale_x(double scale)", asMETHOD( vec3d, scale_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void scale_y(double scale)", asMETHOD( vec3d, scale_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void scale_z(double scale)", asMETHOD( vec3d, scale_z ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void offset_x(double offset)", asMETHOD( vec3d, offset_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void offset_y(double offset)", asMETHOD( vec3d, offset_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void offset_z(double offset)", asMETHOD( vec3d, offset_z ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_x(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_x ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void rotate_z_zero_y(double cos_alpha, double sin_alpha)", asMETHOD( vec3d, rotate_z_zero_y ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xy()", asMETHOD( vec3d, reflect_xy ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_xz()", asMETHOD( vec3d, reflect_xz ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d reflect_yz()", asMETHOD( vec3d, reflect_yz ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "vec3d opAdd(const vec3d &in) const", asFUNCTIONPR( operator+, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d opSub(const vec3d &in) const", asFUNCTIONPR( operator-, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul(double b) const", asFUNCTIONPR( operator*, ( const vec3d & a, double b ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d opMul_r(const vec3d &in) const", asFUNCTIONPR( operator*, ( const vec3d&, const vec3d& ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "vec3d opDiv(double b) const", asFUNCTIONPR( operator/, ( const vec3d&, double b ), vec3d ), asCALL_CDECL_OBJFIRST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "vec3d", "double mag() const", asMETHOD( vec3d, mag ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "vec3d", "void normalize()", asMETHOD( vec3d, normalize ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "double dist(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double dist_squared(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dist_squared, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double dot(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( dot, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d cross(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( cross, ( const vec3d&, const vec3d& ), vec3d  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double angle(const vec3d& in a, const vec3d& in b)", asFUNCTIONPR( angle, ( const vec3d&, const vec3d& ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double signed_angle(const vec3d& in a, const vec3d& in b, const vec3d& in ref )", asFUNCTIONPR( signed_angle, ( const vec3d & a, const vec3d & b, const vec3d & ref ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double cos_angle(const vec3d& in a, const vec3d& in b )", asFUNCTIONPR( cos_angle, ( const vec3d & a, const vec3d & b ), double  ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d RotateArbAxis(const vec3d& in p, double theta, const vec3d& in axis )", asFUNCTIONPR( RotateArbAxis, ( const vec3d & p, double theta, const vec3d & axis ), vec3d ), asCALL_CDECL );
    assert( r >= 0 );


}

//==== Matrix4d Constructors ====//
static void Matrix4dDefaultConstructor( Matrix4d *self )
{
    new( self ) Matrix4d();
}

//==== Register Matrix4d Object ====//
void ScriptMgrSingleton::RegisterMatrix4d( asIScriptEngine* se )
{
    //==== Register Matrix4d Object =====//
    int r = se->RegisterObjectType( "Matrix4d", sizeof( Matrix4d ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA );
    assert( r >= 0 );
    r = se->RegisterObjectBehaviour( "Matrix4d", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( Matrix4dDefaultConstructor ), asCALL_CDECL_OBJLAST );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "Matrix4d", "void loadIdentity()", asMETHOD( Matrix4d, loadIdentity ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void translatef( const double & in x, const double & in y, const double & in z)", asMETHOD( Matrix4d, translatef ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateX( const double & in ang )", asMETHOD( Matrix4d, rotateX ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateY( const double & in ang )", asMETHOD( Matrix4d, rotateY ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void rotateZ( const double & in ang )", asMETHOD( Matrix4d, rotateZ ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void rotate( const double & in ang, const vec3d & in axis )", asMETHOD( Matrix4d, rotate ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void scale( const double & in scale )", asMETHOD( Matrix4d, scale ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "Matrix4d", "vec3d xform( const vec3d & in v )", asMETHOD( Matrix4d, xform ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "vec3d getAngles()", asMETHOD( Matrix4d, getAngles ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void loadXZRef()", asMETHOD( Matrix4d, loadXZRef ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void loadXYRef()", asMETHOD( Matrix4d, loadXYRef ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "Matrix4d", "void loadYZRef()", asMETHOD( Matrix4d, loadYZRef ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "Matrix4d", "void affineInverse()", asMETHOD( Matrix4d, affineInverse ), asCALL_THISCALL );
    assert( r >= 0 );

    r = se->RegisterObjectMethod( "Matrix4d", "void buildXForm( const vec3d & in pos, const vec3d & in rot, const vec3d & in cent_rot )", asMETHOD( Matrix4d, buildXForm ), asCALL_THISCALL );
    assert( r >= 0 );

}

//==== Register Custom Geom Mgr Object ====//
void ScriptMgrSingleton::RegisterCustomGeomMgr( asIScriptEngine* se )
{
    int r;
    r = se->RegisterGlobalFunction( "string AddParm( int type, const string & in name, const string & in group )",
                                    asMETHOD( CustomGeomMgrSingleton, AddParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "string GetCurrCustomGeom()",
                                    asMETHOD( CustomGeomMgrSingleton, GetCurrCustomGeom ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "string GetCustomParm( int index )",
                                    asMETHOD( CustomGeomMgrSingleton, GetCustomParm ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "int AddGui( int type, const string & in label = string(), const string & in parm_name = string(), const string & in group_name = string(), double range = 10.0 )",
                                    asMETHOD( CustomGeomMgrSingleton, AddGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void UpdateGui( int gui_id, const string & in parm_id )",
                                    asMETHOD( CustomGeomMgrSingleton, AddUpdateGui ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "string AddXSecSurf()",
                                    asMETHOD( CustomGeomMgrSingleton, AddXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void RemoveXSecSurf(const string & in xsec_id)",
                                    asMETHOD( CustomGeomMgrSingleton, RemoveXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void ClearXSecSurfs()",
                                    asMETHOD( CustomGeomMgrSingleton, ClearXSecSurfs ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void SkinXSecSurf( bool closed_flag = false )",
                                    asMETHOD( CustomGeomMgrSingleton, SkinXSecSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void CloneSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, CloneSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void TransformSurf(int index, Matrix4d & in mat)",
                                    asMETHOD( CustomGeomMgrSingleton, TransformSurf ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );

    r = se->RegisterGlobalFunction( "void SetCustomXSecLoc( const string & in xsec_id, const vec3d & in loc )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecLoc ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void SetCustomXSecRot( const string & in xsec_id, const vec3d & in rot )",
                                    asMETHOD( CustomGeomMgrSingleton, SetCustomXSecRot ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "bool CheckClearTriggerEvent( int gui_id )",
                                    asMETHOD( CustomGeomMgrSingleton, CheckClearTriggerEvent ), asCALL_THISCALL_ASGLOBAL, &CustomGeomMgr );
    assert( r );

}



//==== Register Adv Link Mgr Object ====//
void ScriptMgrSingleton::RegisterAdvLinkMgr( asIScriptEngine* se )
{
    int r;
    r = se->RegisterGlobalFunction( "void AddInput( const string & in geom_name, int geom_index, const string & in parm_name, const string & in parm_group, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddInput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void AddOutput( const string & in geom_name, int geom_index, const string & in parm_name, const string & in parm_group, const string & in var_name )",
                                    asMETHOD( AdvLinkMgrSingleton, AddOutput ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "void SetVar( const string & in var_name, double val )", asMETHOD( AdvLinkMgrSingleton, SetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "double GetVar( const string & in var_name )", asMETHOD( AdvLinkMgrSingleton, GetVar ), asCALL_THISCALL_ASGLOBAL, &AdvLinkMgr );
    assert( r );
}

//==== Register API E Functions ====//
void ScriptMgrSingleton::RegisterAPIErrorObj( asIScriptEngine* se )
{
    //==== Register vec3d Object =====//
    int r = se->RegisterObjectType( "ErrorObj", sizeof( vsp::ErrorObj ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "ErrorObj", "ERROR_CODE GetErrorCode()", asMETHOD( vsp::ErrorObj, GetErrorCode ), asCALL_THISCALL );
    assert( r >= 0 );
    r = se->RegisterObjectMethod( "ErrorObj", "string GetErrorString()", asMETHOD( vsp::ErrorObj, GetErrorString ), asCALL_THISCALL );
    assert( r >= 0 );



}


//==== Register VSP API Functions ====//
void ScriptMgrSingleton::RegisterAPI( asIScriptEngine* se )
{
    int r;

    //==== API Error Fucntions ====//
    r = se->RegisterGlobalFunction( "bool GetErrorLastCallFlag()", asMETHOD( vsp::ErrorMgrSingleton, GetErrorLastCallFlag ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "int GetNumTotalErrors()", asMETHOD( vsp::ErrorMgrSingleton, GetNumTotalErrors ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "ErrorObj PopLastError()", asMETHOD( vsp::ErrorMgrSingleton, PopLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );
    r = se->RegisterGlobalFunction( "ErrorObj GetLastError()", asMETHOD( vsp::ErrorMgrSingleton, GetLastError ), asCALL_THISCALL_ASGLOBAL, &vsp::ErrorMgr );
    assert( r );

    //==== Vehicle Functions ====//
    r = se->RegisterGlobalFunction( "void Update()", asFUNCTION( vsp::Update ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ClearVSPModel()", asFUNCTION( vsp::ClearVSPModel ), asCALL_CDECL );
    assert( r >= 0 );


    //==== File I/O Functions ====//
    r = se->RegisterGlobalFunction( "void ReadVSPFile( const string & in file_name )", asFUNCTION( vsp::ReadVSPFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void WriteVSPFile( const string & in file_name, int set )", asFUNCTION( vsp::WriteVSPFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void InsertVSPFile( const string & in file_name, const string & in parent )", asFUNCTION( vsp::InsertVSPFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ExportFile( const string & in file_name, int write_set_index, int file_type )", asFUNCTION( vsp::ExportFile ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ImportFile( const string & in file_name, int file_type, const string & in parent )", asFUNCTION( vsp::ImportFile ), asCALL_CDECL );
    assert( r >= 0 );


    //==== Computations ====//
    r = se->RegisterGlobalFunction( "void SetComputationFileName( int file_type, const string & in file_name )", asFUNCTION( vsp::SetComputationFileName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputeMassProps( int set, int num_slices )", asFUNCTION( vsp::ComputeMassProps ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputeCompGeom( int set, bool half_mesh, int file_export_types )", asFUNCTION( vsp::ComputeCompGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputePlaneSlice( int set, int num_slices, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0 )", asFUNCTION( vsp::ComputePlaneSlice ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string ComputeAwaveSlice( int set, int num_slices, int num_rots, double ang_control, bool comp_ang, const vec3d & in norm, bool auto_bnd, double start_bnd = 0, double end_bnd = 0)",
                                    asFUNCTION( vsp::ComputeAwaveSlice ), asCALL_CDECL );
    assert( r >= 0 );


    //==== Results Functions ====//
    r = se->RegisterGlobalFunction( "int GetNumResults( const string & in name )", asFUNCTION( vsp::GetNumResults ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string FindResultsID( const string & in name, int index = 0 )", asFUNCTION( vsp::FindResultsID ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string FindLatestResultsID( const string & in name )", asFUNCTION( vsp::FindLatestResultsID ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumData( const string & in results_id, const string & in data_name )", asFUNCTION( vsp::GetNumData ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetAllResultsNames()", asMETHOD( ScriptMgrSingleton, GetAllResultsNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetAllDataNames(const string & in results_id )", asMETHOD( ScriptMgrSingleton, GetAllDataNames ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<int>@  GetIntResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetIntResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<double>@  GetDoubleResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetDoubleResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetStringResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetStringResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  GetVec3dResults( const string & in id, const string & in name, int index = 0 )", asMETHOD( ScriptMgrSingleton, GetVec3dResults ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string CreateGeomResults( const string & in geom_id, const string & in name )", asFUNCTION( vsp::CreateGeomResults ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteAllResults()", asFUNCTION( vsp::DeleteAllResults ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void DeleteResult( const string & in id )", asFUNCTION( vsp::DeleteResult ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "void WriteTestResults()", asMETHOD( ResultsMgrSingleton, WriteTestResults ), asCALL_THISCALL_ASGLOBAL, &ResultsMgr );
    assert( r >= 0 );


    //==== Geom Functions ====//
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomTypes()", asMETHOD( ScriptMgrSingleton, GetGeomTypes ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AddGeom( const string & in type, const string & in parent )", asFUNCTION( vsp::AddGeom ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CutGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CutGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CopyGeomToClipboard(const string & in geom_id)", asFUNCTION( vsp::CopyGeomToClipboard ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PasteGeomClipboard(const string & in parent_id)", asFUNCTION( vsp::PasteGeomClipboard ), asCALL_CDECL );
    assert( r >= 0 );

    r = se->RegisterGlobalFunction( "array<string>@  FindGeoms()", asMETHOD( ScriptMgrSingleton, FindGeoms ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  FindGeomsWithName(const string & in name)", asMETHOD( ScriptMgrSingleton, FindGeomsWithName ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetGeomName( const string & in geom_id, const string & in name )", asFUNCTION( vsp::SetGeomName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetGeomName( const string & in geom_id )", asFUNCTION( vsp::GetGeomName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomParmIDs(const string & in geom_id )", asMETHOD( ScriptMgrSingleton, GetGeomParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetParm(const string & in geom_id, const string & in name, const string & in group )", asFUNCTION( vsp::GetParm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumXSecSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumXSecSurfs ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumMainSurfs( const string & in geom_id )", asFUNCTION( vsp::GetNumMainSurfs ), asCALL_CDECL );
    assert( r >= 0 );


    //==== XSecSurf Functions ====//
    r = se->RegisterGlobalFunction( "string GetXSecSurf( const string & in geom_id, int index )", asFUNCTION( vsp::GetXSecSurf ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecType( const string & in xsec_surf_id, int type )", asFUNCTION( vsp::SetXSecType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetXSecType( const string & in xsec_surf_id )", asFUNCTION( vsp::GetXSecType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetNumXSec( const string & in xsec_surf_id )", asFUNCTION( vsp::GetNumXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::GetXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string AppendXSec( const string & in xsec_surf_id, int type )", asFUNCTION( vsp::AppendXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CutXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::CutXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void CopyXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::CopyXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void PasteXSec( const string & in xsec_surf_id, int xsec_index )", asFUNCTION( vsp::PasteXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string InsertXSec( const string & in xsec_surf_id, int type, int xsec_index )", asFUNCTION( vsp::InsertXSec ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void ChangeXSecShape( const string & in xsec_surf_id, int xsec_index, int type )", asFUNCTION( vsp::ChangeXSecShape ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecSurfGlobalXForm( const string & in xsec_surf_id, const Matrix4d & in mat )", asFUNCTION( vsp::SetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "Matrix4d GetXSecSurfGlobalXForm( const string & in xsec_surf_id )", asFUNCTION( vsp::GetXSecSurfGlobalXForm ), asCALL_CDECL );
    assert( r >= 0 );

    //==== XSec Functions ====//
    r = se->RegisterGlobalFunction( "int GetXSecShape( const string& in xsec_id )", asFUNCTION( vsp::GetXSecShape ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetXSecWidth( const string& in xsec_id )", asFUNCTION( vsp::GetXSecWidth ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetXSecHeight( const string& in xsec_id )", asFUNCTION( vsp::GetXSecHeight ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecWidthHeight( const string& in xsec_id, double w, double h )", asFUNCTION( vsp::SetXSecWidthHeight ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetXSecParmIDs(const string & in xsec_id )", asMETHOD( ScriptMgrSingleton, GetXSecParmIDs ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetXSecParm( const string& in xsec_id, const string& in name )", asFUNCTION( vsp::GetXSecParm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<vec3d>@  ReadFileXSec(const string& in xsec_id, const string& in file_name  )", asMETHOD( ScriptMgrSingleton, ReadFileXSec ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecPnts( const string& in xsec_id, array<vec3d>@ pnt_arr )", asMETHOD( ScriptMgrSingleton, SetXSecPnts ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d ComputeXSecPnt( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecPnt ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "vec3d ComputeXSecTan( const string& in xsec_id, double fract )", asFUNCTION( vsp::ComputeXSecTan ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecContinuity( const string& in xsec_id, int cx )", asFUNCTION( vsp::SetXSecContinuity ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecTanAngles( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanAngles ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecTanStrengths( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecTanStrengths ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetXSecCurvatures( const string& in xsec_id, int side, double top, double right = -1.0e12, double bottom = -1.0e12, double left = -1.0e12 )", asFUNCTION( vsp::SetXSecCurvatures ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Sets Functions ====//
    r = se->RegisterGlobalFunction( "int GetNumSets()", asFUNCTION( vsp::GetNumSets ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSetName( int index, const string& in name )", asFUNCTION( vsp::SetSetName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetSetName( int index )", asFUNCTION( vsp::GetSetName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomSetAtIndex( int index )", asMETHOD( ScriptMgrSingleton, GetGeomSetAtIndex ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "array<string>@  GetGeomSet( const string & in name )", asMETHOD( ScriptMgrSingleton, GetGeomSet ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "bool GetSetFlag( const string & in geom_id, int set_index )", asFUNCTION( vsp::GetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetSetFlag( const string & in geom_id, int set_index, bool flag )", asFUNCTION( vsp::SetSetFlag ), asCALL_CDECL );
    assert( r >= 0 );

    //==== Parm Functions ====//
    r = se->RegisterGlobalFunction( "bool ValidParm( const string & in id )", asFUNCTION( vsp::ValidParm ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmValLimits(const string & in parm_id, double val, double lower_limit, double upper_limit )",
                                    asFUNCTION( vsp::SetParmValLimits ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in parm_id, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmVal(const string & in geom_id, const string & in name, const string & in group, double val )",
                                    asFUNCTIONPR( vsp::SetParmVal, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double SetParmValUpdate(const string & in geom_id, const string & in name, const string & in group, double val )",
                                    asFUNCTIONPR( vsp::SetParmValUpdate, ( const string &, const string &, const string &, double val ), double ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetParmVal(const string & in parm_id )", asFUNCTION( vsp::GetParmVal ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetIntParmVal(const string & in parm_id )", asFUNCTION( vsp::GetIntParmVal ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "bool GetBoolParmVal(const string & in parm_id )", asFUNCTION( vsp::GetBoolParmVal ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetParmUpperLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmUpperLimit ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetParmUpperLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmUpperLimit ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetParmLowerLimit( const string & in parm_id, double val )", asFUNCTION( vsp::SetParmLowerLimit ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double GetParmLowerLimit( const string & in parm_id )", asFUNCTION( vsp::GetParmLowerLimit ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "int GetParmType( const string & in parm_id )", asFUNCTION( vsp::GetParmType ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetParmName( const string & in parm_id )", asFUNCTION( vsp::GetParmName ), asCALL_CDECL );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "string GetParmContainer( const string & in parm_id )", asFUNCTION( vsp::GetParmContainer ), asCALL_CDECL );
    assert( r >= 0 );

}

void ScriptMgrSingleton::RegisterUtility( asIScriptEngine* se )
{
    //==== Register Utility Functions ====//
    int r;
    r = se->RegisterGlobalFunction( "void Print(const string & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const string &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );  
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(const vec3d & in data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (const vec3d &, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );  
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(double data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (double, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );  
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void Print(int data, bool new_line = true )", asMETHODPR( ScriptMgrSingleton, Print, (int, bool), void ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );  
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Min( double x, double y)", asMETHOD( ScriptMgrSingleton, Min ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Max( double x, double y)", asMETHOD( ScriptMgrSingleton, Max ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Rad2Deg( double r )", asMETHOD( ScriptMgrSingleton, Rad2Deg ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "double Deg2Rad( double d )", asMETHOD( ScriptMgrSingleton, Deg2Rad ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );


    //====  Register Proxy Utility Functions ====//
    r = se->RegisterGlobalFunction( "array<vec3d>@ GetProxyVec3dArray()", asMETHOD( ScriptMgrSingleton, GetProxyVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
    r = se->RegisterGlobalFunction( "void SetVec3dArray( array<vec3d>@ arr )", asMETHOD( ScriptMgrSingleton, SetVec3dArray ), asCALL_THISCALL_ASGLOBAL, &ScriptMgr );
    assert( r >= 0 );
}


void ScriptMgrSingleton::SetVec3dArray( CScriptArray* varr )
{
    vector< vec3d > vec;
    vec.resize( varr->GetSize() );
    for ( int i = 0 ; i < ( int )varr->GetSize() ; i++ )
    {
        vec[i] = * ( vec3d* )( varr->At( i ) );
    }


}

//===== Utility Functions Vec3d Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyVec3dArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = new CScriptArray( m_ProxyVec3dArray.size(), m_Vec3dArrayType );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyVec3dArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyStringArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = new CScriptArray( m_ProxyStringArray.size(), m_StringArrayType );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyStringArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyIntArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = new CScriptArray( m_ProxyIntArray.size(), m_IntArrayType );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyIntArray[i] );
    }
    return sarr;
}

//==== Utility Functions String Proxy Array =====//
CScriptArray* ScriptMgrSingleton::GetProxyDoubleArray()
{
    //==== This Will Get Deleted By The Script Engine ====//
    CScriptArray* sarr = new CScriptArray( m_ProxyDoubleArray.size(), m_DoubleArrayType );
    for ( int i = 0 ; i < ( int )sarr->GetSize() ; i++ )
    {
        sarr->SetValue( i, &m_ProxyDoubleArray[i] );
    }
    return sarr;
}


//==== Wrappers For API Functions That Return Vectors ====//
CScriptArray* ScriptMgrSingleton::GetGeomTypes()
{
    m_ProxyStringArray = vsp::GetGeomTypes();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindGeoms()
{
    m_ProxyStringArray = vsp::FindGeoms();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::FindGeomsWithName( const string & name )
{
    m_ProxyStringArray = vsp::FindGeomsWithName( name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomParmIDs( const string & geom_id )
{
    m_ProxyStringArray = vsp::GetGeomParmIDs( geom_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetXSecParmIDs( const string & xsec_id )
{
    m_ProxyStringArray = vsp::GetXSecParmIDs( xsec_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::ReadFileXSec( const string& xsec_id, const string& file_name )
{
    m_ProxyVec3dArray = vsp::ReadFileXSec( xsec_id, file_name );
    return GetProxyVec3dArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomSetAtIndex( int index )
{
    m_ProxyStringArray = vsp::GetGeomSetAtIndex( index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetGeomSet( const string & name )
{
    m_ProxyStringArray = vsp::GetGeomSet( name );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllResultsNames()
{
    m_ProxyStringArray = vsp::GetAllResultsNames();
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetAllDataNames( const string & results_id )
{
    m_ProxyStringArray = vsp::GetAllDataNames( results_id );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetIntResults( const string & id, const string & name, int index )
{
    m_ProxyIntArray = vsp::GetIntResults( id, name, index );
    return GetProxyIntArray();
}

CScriptArray* ScriptMgrSingleton::GetDoubleResults( const string & id, const string & name, int index )
{
    m_ProxyDoubleArray = vsp::GetDoubleResults( id, name, index );
    return GetProxyDoubleArray();
}

CScriptArray* ScriptMgrSingleton::GetStringResults( const string & id, const string & name, int index )
{
    m_ProxyStringArray = vsp::GetStringResults( id, name, index );
    return GetProxyStringArray();
}

CScriptArray* ScriptMgrSingleton::GetVec3dResults( const string & id, const string & name, int index )
{
    m_ProxyVec3dArray = vsp::GetVec3dResults( id, name, index );
    return GetProxyVec3dArray();
}

void ScriptMgrSingleton::SetXSecPnts( const string& xsec_id, CScriptArray* pnt_arr )
{
    vector< vec3d > pnt_vec;
    pnt_vec.resize( pnt_arr->GetSize() );
    for ( int i = 0 ; i < ( int )pnt_arr->GetSize() ; i++ )
    {
        pnt_vec[i] = * ( vec3d* )( pnt_arr->At( i ) );
    }
    vsp::SetXSecPnts( xsec_id, pnt_vec );
}

//==== Console Print String Data ====//
void ScriptMgrSingleton::Print( const string & data, bool new_line )
{
    printf( " %s ", data.c_str() );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Vec3d Data ====//
void ScriptMgrSingleton::Print( const vec3d & data, bool new_line )
{
    printf( " %f, %f, %f ", data.x(), data.y(), data.z() );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Double Data ====//
void ScriptMgrSingleton::Print( double data, bool new_line )
{
    printf( " %f ", data );
    if ( new_line ) printf( "\n" );
}

//==== Console Print Int Data ====//
void ScriptMgrSingleton::Print( int data, bool new_line )
{
    printf( " %d ", data );
    if ( new_line ) printf( "\n" );
}
