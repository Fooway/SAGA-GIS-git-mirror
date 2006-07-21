
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                        Grid_IO                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     USGS_SRTM.cpp                     //
//                                                       //
//                 Copyright (C) 2003 by                 //
//                      Olaf Conrad                      //
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
// 59 Temple Place - Suite 330, Boston, MA 02111-1307,   //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <string.h>

#include "usgs_srtm.h"


///////////////////////////////////////////////////////////
//														 //
//						Import							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CUSGS_SRTM_Import::CUSGS_SRTM_Import(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name(_TL("Import USGS SRTM Grid"));

	Set_Author(_TL("Copyrights (c) 2004 by Olaf Conrad"));

	Set_Description(_TL(
		"Import grid from USGS SRTM (Shuttle Radar Topography Mission) data.\n"

		"You find data and further information at:\n"
		"  ftp://edcsgs9.cr.usgs.gov/pub/data/srtm/\n"
		"  http://seamless.usgs.gov/\n"
		"  http://www.jpl.nasa.gov/srtm/\n"

		"\nFarr, T.G., M. Kobrick (2000):\n"
		"  'Shuttle Radar Topography Mission produces a wealth of data',\n"
		"  Amer. Geophys. Union Eos, v. 81, p. 583-585\n"

		"\nRosen, P.A., S. Hensley, I.R. Joughin, F.K. Li, S.N. Madsen, E. Rodriguez, R.M. Goldstein (2000):\n"
		"  'Synthetic aperture radar interferometry'\n"
		"  Proc. IEEE, v. 88, p. 333-382\n")
	);


	//-----------------------------------------------------
	// 2. Parameters...

	Parameters.Add_Grid_Output(
		NULL	, "GRID"	, _TL("Grid"),
		""
	);

	Parameters.Add_FilePath(
		NULL	, "FILE"		, _TL("File"),
		"",
		_TL("USGS SRTM Grids (*.hgt)|*.hgt|All Files|*.*")
	);

	Parameters.Add_Choice(
		NULL	, "RESOLUTION"	, _TL("Resolution"),
		"",
		_TL(
		"1 arc-second|"
		"3 arc-second|"),	1
	);
}

//---------------------------------------------------------
CUSGS_SRTM_Import::~CUSGS_SRTM_Import(void)
{}

//---------------------------------------------------------
bool CUSGS_SRTM_Import::On_Execute(void)
{
	int			x, y, N;
	short		*sLine;
	double		xMin, yMin, D;
	FILE		*Stream;
	CSG_String	fName;
	CGrid		*pGrid;

	//-----------------------------------------------------
	pGrid	= NULL;

	//-----------------------------------------------------
	fName	= SG_File_Get_Name(Parameters("FILE")->asString(), false);
	fName.Make_Upper();

	yMin	= (fName[0] == 'N' ?  1.0 : -1.0) * atoi(fName.c_str() + 1);
	xMin	= (fName[3] == 'W' ? -1.0 :  1.0) * atoi(fName.c_str() + 4);

	//-----------------------------------------------------
	switch( Parameters("RESOLUTION")->asInt() )
	{
	default:
		return( false );

	case 0:	// 1 arcsec...
		N			= 3601;
		D			= 1.0 / 3600.0;
		break;

	case 1:	// 3 arcsec...
		N			= 1201;
		D			= 3.0 / 3600.0;
		break;
	}

	//-----------------------------------------------------
	if( (Stream = fopen(Parameters("FILE")->asString(), "rb")) != NULL )
	{
		if( (pGrid = SG_Create_Grid(GRID_TYPE_Float, N, N, D, xMin, yMin)) != NULL )
		{
			pGrid->Set_Name			(fName);
			pGrid->Set_NoData_Value	(-32768);

			//---------------------------------------------
			sLine	= (short *)SG_Malloc(N * sizeof(short));

			for(y=0; y<N && !feof(Stream) && Set_Progress(y, N); y++)
			{
				fread(sLine, N, sizeof(short), Stream);

				for(x=0; x<N; x++)
				{
					SG_Swap_Bytes(sLine + x, sizeof(short));

					pGrid->Set_Value(x, N - 1 - y, sLine[x]);
				}
			}

			SG_Free(sLine);

			Parameters("GRID")->Set_Value(pGrid);
		}

		fclose(Stream);
	}

	return( pGrid != NULL );
}
