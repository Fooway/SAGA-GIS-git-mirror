/**********************************************************
 * Version $Id: gdal_import_wms.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library                     //
//                                                       //
//                       io_gdal                         //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                  gdal_import_wms.cpp                  //
//                                                       //
//            Copyright (C) 2015 O. Conrad               //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.de                    //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Hamburg                  //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "gdal_import_wms.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGDAL_Import_WMS::CGDAL_Import_WMS(void)
{
	//-----------------------------------------------------
	Set_Name	(_TL("Import Open Street Map Image"));

	Set_Author	("O.Conrad (c) 2016");

	CSG_String	Description;

	Description	= _TW(
		"The \"Import OSM Image\" tool imports a map image from a Web Map Service (WMS) using the "
		"\"Geospatial Data Abstraction Library\" (GDAL) by Frank Warmerdam. "
		"For more information have a look at the GDAL homepage:\n"
		"  <a target=\"_blank\" href=\"http://www.gdal.org/\">"
		"  http://www.gdal.org</a>\n"
	);

	Description	+= CSG_String::Format("\nGDAL %s:%s\n\n", _TL("Version"), SG_Get_GDAL_Drivers().Get_Version().c_str());

	Set_Description(Description);

	//-----------------------------------------------------
	Parameters.Add_Grid(
		NULL	, "TARGET"	, _TL("Target System"),
		_TL(""),
		PARAMETER_INPUT_OPTIONAL
	);

	Parameters.Add_Grid_Output(
		NULL	, "MAP"		, _TL("Map"),
		_TL("")
	);

	Parameters.Add_Choice(
		NULL	, "SERVER"	, _TL("Server"),
		_TL(""),
		CSG_String::Format("%s|%s|",
			_TL("Open Street Map"),
			_TL("MapQuest")
		), 0
	);

	Parameters.Add_Range(NULL	, "XRANGE"	, _TL("X-Range" ), _TL(""), -20037508.34, 20037508.34, -20037508.34, true, 20037508.34, true);
	Parameters.Add_Range(NULL	, "YRANGE"	, _TL("Y-Range" ), _TL(""), -20037508.34, 20037508.34, -20037508.34, true, 20037508.34, true);
	Parameters.Add_Value(NULL	, "NX"		, _TL("Columns" ), _TL(""), PARAMETER_TYPE_Int   , 600, 1, true);
//	Parameters.Add_Value(NULL	, "NY"		, _TL("Rows"    ), _TL(""), PARAMETER_TYPE_Int   , 500, 1, true);
//	Parameters.Add_Value(NULL	, "CELLSIZE", _TL("Cellsize"), _TL(""), PARAMETER_TYPE_Double, 1.0, 0.0, true);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGDAL_Import_WMS::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	return( CSG_Module::On_Parameter_Changed(pParameters, pParameter) );
}

//---------------------------------------------------------
int CGDAL_Import_WMS::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "TARGET") )
	{
		pParameters->Set_Enabled("XRANGE", pParameter->asGrid() == NULL);
		pParameters->Set_Enabled("YRANGE", pParameter->asGrid() == NULL);
		pParameters->Set_Enabled("NX "   , pParameter->asGrid() == NULL);
	}

	return( CSG_Module::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGDAL_Import_WMS::On_Execute(void)
{
	//-----------------------------------------------------
	CSG_Grid_System	System;

	if( !Get_System(System, Parameters("TARGET")->asGrid()) )
	{
		return( false );
	}

	//-----------------------------------------------------
	CSG_Grid	*pBands[3];

	if( !Get_Bands(pBands, System) )
	{
		Error_Set(_TL("failed to retrieve map image data"));

		return( false );
	}

	//-----------------------------------------------------
	if( Parameters("TARGET")->asGrid() )
	{
		Get_Projected(pBands, Parameters("TARGET")->asGrid());
	}

	//-----------------------------------------------------
	return( Set_Image(pBands) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGDAL_Import_WMS::Get_System(CSG_Grid_System &System, CSG_Grid *pTarget)
{
	//-----------------------------------------------------
	if( !pTarget )
	{
		CSG_Rect	Extent(
			Parameters("XRANGE")->asRange()->Get_LoVal(),
			Parameters("YRANGE")->asRange()->Get_LoVal(),
			Parameters("XRANGE")->asRange()->Get_HiVal(),
			Parameters("YRANGE")->asRange()->Get_HiVal()
		);

		double	Cellsize	= Extent.Get_XRange() / Parameters("NX")->asDouble();

		return( System.Assign(Cellsize, Extent) );
	}

	//-----------------------------------------------------
	if( !pTarget->Get_Projection().is_Okay() )
	{
		return( false );
	}

	CSG_Shapes	rTarget(SHAPE_TYPE_Point), rSource;

	rTarget.Get_Projection()	= pTarget->Get_Projection();

	rTarget.Add_Shape()->Add_Point(pTarget->Get_XMin(true), pTarget->Get_YMin(true));
	rTarget.Add_Shape()->Add_Point(pTarget->Get_XMin(true), pTarget->Get_YMax(true));
	rTarget.Add_Shape()->Add_Point(pTarget->Get_XMax(true), pTarget->Get_YMax(true));
	rTarget.Add_Shape()->Add_Point(pTarget->Get_XMax(true), pTarget->Get_YMin(true));

	//-----------------------------------------------------
	CSG_Module	*pModule	= SG_Get_Module_Library_Manager().Get_Module("pj_proj4", 2);	// Coordinate Transformation (Shapes);

	if(	!pModule )
	{
		return( false );
	}

	pModule->Settings_Push();

	if( SG_MODULE_PARAMETER_SET("CRS_PROJ4" , SG_T("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +k=1.0"))
	&&  SG_MODULE_PARAMETER_SET("SOURCE"    , &rTarget)
	&&  SG_MODULE_PARAMETER_SET("TARGET"    , &rSource)
	&&  pModule->Execute() )
	{
		double	Cellsize	= rSource.Get_Extent().Get_XRange() / Parameters("NX")->asDouble();

		System.Assign(Cellsize, rSource.Get_Extent());

		pModule->Settings_Pop();

		return( true );
	}

	pModule->Settings_Pop();

	return( false );
}

///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGDAL_Import_WMS::Get_Projected(CSG_Grid *pBands[3], CSG_Grid *pTarget)
{
	CSG_Module	*pModule	= SG_Get_Module_Library_Manager().Get_Module("pj_proj4", 3);	// Coordinate Transformation (Grid List);

	if(	!pModule )
	{
		return( false );
	}

	//-----------------------------------------------------
	pModule->Settings_Push();

	if( SG_MODULE_PARAMETER_SET("CRS_PROJ4" , pTarget->Get_Projection().Get_Proj4())
	&&  SG_MODULE_PARAMETER_SET("RESAMPLING", 3)
	&&  SG_MODULE_PARAMLIST_ADD("SOURCE"    , pBands[0])
	&&  SG_MODULE_PARAMLIST_ADD("SOURCE"    , pBands[1])
	&&  SG_MODULE_PARAMLIST_ADD("SOURCE"    , pBands[2])
	&&  pModule->Get_Parameters("TARGET")->Get_Parameter("DEFINITION")->Set_Value(1)
	&&  pModule->Get_Parameters("TARGET")->Get_Parameter("SYSTEM")->asGrid_System()->Assign(pTarget->Get_System())
	&&  pModule->Execute() )
	{
		CSG_Parameter_Grid_List	*pGrids	= pModule->Get_Parameters()->Get_Parameter("GRIDS")->asGridList();

		delete(pBands[0]);	pBands[0]	= pGrids->asGrid(0);
		delete(pBands[1]);	pBands[1]	= pGrids->asGrid(1);
		delete(pBands[2]);	pBands[2]	= pGrids->asGrid(2);

		pModule->Settings_Pop();

		return( true );
	}

	pModule->Settings_Pop();

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGDAL_Import_WMS::Set_Image(CSG_Grid *pBands[3])
{
	//-----------------------------------------------------
	CSG_Grid	*pMap	= SG_Create_Grid(pBands[0]->Get_System(), SG_DATATYPE_Int);

	pMap->Get_Projection()	= pBands[0]->Get_Projection();

	#pragma omp parallel for
	for(int y=0; y<pMap->Get_NY(); y++)	for(int x=0; x<pMap->Get_NX(); x++)
	{
		pMap->Set_Value(x, y, SG_GET_RGB(pBands[0]->asInt(x, y), pBands[1]->asInt(x, y), pBands[2]->asInt(x, y)));
	}

	delete(pBands[0]);
	delete(pBands[1]);
	delete(pBands[2]);

	pMap->Set_Name(_TL("Open Street Map"));

	DataObject_Add(pMap);
	DataObject_Set_Parameter(pMap, "COLORS_TYPE", 6);	// RGB Coded Values

	Parameters("MAP")->Set_Value(pMap);

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGDAL_Import_WMS::Get_Bands(CSG_Grid *pBands[3], const CSG_Grid_System &System)
{
	//-----------------------------------------------------
	CSG_String	Server;

	switch( Parameters("SERVER")->asInt() )
	{
	default:	Server	= "http://tile.openstreetmap.org"          ;	break;
	case  1:	Server	= "http://otile1.mqcdn.com/tiles/1.0.0/osm";	break;	// MapQuest
	}

	//-----------------------------------------------------
	CSG_GDAL_DataSet	DataSet;

	if( DataSet.Open_Read(Get_Request(Server), System) == false || DataSet.Get_Count() != 3 )
	{
		return( false );
	}

	Message_Add("\n", false);
	Message_Add(CSG_String::Format("\n%s: %s", _TL("Driver" ), DataSet.Get_DriverID().c_str()), false);
	Message_Add(CSG_String::Format("\n%s: %d", _TL("Bands"  ), DataSet.Get_Count()           ), false);
	Message_Add(CSG_String::Format("\n%s: %d", _TL("Rows"   ), DataSet.Get_NX()              ), false);
	Message_Add(CSG_String::Format("\n%s: %d", _TL("Columns"), DataSet.Get_NY()              ), false);
	Message_Add("\n", false);

	//-----------------------------------------------------
	SG_UI_Progress_Lock(true);

	pBands[0]	= DataSet.Read(0);
	pBands[1]	= DataSet.Read(1);
	pBands[2]	= DataSet.Read(2);

	SG_UI_Progress_Lock(false);

	//-----------------------------------------------------
	if( !pBands[0] || !pBands[1] || !pBands[2] )
	{
		if( pBands[0] )	delete(pBands[0]);
		if( pBands[1] )	delete(pBands[1]);
		if( pBands[2] )	delete(pBands[2]);

		return( false );
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_String CGDAL_Import_WMS::Get_Request(const CSG_String &Server)
{
	CSG_MetaData	XML, *pEntry;

	XML.Set_Name("GDAL_WMS");

	//-----------------------------------------------------
	pEntry	= XML.Add_Child("Service");	pEntry->Add_Property("name" , "TMS");

	pEntry->Add_Child("ServerUrl"  , Server + "/${z}/${x}/${y}.png");

	//-----------------------------------------------------
	pEntry	= XML.Add_Child("DataWindow");		// Define size and extents of the data. (required, except for TiledWMS and VirtualEarth)

	pEntry->Add_Child("UpperLeftX" , -20037508.34);		// X (longitude) coordinate of upper-left corner. (optional, defaults to -180.0, except for VirtualEarth)
	pEntry->Add_Child("UpperLeftY" ,  20037508.34);		// Y (latitude) coordinate of upper-left corner. (optional, defaults to 90.0, except for VirtualEarth)
	pEntry->Add_Child("LowerRightX",  20037508.34);		// X (longitude) coordinate of lower-right corner. (optional, defaults to 180.0, except for VirtualEarth)
	pEntry->Add_Child("LowerRightY", -20037508.34);		// Y (latitude) coordinate of lower-right corner. (optional, defaults to -90.0, except for VirtualEarth)
	pEntry->Add_Child("TileLevel"  ,           18);		// Tile level at highest resolution. (tiled image sources only, optional, defaults to 0)
	pEntry->Add_Child("TileCountX" ,            1);		// Can be used to define image size, SizeX = TileCountX * BlockSizeX * 2TileLevel. (tiled image sources only, optional, defaults to 0)
	pEntry->Add_Child("TileCountY" ,            1);		// Can be used to define image size, SizeY = TileCountY * BlockSizeY * 2TileLevel. (tiled image sources only, optional, defaults to 0)
	pEntry->Add_Child("YOrigin"    ,        "top");		// Can be used to define the position of the Y origin with respect to the tile grid. Possible values are 'top', 'bottom', and 'default', where the default behavior is mini-driver-specific. (TMS mini-driver only, optional, defaults to 'bottom' for TMS)

	//-----------------------------------------------------
	pEntry	= XML.Add_Child("Projection", "EPSG:3857");	// Image projection (optional, defaults to value reported by mini-driver or EPSG:4326)
	pEntry	= XML.Add_Child("BandsCount",           3);	// Number of bands/channels, 1 for grayscale data, 3 for RGB, 4 for RGBA. (optional, defaults to 3)
	pEntry	= XML.Add_Child("BlockSizeX",         256);	// Block size in pixels. (optional, defaults to 1024, except for VirtualEarth)
	pEntry	= XML.Add_Child("BlockSizeY",         256);

	//-----------------------------------------------------
	pEntry	= XML.Add_Child("Cache");

	pEntry->Add_Child("Path", SG_File_Make_Path(SG_Dir_Get_Temp(), SG_T("gdalwmscache")));

	//-----------------------------------------------------
	return( XML.asText(2) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------