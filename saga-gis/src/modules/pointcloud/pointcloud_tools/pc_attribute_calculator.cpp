/**********************************************************
 * Version $Id$
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                   pointcloud_tools                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//              pc_attribute_calculator.cpp              //
//                                                       //
//                 Copyright (C) 2010 by                 //
//                    Volker Wichmann                    //
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
//    e-mail:     wichmann@laserdata                     //
//                                                       //
//    contact:    Volker Wichmann                        //
//                LASERDATA GmbH                         //
//                Management and analysis of             //
//                laserscanning data                     //
//                Innsbruck, Austria                     //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "pc_attribute_calculator.h"


///////////////////////////////////////////////////////////
//														 //
//				Construction/Destruction				 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------

CPC_Attribute_Calculator::CPC_Attribute_Calculator(void)
{

	Set_Name(_TL("Point Cloud Attribute Calculator"));

	Set_Author(_TL("Volker Wichmann (c) 2010, LASERDATA GmbH"));

	CSG_String	s(_TW(
		"The Point Cloud Attribute Calculator calculates a new attribute "
		"based on existing attributes and a mathematical formula. "
		"The attribute fields are referenced by characters 'a' to 'z' "
		"with 'a' referencing the first field, 'b' the second field and so "
		"on. Please note that the first three fields contain the x,y,z coordinates.\n"
		"This (meaningless) example calculates the sine of the x coordinate and "
		"multiplies it with the z coordinate: sin(a) * c\n\n"
		"The following operators are available for the formula definition:\n"
	));

	s	+= CSG_Formula::Get_Help_Operators();

	Set_Description(s);


	//-----------------------------------------------------
	Parameters.Add_PointCloud(
		NULL	, "PC_IN"		,_TL("Point Cloud"),
		_TL("Input"),
		PARAMETER_INPUT
	);

	Parameters.Add_PointCloud(
		NULL	, "PC_OUT"		,_TL("Result"),
		_TL("Output"),
		PARAMETER_OUTPUT
	);

	Parameters.Add_String(
		NULL	, "FORMULA"		, _TL("Formula"),
		_TL(""),
		SG_T("a+b")
	);

	Parameters.Add_String(
		NULL	, "NAME"		, _TL("Output Field Name"),
		_TL(""),
		SG_T("a+b")
	);

	Parameters.Add_Choice(
		NULL	, "TYPE"		, _TL("Field data type"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|%s|"),
			_TL("1 bit"),
			_TL("1 byte unsigned integer"),
			_TL("1 byte signed integer"),
			_TL("2 byte unsigned integer"),
			_TL("2 byte signed integer"),
			_TL("4 byte unsigned integer"),
			_TL("4 byte signed integer"),
			_TL("4 byte floating point"),
			_TL("8 byte floating point")
		), 7
	);

}


//---------------------------------------------------------
CPC_Attribute_Calculator::~CPC_Attribute_Calculator(void)
{}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CPC_Attribute_Calculator::On_Execute(void)
{
	CSG_PointCloud	*pInput, *pResult;
	int				iFields;
	double			dValue;
	double			*pFieldValues;
	const SG_Char	*pFormula;
	CSG_Formula		Formula;
	CSG_String		Msg;
	TSG_Data_Type	Type;


	//---------------------------------------------------------
	pInput		= Parameters("PC_IN")->asPointCloud();
	pResult		= Parameters("PC_OUT")->asPointCloud();

	switch( Parameters("TYPE")->asInt() )
	{
	case 0:				Type	= SG_DATATYPE_Bit;		break;
	case 1:				Type	= SG_DATATYPE_Byte;		break;
	case 2:				Type	= SG_DATATYPE_Char;		break;
	case 3:				Type	= SG_DATATYPE_Word;		break;
	case 4:				Type	= SG_DATATYPE_Short;	break;
	case 5:				Type	= SG_DATATYPE_DWord;	break;
	case 6:				Type	= SG_DATATYPE_Int;		break;
	case 7: default:	Type	= SG_DATATYPE_Float;	break;
	case 8:				Type	= SG_DATATYPE_Double;	break;
	}

	pResult->Create(pInput);
	pResult->Set_Name(CSG_String::Format(SG_T("%s_%s"), pInput->Get_Name(), Parameters("NAME")->asString()));

	pResult->Add_Field(Parameters("NAME")->asString(), Type);

	pFormula	= Parameters("FORMULA")->asString();
	iFields		= pInput->Get_Field_Count();

	Formula.Set_Formula(pFormula);

	if( Formula.Get_Error(Msg) )
	{
		SG_UI_Msg_Add_Error(Msg);

		return( false );
	}

	
	//---------------------------------------------------------
	pFieldValues	= new double[iFields];

	for( int i=0; i<pInput->Get_Record_Count(); i++ )
	{
		pResult->Add_Point(pInput->Get_X(i), pInput->Get_Y(i), pInput->Get_Z(i));
		
		for( int j=0; j<iFields; j++ )
		{
			pFieldValues[j] = pInput->Get_Value(i, j);

			if( j > 2 )
				pResult->Set_Value(j, pInput->Get_Value(i, j));
		}

		dValue = Formula.Get_Value(pFieldValues, iFields);

		pResult->Set_Value(iFields, dValue);

	}

	delete[] pFieldValues;

	return (true);

}


//---------------------------------------------------------
int CPC_Attribute_Calculator::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{

	if( !SG_STR_CMP(pParameter->Get_Identifier(), SG_T("FORMULA")) )
		pParameters->Get_Parameter(SG_T("NAME"))->Set_Value(pParameter->asString());

    return (true);

}