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
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
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
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Inverse Distance Weighted"));

	Set_Author		(SG_T("O.Conrad (c) 2003"));

	Set_Description	(_TW(
		"Inverse distance grid interpolation from irregular distributed points."
	));

	//-----------------------------------------------------
	CSG_Parameter	*pSearch	= Parameters.Add_Node(
		NULL	, "NODE_SEARCH"			, _TL("Search Options"),
		_TL("")
	);

	pNode	= Parameters.Add_Choice(
		pSearch	, "SEARCH_RANGE"		, _TL("Search Range"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("local"),
			_TL("global")
		)
	);

	Parameters.Add_Value(
		pNode	, "SEARCH_RADIUS"		, _TL("Maximum Search Distance"),
		_TL("local maximum search distance given in map units"),
		PARAMETER_TYPE_Double	, 1000.0, 0, true
	);

	pNode	= Parameters.Add_Choice(
		pSearch	, "SEARCH_POINTS_ALL"	, _TL("Number of Points"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("maximum number of nearest points"),
			_TL("all points within search distance")
		)
	);

	Parameters.Add_Value(
		pNode	, "SEARCH_POINTS_MAX"	, _TL("Maximum Number of Points"),
		_TL("maximum number of nearest points"),
		PARAMETER_TYPE_Int, 20, 1, true
	);

	Parameters.Add_Choice(
		pNode	, "SEARCH_DIRECTION"	, _TL("Search Direction"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("all directions"),
			_TL("quadrants")
		)
	);

	//-----------------------------------------------------
	pNode	= Parameters.Add_Choice(
		NULL	, "WEIGHTING"	, _TL("Distance Weighting"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|"),
			_TL("inverse distance to a power"),
			_TL("linearly decreasing within search radius"),
			_TL("exponential weighting scheme"),
			_TL("gaussian weighting scheme")
		), 0
	);

	Parameters.Add_Value(
		pNode	, "WEIGHT_POWER"		, _TL("Power"),
		_TL(""),
		PARAMETER_TYPE_Double	, 2.0
	);

	Parameters.Add_Value(
		pNode	, "WEIGHT_BANDWIDTH"	, _TL("Bandwidth"),
		_TL(""),
		PARAMETER_TYPE_Double	, 1.0, 0.0, true
	);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CInterpolation_InverseDistance::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "SHAPES") && pParameter->asShapes() )
	{
		pParameters->Get_Parameter("SEARCH_RADIUS")->Set_Value(SG_Get_Rounded_To_SignificantFigures(
			5 * sqrt(pParameter->asShapes()->Get_Extent().Get_Area() / pParameter->asShapes()->Get_Count()), 1
		));
	}

	return( CInterpolation::On_Parameter_Changed(pParameters, pParameter) );
}

//---------------------------------------------------------
int CInterpolation_InverseDistance::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	!SG_STR_CMP(pParameter->Get_Identifier(), SG_T("SEARCH_RANGE")) )
	{
		pParameters->Get_Parameter("SEARCH_RADIUS"    )->Set_Enabled(pParameter->asInt() == 0);	// local
	}

	if(	!SG_STR_CMP(pParameter->Get_Identifier(), SG_T("SEARCH_POINTS_ALL")) )
	{
		pParameters->Get_Parameter("SEARCH_POINTS_MAX")->Set_Enabled(pParameter->asInt() == 0);	// maximum number of points
		pParameters->Get_Parameter("SEARCH_DIRECTION" )->Set_Enabled(pParameter->asInt() == 0);	// maximum number of points per quadrant
	}

	if(	!SG_STR_CMP(pParameter->Get_Identifier(), SG_T("WEIGHTING")) )
	{
		pParameters->Get_Parameter("WEIGHT_POWER"     )->Set_Enabled(pParameter->asInt() == 0);	// idw to a power
		pParameters->Get_Parameter("WEIGHT_BANDWIDTH" )->Set_Enabled(pParameter->asInt() >= 2);	// exponential or gaussian
	}

	return( CInterpolation::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CInterpolation_InverseDistance::On_Initialize(void)
{
	m_Weighting		= Parameters("WEIGHTING"        )->asInt();
	m_Power			= Parameters("WEIGHT_POWER"     )->asDouble();
	m_Bandwidth		= Parameters("WEIGHT_BANDWIDTH" )->asDouble();

	m_nPoints_Max	= Parameters("SEARCH_POINTS_ALL")->asInt() == 0 ? Parameters("SEARCH_POINTS_MAX")->asInt   () : 0;
	m_Radius		= Parameters("SEARCH_RANGE"     )->asInt() == 0 ? Parameters("SEARCH_RADIUS"    )->asDouble() : 0.0;
	m_iQuadrant		= Parameters("SEARCH_DIRECTION" )->asInt() == 0 ? -1 : 4;

	return( (m_nPoints_Max <= 0 && m_Radius <= 0.0) || Set_Search_Engine() );
}

//---------------------------------------------------------
inline double CInterpolation_InverseDistance::Get_Weight(double Distance)
{
	switch( m_Weighting )
	{
	default:	return( Distance > 0.0 ? pow(Distance, -m_Power) : -1.0 );
	case 1:		return( Distance < m_Radius ? (1.0 - Distance / m_Radius) : 0.0 );
	case 2:		return( exp(-Distance / m_Bandwidth) );
	case 3:		return( exp(-0.5 * SG_Get_Square(Distance / m_Bandwidth)) );
	}
}

//---------------------------------------------------------
int CInterpolation_InverseDistance::Get_Count(double x, double y)
{
	if( m_nPoints_Max > 0 || m_Radius > 0.0 )			// using search engine
	{
		return( m_Search.Select_Nearest_Points(x, y, m_nPoints_Max, m_Radius, m_iQuadrant) );
	}

	return( m_pShapes->Get_Count() );					// without search engine
}

//---------------------------------------------------------
inline bool CInterpolation_InverseDistance::Get_Point(int iPoint, double x, double y, double &w, double &z)
{
	TSG_Point	p;

	if( m_nPoints_Max > 0 || m_Radius > 0.0 )	// using search engine
	{
		if( !m_Search.Get_Selected_Point(iPoint, p.x, p.y, z) )
		{
			return( false );
		}
	}
	else										// without search engine
	{
		CSG_Shape	*pPoint	= m_pShapes->Get_Shape(iPoint);

		if( !pPoint || pPoint->is_NoData(m_zField) )
		{
			return( false );
		}

		p	= pPoint->Get_Point(0);
		z	= pPoint->asDouble(m_zField);
	}

	w	= Get_Weight(SG_Get_Distance(x, y, p.x, p.y));

	return( true );
}

//---------------------------------------------------------
bool CInterpolation_InverseDistance::Get_Value(double x, double y, double &z)
{
	int		nPoints	= Get_Count(x, y);

	if( nPoints <= 0 )
	{
		return( false );
	}

	//-----------------------------------------------------
	CSG_Simple_Statistics	s;

	for(int iPoint=0; iPoint<nPoints; iPoint++)
	{
		double	w;

		if( Get_Point(iPoint, x, y, w, z) )
		{
			if( w < 0.0 )
			{
				return( true );
			}

			s.Add_Value(z, w);
		}
	}

	z	= s.Get_Mean();

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
