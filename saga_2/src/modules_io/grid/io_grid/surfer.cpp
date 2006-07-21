
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
//                      Surfer.cpp                       //
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

#include "surfer.h"

//---------------------------------------------------------
#define NODATAVALUE	1.70141e38f


///////////////////////////////////////////////////////////
//														 //
//						Import							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSurfer_Import::CSurfer_Import(void)
{
	CParameter	*pNode;

	//-----------------------------------------------------
	// 1. Info...

	Set_Name(_TL("Import Surfer Grid"));

	Set_Author(_TL("Copyrights (c) 2001 by Olaf Conrad"));

	Set_Description(_TL(
		"Import grid from Golden Software's Surfer grid format.\n")
	);


	//-----------------------------------------------------
	// 2. Parameters...

	Parameters.Add_Grid_Output(
		NULL	, "GRID"	, _TL("Grid"),
		""
	);

	Parameters.Add_FilePath(
		NULL	, "FILE"	, _TL("File"),
		"",
		_TL("Surfer Grid (*.grd)|*.grd|All Files|*.*")
	);

	pNode	= Parameters.Add_Choice(
		NULL	, "NODATA"	, _TL("No Data Value"),
		"",

		CSG_String::Format("%s|%s|",
			_TL("Surfer's No Data Value"),
			_TL("User Defined")
		), 0
	);

	Parameters.Add_Value(
		pNode	, "NODATA_VAL"	, _TL("User Defined No Data Value"),
		"",
		PARAMETER_TYPE_Double	, -99999.0
	);
}

//---------------------------------------------------------
CSurfer_Import::~CSurfer_Import(void)
{}

