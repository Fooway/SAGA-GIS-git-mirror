
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       Shapes_IO                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                       Gstat.cpp                       //
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

#include "gstat.h"


///////////////////////////////////////////////////////////
//														 //
//						Export							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGStat_Export::CGStat_Export(void)
{
	CParameter	*pNode_0;

	//-----------------------------------------------------
	Set_Name(_TL("Export GStat Shapes"));

	Set_Author(_TL("Copyrights (c) 2003 by Olaf Conrad"));

	Set_Description(
		_TL("GStat shapes format export.")
	);

	//-----------------------------------------------------
	pNode_0	= Parameters.Add_Shapes(
		NULL	, "SHAPES"	, _TL("Shapes"),
		"",
		PARAMETER_INPUT
	);

	pNode_0	= Parameters.Add_FilePath(
		NULL	, "FILENAME", _TL("File"),
		"",
		_TL(
		"GStat Files (*.gstat)"	"|*.gstat|"
		"Text Files (*.txt)"	"|*.txt|"
		"All Files"				"|*.*"),
		NULL, true
	);
}

//---------------------------------------------------------
CGStat_Export::~CGStat_Export(void)
{}

//---------------------------------------------------------
bool CGStat_Export::On_Execute(void)
{
	int			iShape, iPart, iPoint, iField;
	FILE		*Stream;
	TGEO_Point	Point;
	CShape		*pShape;
	CShapes		*pShapes;

	//-----------------------------------------------------
	pShapes		= Parameters("SHAPES")->asShapes();

	//-----------------------------------------------------
	if( (Stream = fopen(Parameters("FILENAME")->asString(), "w")) != NULL )
	{
		switch( pShapes->Get_Type() )
		{
		//-------------------------------------------------
		case SHAPE_TYPE_Point:
			fprintf(Stream, "%s (created by DiGeM 2.0)\n%d\nX-Coordinate\nY-Coordinate",
				Parameters("FILENAME")->asString(),
				pShapes->Get_Table().Get_Field_Count() + 2
			);

			for(iField=0; iField<pShapes->Get_Table().Get_Field_Count(); iField++)
			{
				if( pShapes->Get_Table().Get_Field_Type(iField) == TABLE_FIELDTYPE_String )
				{
					fprintf(Stream, "\n%%%s",	pShapes->Get_Table().Get_Field_Name(iField) );
				}
				else
				{
					fprintf(Stream, "\n%s",	pShapes->Get_Table().Get_Field_Name(iField) );
				}
			}

			for(iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape, pShapes->Get_Count()); iShape++)
			{
				pShape	= pShapes->Get_Shape(iShape);

				for(iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
				{
					for(iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
					{
						Point	= pShape->Get_Point(iPoint, iPart);
						fprintf(Stream, "\n%f\t%f", Point.x, Point.y);

						for(iField=0; iField<pShapes->Get_Table().Get_Field_Count(); iField++)
						{
							if( pShapes->Get_Table().Get_Field_Type(iField) == TABLE_FIELDTYPE_String )
							{
								fprintf(Stream, "\t\"%s\"",	pShape->Get_Record()->asString(iField) );
							}
							else
							{
								fprintf(Stream, "\t%f",		pShape->Get_Record()->asDouble(iField) );
							}
						}
					}
				}
			}
			break;

		//-------------------------------------------------
		case SHAPE_TYPE_Line:
			fprintf(Stream, "EXP %s\nARC ", pShapes->Get_Name());

			for(iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape,pShapes->Get_Count()); iShape++)
			{
				pShape	= pShapes->Get_Shape(iShape);

				for(iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
				{
					//I_ok...
					fprintf(Stream, "%d ", iShape + 1);
					// dummy_I dummy_I dummy_I dummy_I dummy_I...
					fprintf(Stream, "1 2 3 4 5 ");
					// I_np...
					fprintf(Stream, "%d ", pShape->Get_Point_Count(iPart));

					for(iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
					{
						Point	= pShape->Get_Point(iPoint, iPart);
						fprintf(Stream, "%f %f ", Point.x, Point.y);
					}
				}
			}
			break;

		//-------------------------------------------------
		case SHAPE_TYPE_Polygon:
			fprintf(Stream, "EXP %s\nARC ", pShapes->Get_Name());

			for(iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape, pShapes->Get_Count()); iShape++)
			{
				pShape	= pShapes->Get_Shape(iShape);

				for(iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
				{
					//I_ok...
					fprintf(Stream, "%d ", iShape + 1);
					// dummy_I dummy_I dummy_I dummy_I dummy_I...
					fprintf(Stream, "1 2 3 4 5 ");
					// I_np...
					fprintf(Stream, "%d ", pShape->Get_Point_Count(iPart));

					for(iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
					{
						Point	= pShape->Get_Point(iPoint, iPart);
						fprintf(Stream, "%f %f ", Point.x, Point.y);
					}
				}
			}
			break;
		}

		fclose(Stream);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//						Import							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGStat_Import::CGStat_Import(void)
{
	CParameter	*pNode_0;

	//-----------------------------------------------------
	Set_Name(_TL("Import GStat Shapes"));

	Set_Author(_TL("Copyrights (c) 2003 by Olaf Conrad"));

	Set_Description(
		_TL("GStat shapes format import.")
	);

	//-----------------------------------------------------
	pNode_0	= Parameters.Add_Shapes(
		NULL	, "SHAPES"	, _TL("Shapes"),
		"",
		PARAMETER_OUTPUT
	);

	pNode_0	= Parameters.Add_FilePath(
		NULL	, "FILENAME", _TL("File"),
		"",
		_TL(
		"GStat Files (*.gstat)"	"|*.gstat|"
		"Text Files (*.txt)"	"|*.txt|"
		"All Files"				"|*.*"),

		NULL, false
	);
}

//---------------------------------------------------------
CGStat_Import::~CGStat_Import(void)
{}

//---------------------------------------------------------
bool CGStat_Import::On_Execute(void)
{
	char		c[3];
	int			i, nFields, fLength;
	double		x, y, Value;
	FILE		*Stream;
	CAPI_String	s;
	CShape		*pShape;
	CShapes		*pShapes;

	//-----------------------------------------------------
	pShapes		= Parameters("SHAPES")->asShapes();

	//-----------------------------------------------------
	if( (Stream = fopen(Parameters("FILENAME")->asString(), "rb")) != NULL )
	{
		fseek(Stream, 0, SEEK_END);
		fLength	= ftell(Stream);
		fseek(Stream, 0, SEEK_SET);

		if( fLength > 0 && API_Read_Line(Stream, s) )
		{
			//---------------------------------------------
			// Point...
			if( s.Cmp("EXP") )
			{
				pShapes->Create(SHAPE_TYPE_Point, Parameters("FILENAME")->asString());

				//-----------------------------------------
				// Load Header...

				// Field Count...
				fscanf(Stream, "%d", &nFields);
				API_Read_Line(Stream, s);	// zur naexten Zeile...

				// Fields...
				for(i=0; i<nFields; i++)
				{
					if( API_Read_Line(Stream, s) )
					{
						if( !s.Cmp("[ignore]") || s[0] == '%' )
						{
							pShapes->Get_Table().Add_Field(s, TABLE_FIELDTYPE_String);
						}
						else
						{
							pShapes->Get_Table().Add_Field(s, TABLE_FIELDTYPE_Double);
						}
					}
				}

				//-----------------------------------------
				if( nFields < 2 )
				{
					Message_Dlg(_TL("Invalid File Format."), _TL("Loading GSTAT-File"));
				}
				else
				{
					while( !feof(Stream) && Set_Progress(ftell(Stream), fLength) )
					{
						fscanf(Stream, "%lf%lf", &x, &y); 

						if( !feof(Stream) )
						{
							pShape	= pShapes->Add_Shape();
							pShape->Add_Point(x, y);
							pShape->Get_Record()->Set_Value(0, x);
							pShape->Get_Record()->Set_Value(1, y);

							for(i=2; i<nFields && !feof(Stream); i++)
							{
								if( pShapes->Get_Table().Get_Field_Name(i) == "[ignore]" )
								{
									Stream_Find_NextWhiteChar(Stream);
									pShape->Get_Record()->Set_Value(i, "NA");
								}
								else if( pShapes->Get_Table().Get_Field_Name(i)[0] == '%' )
								{
									Stream_Get_StringInQuota(Stream, s);
									pShape->Get_Record()->Set_Value(i, s);
								}
								else
								{
									fscanf(Stream, "%lf", &Value);
									pShape->Get_Record()->Set_Value(i, Value);
								}
							}

							API_Read_Line(Stream, s);
						}
					}
				}
			}

			//---------------------------------------------
			// Line, Polygon...
			else
			{
				fread(c, 3, sizeof(char), Stream);

				if( !strncmp(c, "ARC", 3) )
				{
					pShapes->Create(SHAPE_TYPE_Line, Parameters("FILENAME")->asString());
					pShapes->Get_Table().Add_Field("VALUE", TABLE_FIELDTYPE_Double);

					//---------------------------------------------
					while( !feof(Stream) && Set_Progress(ftell(Stream), fLength) )
					{
						fscanf(Stream, "%lf", &Value);						// i_ok...
						fscanf(Stream, "%d%d%d%d%d", &i, &i, &i, &i, &i);	// dummy 1..5
						fscanf(Stream, "%d", &nFields);

						if( nFields > 0 )
						{
							pShape	= NULL;

							for(i=0; i<nFields; i++)
							{
								fscanf(Stream, "%lf%lf", &x, &y);

								if( !feof(Stream) )
								{
									if( !pShape )
									{
										pShape	= pShapes->Add_Shape();
										pShape->Get_Record()->Set_Value(0, Value);
									}

									pShape->Add_Point(x, y);
								}
								else
								{
									break;
								}
							}
						}
					}
				}
			}
		}

		fclose(Stream);
	}

	return( pShapes->Get_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
//					Import - Helper						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGStat_Import::Stream_Find_NextWhiteChar(FILE *Stream)
{
	char	c;

	if( Stream )
	{
		do
		{
			c	= fgetc(Stream);
		}
		while( !feof(Stream) && c > 32 );

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CGStat_Import::Stream_Get_StringInQuota(FILE *Stream, CAPI_String &String)
{
	char	c;

	String.Clear();

	if( Stream )
	{
		do
		{
			c	= fgetc(Stream);
		}
		while( !feof(Stream) && c != '\"' );

		while( !feof(Stream) && (c = fgetc(Stream)) != '\"' )
		{
			String.Append(c);
		}

		return( true );
	}

	return( false );
}
