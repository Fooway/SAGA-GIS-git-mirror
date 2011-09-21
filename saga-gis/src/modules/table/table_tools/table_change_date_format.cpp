/**********************************************************
 * Version $Id: table_change_date_format.cpp 911 2011-11-11 11:11:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                      table_tools                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//              table_change_date_format.cpp             //
//                                                       //
//                 Copyright (C) 2011 by                 //
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
//    e-mail:     oconrad@saga-gis.de                    //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Hamburg                  //
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
#include "table_change_date_format.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CTable_Change_Date_Format::CTable_Change_Date_Format(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	// 1. Info...

	Set_Name		(_TL("Change Date Format"));

	Set_Author		(SG_T("O.Conrad (c) 2011"));

	Set_Description	(_TW(
		""
	));


	//-----------------------------------------------------
	// 2. Parameters...

	pNode	= Parameters.Add_Table(
		NULL	, "TABLE"			, _TL("Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD"			, _TL("Date Field"),
		_TL("")
	);

	Parameters.Add_Table(
		NULL	, "OUTPUT"			, _TL("Output"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Choice(
		NULL	, "FMT_IN"			, _TL("Input Format"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|"),
			_TL("dd.mm.yy"),
			_TL("yy.mm.dd"),
			_TL("dd:mm:yy"),
			_TL("yy:mm:dd"),
			_TL("ddmmyyyy, fix size"),
			_TL("yyyymmdd, fix size"),
			_TL("ddmmyy, fix size"),
			_TL("yymmdd, fix size")
		), 0
	);

	Parameters.Add_Choice(
		NULL	, "FMT_OUT"			, _TL("Input Format"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|"),
			_TL("dd.mm.yy"),
			_TL("yy.mm.dd"),
			_TL("dd:mm:yy"),
			_TL("yy:mm:dd"),
			_TL("ddmmyyyy, fix size"),
			_TL("yyyymmdd, fix size"),
			_TL("ddmmyy, fix size"),
			_TL("yymmdd, fix size")
		), 7
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CTable_Change_Date_Format::On_Execute(void)
{
	//-----------------------------------------------------
	int		fmt_In	= Parameters("FMT_IN" )->asInt();
	int		fmt_Out	= Parameters("FMT_OUT")->asInt();

	if( fmt_In == fmt_Out )
	{
		Error_Set(_TL("nothing to do: input format is same as output format"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Table	*pTable	= Parameters("OUTPUT")->asTable();

	if( pTable && pTable != Parameters("TABLE")->asTable() )
	{
		pTable->Assign  (Parameters("TABLE")->asTable());
		pTable->Set_Name(Parameters("TABLE")->asTable()->Get_Name());
	}
	else
	{
		pTable	= Parameters("TABLE")->asTable();
	}

	//-----------------------------------------------------
	int		fDate	= Parameters("FIELD" )->asInt();

	if( pTable->Get_Field_Type(fDate) != SG_DATATYPE_String && fmt_Out < 4 )
	{
		pTable->Set_Field_Type(fDate, SG_DATATYPE_String);
	}

	//-----------------------------------------------------
	SG_Char	sep_In	= fmt_In  == 0 || fmt_In  == 1 ? SG_T('.') : SG_T(':');
	SG_Char	sep_Out	= fmt_Out == 0 || fmt_Out == 1 ? SG_T('.') : SG_T(':');

	//-----------------------------------------------------
	for(int iRecord=0; iRecord<pTable->Get_Count() && Set_Progress(iRecord, pTable->Get_Count()); iRecord++)
	{
		CSG_Table_Record	*pRecord	= pTable->Get_Record(iRecord);

		CSG_String	sDate	= pRecord->asString(fDate);

		int	y, m, d;

		switch( fmt_In )
		{
		case 0:	// dd.mm.yy
		case 2:	// dd:mm:yy
			d	= sDate.BeforeFirst(sep_In).asInt();
			m	= sDate.AfterFirst (sep_In).asInt();
			y	= sDate.AfterLast  (sep_In).asInt();
			break;

		case 1:	// yy.mm.dd
		case 3:	// yy:mm:dd
			y	= sDate.BeforeFirst(sep_In).asInt();
			m	= sDate.AfterFirst (sep_In).asInt();
			d	= sDate.AfterLast  (sep_In).asInt();
			break;

		case 4:	// ddmmyyyy
			d	= sDate.Left (2)   .asInt();
			m	= sDate.Mid  (3, 2).asInt();
			y	= sDate.Right(4)   .asInt();
			break;

		case 5:	// yyyymmdd
			y	= sDate.Left (4)   .asInt();
			m	= sDate.Mid  (5, 2).asInt();
			d	= sDate.Right(2)   .asInt();
			break;

		case 6:	// ddmmyy
			d	= sDate.Left (2)   .asInt();
			m	= sDate.Mid  (3, 2).asInt();
			y	= sDate.Right(2)   .asInt();
			break;

		case 7:	// yymmdd
			y	= sDate.Left (2)   .asInt();
			m	= sDate.Mid  (3, 2).asInt();
			d	= sDate.Right(2)   .asInt();
			break;
		}

		switch( fmt_Out )
		{
		case 0:	sDate.Printf(SG_T("%d.%d.%d")    , d, m, y);	break;	// dd:mm:yy
		case 1:	sDate.Printf(SG_T("%d.%d.%d")    , y, m, d);	break;	// yy:mm:dd
		case 2:	sDate.Printf(SG_T("%d:%d:%d")    , d, m, y);	break;	// dd:mm:yy
		case 3:	sDate.Printf(SG_T("%d:%d:%d")    , y, m, d);	break;	// yy:mm:dd
		case 4:	sDate.Printf(SG_T("%02d%02d%04d"), d, m, y);	break;	// ddmmyyyy
		case 5:	sDate.Printf(SG_T("%04d%02d%02d"), y, m, d);	break;	// yyyymmdd
		case 6:	sDate.Printf(SG_T("%02d%02d%02d"), d, m, y - 100 * (int)(y / 100.0));	break;	// ddmmyy
		case 7:	sDate.Printf(SG_T("%02d%02d%02d"), y - 100 * (int)(y / 100.0), m, d);	break;	// yymmdd
		}

		pRecord->Set_Value(fDate, sDate);
	}

	//-----------------------------------------------------
	if( pTable == Parameters("TABLE")->asTable() )
	{
		DataObject_Update(pTable);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CTable_Change_Time_Format::CTable_Change_Time_Format(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	// 1. Info...

	Set_Name		(_TL("Change Time Format"));

	Set_Author		(SG_T("O.Conrad (c) 2011"));

	Set_Description	(_TW(
		""
	));


	//-----------------------------------------------------
	// 2. Parameters...

	pNode	= Parameters.Add_Table(
		NULL	, "TABLE"			, _TL("Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD"			, _TL("Time Field"),
		_TL("")
	);

	Parameters.Add_Table(
		NULL	, "OUTPUT"			, _TL("Output"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Choice(
		NULL	, "FMT_IN"			, _TL("Input Format"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|"),
			_TL("hh.mm.ss"),
			_TL("hh:mm:ss"),
			_TL("hhmmss, fix size"),
			_TL("hours"),
			_TL("minutes"),
			_TL("seconds")
		), 1
	);

	Parameters.Add_Choice(
		NULL	, "FMT_OUT"			, _TL("Input Format"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|"),
			_TL("hh.mm.ss"),
			_TL("hh:mm:ss"),
			_TL("hhmmss, fix size"),
			_TL("hours"),
			_TL("minutes"),
			_TL("seconds")
		), 5
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CTable_Change_Time_Format::On_Execute(void)
{
	//-----------------------------------------------------
	int		fmt_In	= Parameters("FMT_IN" )->asInt();
	int		fmt_Out	= Parameters("FMT_OUT")->asInt();

	if( fmt_In == fmt_Out )
	{
		Error_Set(_TL("nothing to do: input format is same as output format"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Table	*pTable	= Parameters("OUTPUT")->asTable();

	if( pTable && pTable != Parameters("TABLE")->asTable() )
	{
		pTable->Assign  (Parameters("TABLE")->asTable());
		pTable->Set_Name(Parameters("TABLE")->asTable()->Get_Name());
	}
	else
	{
		pTable	= Parameters("TABLE")->asTable();
	}

	//-----------------------------------------------------
	int		fDate	= Parameters("FIELD" )->asInt();

	if( pTable->Get_Field_Type(fDate) != SG_DATATYPE_String && fmt_Out < 2 )
	{
		pTable->Set_Field_Type(fDate, SG_DATATYPE_String);
	}

	//-----------------------------------------------------
	SG_Char	sep_In	= fmt_In  == 0 ? SG_T('.') : SG_T(':');
	SG_Char	sep_Out	= fmt_Out == 0 ? SG_T('.') : SG_T(':');

	//-----------------------------------------------------
	for(int iRecord=0; iRecord<pTable->Get_Count() && Set_Progress(iRecord, pTable->Get_Count()); iRecord++)
	{
		CSG_Table_Record	*pRecord	= pTable->Get_Record(iRecord);

		CSG_String	sDate	= pRecord->asString(fDate);

		double	s;

		switch( fmt_In )
		{
		case 0:	// hh.mm.ss
		case 1:	// hh:mm:ss
			s	 = sDate.BeforeFirst(sep_In).asInt() * 3600;
			s	+= sDate.AfterFirst (sep_In).asInt() * 60;
			s	+= sDate.AfterLast  (sep_In).asDouble();
			break;

		case 2:	// hhmmss
			s	 = sDate.Left (2)   .asInt() * 3600;
			s	+= sDate.Mid  (3, 2).asInt() * 60;
			s	+= sDate.Right(2)   .asDouble();
			break;

		case 3:	// hours
			s	 = sDate.asDouble() * 3600;
			break;

		case 4:	// minutes
			s	 = sDate.asDouble() * 60;
			break;

		case 5:	// seconds
			s	 = sDate.asDouble();
			break;
		}

		switch( fmt_Out )
		{
		case 0:	case 1: case 2:
			{
				int	h	= (int)(s / 3600);	s	= s - h * 3600;
				int	m	= (int)(s /   60);	s	= s - m *   60;

				switch( fmt_Out )
				{
			//	case 0:	sDate.Printf(SG_T("%02d.%02d.%02.*f"), h, m, SG_Get_Significant_Decimals(s), s);	break;	// hh:mm:ss.s
			//	case 1:	sDate.Printf(SG_T("%02d:%02d:%02.*f"), h, m, SG_Get_Significant_Decimals(s), s);	break;	// hh:mm:ss.s
				case 0:	sDate.Printf(SG_T("%02d.%02d.%02d"  ), h, m, (int)(s + 0.5));	break;	// hh:mm:ss
				case 1:	sDate.Printf(SG_T("%02d:%02d:%02d"  ), h, m, (int)(s + 0.5));	break;	// hh:mm:ss
				case 2:	sDate.Printf(SG_T("%02d%02d%02d"    ), h, m, (int)(s + 0.5));	break;	// hhmmss
				}
			}
			break;

		case 3:	case 4: case 5:
			{
				switch( fmt_Out )
				{
				case 3:	s	= s / 3600;	break;	// hours
				case 4:	s	= s /   60;	break;	// minutes
			//	case 5:	s	= s       ;	break;	// seconds
				}

				sDate.Printf(SG_T("%.*f"), SG_Get_Significant_Decimals(s), s);
			}
			break;
		}

		pRecord->Set_Value(fDate, sDate);
	}

	//-----------------------------------------------------
	if( pTable == Parameters("TABLE")->asTable() )
	{
		DataObject_Update(pTable);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------