//---------------------------------------------------------
bool CSurfer_Import::On_Execute(void)
{
	int		x, y, NX, NY;
	short	sValue;
	long	lValue;
	float	*fLine;
	double	*dLine, dValue, DX, DY, xMin, yMin;
	FILE	*Stream;
	CGrid	*pGrid;

	//-----------------------------------------------------
	pGrid	= NULL;

	//-----------------------------------------------------
	if( Parameters("FILE")->asString() && (Stream = fopen(Parameters("FILE")->asString(), "rb")) != NULL )
	{
		fread(&lValue, 1, sizeof(long), Stream);

		//-------------------------------------------------
		// Surfer 7: Binary...

		if( !strncmp((char *)&lValue, "DSRB", 4) )
		{
			fread(&lValue, 1, sizeof(long)	, Stream);			// SectionSize...
			fread(&lValue, 1, sizeof(long)	, Stream);			// Version
			fread(&lValue, 1, sizeof(long)	, Stream);

			if( lValue == 0x44495247 )							// Grid-Header...
			{
				fread(&lValue	, 1, sizeof(long)	, Stream);	// SectionSize...

				fread(&lValue	, 1, sizeof(long)	, Stream);	// NX...
				NY		= (int)lValue;
				fread(&lValue	, 1, sizeof(long)	, Stream);	// NY...
				NX		= (int)lValue;

				fread(&xMin		, 1, sizeof(double)	, Stream);	// xMin...
				fread(&yMin		, 1, sizeof(double)	, Stream);	// yMin...

				fread(&DX		, 1, sizeof(double)	, Stream);	// DX...
				fread(&DY		, 1, sizeof(double)	, Stream);	// DY...

				fread(&dValue	, 1, sizeof(double)	, Stream);	// zMin...
				fread(&dValue	, 1, sizeof(double)	, Stream);	// zMax...

				fread(&dValue	, 1, sizeof(double)	, Stream);	// Rotation (unused)...
				fread(&dValue	, 1, sizeof(double)	, Stream);	// Blank Value...
				fread(&lValue	, 1, sizeof(long)	, Stream);	// ???...

				if( lValue == 0x41544144 )						// Load Binary Double...
				{
					fread(&lValue, 1, sizeof(long)	, Stream);	// SectionSize...

					//-------------------------------------
					if( !feof(Stream) && (pGrid = SG_Create_Grid(GRID_TYPE_Double, NX, NY, DX, xMin, yMin)) != NULL )
					{
						dLine	= (double *)SG_Malloc(pGrid->Get_NX() * sizeof(double));

						for(y=0; y<pGrid->Get_NY() && !feof(Stream) && Set_Progress(y, pGrid->Get_NY()); y++)
						{
							fread(dLine, pGrid->Get_NX(), sizeof(double), Stream);

							for(x=0; x<pGrid->Get_NX(); x++)
							{
								pGrid->Set_Value(x, y, dLine[x]);
							}
						}

						SG_Free(dLine);
					}
				}
			}
		}

		//-------------------------------------------------
		// Surfer 6: Binary...

		else if( !strncmp((char *)&lValue, "DSBB", 4) )
		{
			fread(&sValue	, 1, sizeof(short)	, Stream);
			NX		= sValue;
			fread(&sValue	, 1, sizeof(short)	, Stream);
			NY		= sValue;

			fread(&xMin		, 1, sizeof(double)	, Stream);
			fread(&dValue	, 1, sizeof(double)	, Stream);	// XMax
			DX		= (dValue - xMin) / (NX - 1.0);

			fread(&yMin		, 1, sizeof(double)	, Stream);
			fread(&dValue	, 1, sizeof(double)	, Stream);	// YMax...
			DY		= (dValue - yMin) / (NY - 1.0);

			fread(&dValue	, 1, sizeof(double)	, Stream);	// ZMin...
			fread(&dValue	, 1, sizeof(double)	, Stream);	// ZMax...

			//---------------------------------------------
			if( !feof(Stream) && (pGrid = SG_Create_Grid(GRID_TYPE_Float, NX, NY, DX, xMin, yMin)) != NULL )
			{
				fLine	= (float *)SG_Malloc(pGrid->Get_NX() * sizeof(float));

				for(y=0; y<pGrid->Get_NY() && !feof(Stream) && Set_Progress(y, pGrid->Get_NY()); y++)
				{
					fread(fLine, pGrid->Get_NX(), sizeof(float), Stream);

					for(x=0; x<pGrid->Get_NX(); x++)
					{
						pGrid->Set_Value(x, y, fLine[x]);
					}
				}

				SG_Free(fLine);
			}
		}

		//-------------------------------------------------
		// Surfer 6: ASCII...

		else if( !strncmp((char *)&lValue, "DSAA", 4) )
		{
			fscanf(Stream, "%d %d"	, &NX	, &NY);

			fscanf(Stream, "%lf %lf", &xMin	, &dValue);
			DX		= (dValue - xMin) / (NX - 1.0);

			fscanf(Stream, "%lf %lf", &yMin	, &dValue);
			DY		= (dValue - yMin) / (NY - 1.0);

			fscanf(Stream, "%lf %lf", &dValue, &dValue);

			//---------------------------------------------
			if( !feof(Stream) && (pGrid = SG_Create_Grid(GRID_TYPE_Float, NX, NY, DX, xMin, yMin)) != NULL )
			{
				for(y=0; y<pGrid->Get_NY() && !feof(Stream) && Set_Progress(y, pGrid->Get_NY()); y++)
				{
					for(x=0; x<pGrid->Get_NX(); x++)
					{
						fscanf(Stream, "%lf", &dValue);

						pGrid->Set_Value(x, y, dValue);
					}
				}
			}
		}

		fclose(Stream);
	}

	//-----------------------------------------------------
	if( pGrid )
	{
		pGrid->Set_Name(Parameters("FILE")->asString());
		pGrid->Set_NoData_Value(Parameters("NODATA")->asInt() == 0 ? NODATAVALUE : Parameters("NODATA_VAL")->asDouble());

		Parameters("GRID")->Set_Value(pGrid);
	}

	return( pGrid != NULL );
}


///////////////////////////////////////////////////////////
//														 //
//						Export							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSurfer_Export::CSurfer_Export(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name(_TL("Export Surfer Grid"));

	Set_Author(_TL("Copyrights (c) 2001 by Olaf Conrad"));

	Set_Description(_TL(
		"Export grid to Golden Software's Surfer grid format.\n")
	);


	//-----------------------------------------------------
	// 2. Parameters...

	Parameters.Add_Grid(
		NULL	, "GRID"	, _TL("Grid"),
		"",
		PARAMETER_INPUT
	);

	Parameters.Add_FilePath(
		NULL	, "FILE"	, _TL("File"),
		"",
		_TL(
		"Surfer Grid (*.grd)|*.grd|All Files|*.*"), NULL, true
	);

	Parameters.Add_Choice(
		NULL	, "FORMAT"	, _TL("Format"),
		"",
		_TL(
		"Binary|"
		"ASCII|"), 0
	);

	Parameters.Add_Value(
		NULL	, "NODATA"	, _TL("Use Surfer's No-Data Value"),
		"",
		PARAMETER_TYPE_Bool	, false
	);
}

