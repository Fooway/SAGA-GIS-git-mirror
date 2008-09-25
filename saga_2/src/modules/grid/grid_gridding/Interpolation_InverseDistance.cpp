
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                     Grid_Gridding                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//           Interpolation_InverseDistance.cpp           //
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
#include "Interpolation_InverseDistance.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CInterpolation_InverseDistance::CInterpolation_InverseDistance(void)
{
	Set_Name		(_TL("Inverse Distance"));

	Set_Author		(SG_T("(c) 2003 by O.Conrad"));

	Set_Description	(_TW(
		"Inverse distance to a power method for grid interpolation from irregular distributed points."
	));

	Parameters.Add_Value(
		NULL	, "POWER"		, _TL("Inverse Distance: Power"),
		_TL(""),
		PARAMETER_TYPE_Double	, 1.0
	);

	Parameters.Add_Value(
		NULL	, "RADIUS"		, _TL("Search Radius"),
		_TL(""),
		PARAMETER_TYPE_Double	, 100.0
	);

	Parameters.Add_Value(
		NULL	, "NPOINTS"		, _TL("Maximum Points"),
		_TL(""),
		PARAMETER_TYPE_Int		, 10.0
	);

	Parameters.Add_Choice(
		NULL	, "MODE"		, _TL("Search Mode"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("all directions"),
			_TL("quadrants")
		)
	);
}

//---------------------------------------------------------
CInterpolation_InverseDistance::~CInterpolation_InverseDistance(void)
{}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CInterpolation_InverseDistance::On_Initialize(void)
{
	m_Power			= Parameters("POWER")	->asDouble();
	m_Radius		= Parameters("RADIUS")	->asDouble();
	m_nPoints_Max	= Parameters("NPOINTS")	->asInt();
	m_Mode			= Parameters("MODE")	->asInt();

	return( Set_Search_Engine() );
}

//---------------------------------------------------------
bool CInterpolation_InverseDistance::_Get_Value(const TSG_Point &p, double &z, double &ds)
{
	for(int i=0; i<m_Search.Get_Selected_Count(); i++)
	{
		CSG_Shape	*pPoint	= m_Search.Get_Selected_Point(i);

		if( pPoint )
		{
			double	d	= SG_Get_Distance(p, pPoint->Get_Point(0));

			if( d <= 0.0 )
			{
				z	= pPoint->Get_Record()->asDouble(m_zField);
				ds	= 1.0;

				return( false );
			}

			d	= pow(d, -m_Power);

			z	+= d * pPoint->Get_Record()->asDouble(m_zField);
			ds	+= d;
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CInterpolation_InverseDistance::Get_Value(double x, double y, double &z)
{
	double		ds;
	TSG_Point	p;

	p.x	= x;
	p.y	= y;
	z	= 0.0;
	ds	= 0.0;

	switch( m_Mode )
	{
	case 0: default:
		if( m_Search.Select_Radius(x, y, m_Radius, false, m_nPoints_Max) > 0 )
		{
			if( !_Get_Value(p, z, ds) )
			{
				return( true );
			}
		}
		break;

	case 1:
		if( m_Search.Select_Quadrants(x, y, m_Radius, m_nPoints_Max) > 0 )
		{
			if( !_Get_Value(p, z, ds) )
			{
				return( true );
			}
		}
		break;
	}

	if( ds > 0.0 )
	{
		z	/= ds;

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