//---------------------------------------------------------
CSurfer_Export::~CSurfer_Export(void)
{}

//---------------------------------------------------------
bool CSurfer_Export::On_Execute(void)
{
	const char	ID_BINARY[]	= "DSBB";

	bool	bNoData;
	short	sValue;
	int		x, y;
	float	*fLine;
	double	dValue;
	FILE	*Stream;
	CGrid	*pGrid;

	//-----------------------------------------------------
	pGrid	= Parameters("GRID")->asGrid();
	bNoData	= Parameters("NODATA")->asBool();

	switch( Parameters("FORMAT")->asInt() )
	{
	//-----------------------------------------------------
	case 0:	// Surfer 6 - Binary...

		if( (Stream = fopen(Parameters("FILE")->asString(), "wb")) != NULL )
		{
			fwrite(ID_BINARY, 4, sizeof(char  ), Stream);

			sValue	= (short)pGrid->Get_NX();
			fwrite(&sValue	, 1, sizeof(short ), Stream);
			sValue	= (short)pGrid->Get_NY();
			fwrite(&sValue	, 1, sizeof(short ), Stream);

			dValue	= pGrid->Get_XMin();
			fwrite(&dValue	, 1, sizeof(double), Stream);
			dValue	= pGrid->Get_XMax();
			fwrite(&dValue	, 1, sizeof(double), Stream);

			dValue	= pGrid->Get_YMin();
			fwrite(&dValue	, 1, sizeof(double), Stream);
			dValue	= pGrid->Get_YMax();
			fwrite(&dValue	, 1, sizeof(double), Stream);

			dValue	= pGrid->Get_ZMin();
			fwrite(&dValue	, 1, sizeof(double), Stream);
			dValue	= pGrid->Get_ZMax();
			fwrite(&dValue	, 1, sizeof(double), Stream);

			//---------------------------------------------
			fLine	= (float *)SG_Malloc(pGrid->Get_NX() * sizeof(float));

			for(y=0; y<pGrid->Get_NY() && Set_Progress(y, pGrid->Get_NY()); y++)
			{
				for(x=0; x<pGrid->Get_NX(); x++)
				{
					fLine[x]	= bNoData && pGrid->is_NoData(x, y) ? NODATAVALUE : pGrid->asFloat(x, y);
				}

				fwrite(fLine, pGrid->Get_NX(), sizeof(float), Stream);
			}

			SG_Free(fLine);

			fclose(Stream);

			return( true );
		}
		break;

	//-----------------------------------------------------
	case 1:	// Surfer - ASCII...

		if( (Stream = fopen(Parameters("FILE")->asString(), "w")) != NULL )
		{
			fprintf(Stream, "DSAA\n" );
			fprintf(Stream, "%d %d\n", pGrid->Get_NX()	, pGrid->Get_NY()	);
			fprintf(Stream, "%f %f\n", pGrid->Get_XMin(), pGrid->Get_XMax()	);
			fprintf(Stream, "%f %f\n", pGrid->Get_YMin(), pGrid->Get_YMax()	);
			fprintf(Stream, "%f %f\n", pGrid->Get_ZMin(), pGrid->Get_ZMax()	);

			//---------------------------------------------
			for(y=0; y<pGrid->Get_NY() && Set_Progress(y, pGrid->Get_NY()); y++)
			{
				for(x=0; x<pGrid->Get_NX(); x++)
				{
					if( bNoData && pGrid->is_NoData(x, y) )
					{
						fprintf(Stream, "1.70141e38 ");
					}
					else
					{
						fprintf(Stream, "%f ", pGrid->asFloat(x, y));
					}
				}

				fprintf(Stream, "\n");
			}

			fclose(Stream);

			return( true );
		}
		break;
	}

	//-----------------------------------------------------
	return( false );
}
