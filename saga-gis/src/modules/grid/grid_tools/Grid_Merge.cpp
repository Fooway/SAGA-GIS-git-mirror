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
//                       Tutorial                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    Grid_Merge.cpp                     //
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
#include "Grid_Merge.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGrid_Merge::CGrid_Merge(void)
{
	//-----------------------------------------------------
	Set_Name		(_TL("Mosaicking"));

	Set_Author		("O.Conrad (c) 2003-12");

	Set_Description	(_TW(
		"Merges multiple grids into one single grid."
	));

	//-----------------------------------------------------
	Parameters.Add_Grid_List(
		NULL	, "GRIDS"		, _TL("Input Grids"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_String(
		NULL	, "NAME"		, _TL("Name"),
		_TL(""),
		_TL("Mosaic")
	);

	Parameters.Add_Choice(
		NULL	, "TYPE"		, _TL("Data Storage Type"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s|%s|%s|%s|%s|%s|",
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

	Parameters.Add_Choice(
		NULL	, "RESAMPLING"		, _TL("Resampling"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s|",
			_TL("Nearest Neighbour"),
			_TL("Bilinear Interpolation"),
			_TL("Bicubic Spline Interpolation"),
			_TL("B-Spline Interpolation")
		), 3
	);

	Parameters.Add_Choice(
		NULL	, "OVERLAP"		, _TL("Overlapping Areas"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s|%s|%s|%s|",
			_TL("first"),
			_TL("last"),
			_TL("minimum"),
			_TL("maximum"),
			_TL("mean"),
			_TL("blend boundary"),
			_TL("feathering")
		), 1
	);

	Parameters.Add_Value(
		NULL	, "BLEND_DIST"	, _TL("Blending Distance"),
		_TL("blending distance given in map units"),
		PARAMETER_TYPE_Double, 10.0, 0.0, true
	);

	Parameters.Add_Choice(
		NULL	, "MATCH"		, _TL("Match"),
		_TL(""),
		CSG_String::Format("%s|%s|",
			_TL("none"),
			_TL("regression"),
			_TL("histogram match")
		), 0
	);

	//-----------------------------------------------------
	m_Grid_Target.Create(&Parameters, true, NULL, "TARGET_");
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGrid_Merge::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "GRIDS") )
	{
		Set_Target(pParameters, pParameter->asGridList());
	}

	return( m_Grid_Target.On_Parameter_Changed(pParameters, pParameter) ? 1 : 0 );
}

//---------------------------------------------------------
int CGrid_Merge::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	!SG_STR_CMP(pParameter->Get_Identifier(), "OVERLAP") )
	{
		pParameters->Get_Parameter("BLEND_DIST")->Set_Enabled(pParameter->asInt() == 5 || pParameter->asInt() == 6);
	}

	return( m_Grid_Target.On_Parameters_Enable(pParameters, pParameter) ? 1 : 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Merge::On_Execute(void)
{
	//-----------------------------------------------------
	if( !Initialize() )
	{
		return( false );
	}

	//-----------------------------------------------------
	for(int i=0; i<m_pGrids->Get_Count(); i++)
	{
		CSG_Grid	*pGrid	= m_pGrids->asGrid(i);

		Set_Weight(pGrid);

		Get_Match(i > 0 ? pGrid : NULL);

		int	ax	= (int)((pGrid->Get_XMin() - m_pMosaic->Get_XMin()) / m_pMosaic->Get_Cellsize());
		int	ay	= (int)((pGrid->Get_YMin() - m_pMosaic->Get_YMin()) / m_pMosaic->Get_Cellsize());

		//-------------------------------------------------
		if(	is_Aligned(pGrid) )
		{
			Process_Set_Text(CSG_String::Format("[%d/%d] %s: %s", i + 1, m_pGrids->Get_Count(), _TL("copying"), pGrid->Get_Name()));

			int	nx	= pGrid->Get_NX(); if( nx > m_pMosaic->Get_NX() - ax )	nx	= m_pMosaic->Get_NX() - ax;
			int	ny	= pGrid->Get_NY(); if( ny > m_pMosaic->Get_NY() - ay )	ny	= m_pMosaic->Get_NY() - ay;

			for(int y=0; y<ny && Set_Progress(y, ny); y++)
			{
				if( ay + y >= 0 )
				{
					#pragma omp parallel for
					for(int x=0; x<nx; x++)
					{
						if( ax + x >= 0 && !pGrid->is_NoData(x, y) )
						{
							Set_Value(ax + x, ay + y, pGrid->asDouble(x, y), Get_Weight(x, y));
						}
					}
				}
			}
		}

		//-------------------------------------------------
		else
		{
			Process_Set_Text(CSG_String::Format("[%d/%d] %s: %s", i + 1, m_pGrids->Get_Count(), _TL("resampling"), pGrid->Get_Name()));

			if( ax < 0 )	ax	= 0;
			if( ay < 0 )	ay	= 0;

			int	nx	= 1 + m_pMosaic->Get_System().Get_xWorld_to_Grid(pGrid->Get_XMax()); if( nx > m_pMosaic->Get_NX() )	nx	= m_pMosaic->Get_NX();
			int	ny	= 1 + m_pMosaic->Get_System().Get_yWorld_to_Grid(pGrid->Get_YMax()); if( ny > m_pMosaic->Get_NY() )	ny	= m_pMosaic->Get_NY();

			for(int y=ay; y<ny && Set_Progress(y-ay, ny-ay); y++)
			{
				double	py	= m_pMosaic->Get_YMin() + y * m_pMosaic->Get_Cellsize();

				#pragma omp parallel for
				for(int x=ax; x<nx; x++)
				{
					double	px	= m_pMosaic->Get_XMin() + x * m_pMosaic->Get_Cellsize();

					Set_Value(x, y, pGrid, px, py);
				}
			}
		}
	}

	//-----------------------------------------------------
	if( m_Overlap == 4 )	// mean
	{
		for(int y=0; y<m_pMosaic->Get_NY() && Set_Progress(y, m_pMosaic->Get_NY()); y++)
		{
			#pragma omp parallel for
			for(int x=0; x<m_pMosaic->Get_NX(); x++)
			{
				double	w	= m_Weights.asDouble(x, y);

				if( w > 0.0 )
				{
					m_pMosaic->Mul_Value(x, y, 1.0 / w);
				}
			}
		}
	}

	//-----------------------------------------------------
	m_Weight .Destroy();
	m_Weights.Destroy();

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Merge::Initialize(void)
{
	//-----------------------------------------------------
	m_pMosaic	= NULL;
	m_Overlap	= Parameters("OVERLAP"   )->asInt();
	m_pGrids	= Parameters("GRIDS"     )->asGridList();
	m_dBlend	= Parameters("BLEND_DIST")->asDouble();

	if( m_pGrids->Get_Count() < 2 )
	{
		Error_Set(_TL("nothing to do, there are less than two grids in input list."));

		return( false );
	}

	//-----------------------------------------------------
	switch( Parameters("RESAMPLING")->asInt() )
	{
	default:	m_Resampling	= GRID_RESAMPLING_NearestNeighbour;	break;
	case  1:	m_Resampling	= GRID_RESAMPLING_Bilinear;			break;
	case  2:	m_Resampling	= GRID_RESAMPLING_BicubicSpline;	break;
	case  3:	m_Resampling	= GRID_RESAMPLING_BSpline;			break;
	}

	//-----------------------------------------------------
	TSG_Data_Type	Type;

	switch( Parameters("TYPE")->asInt() )
	{
	default:	Type	= SG_DATATYPE_Float;	break;
	case 0:		Type	= SG_DATATYPE_Bit;		break;
	case 1:		Type	= SG_DATATYPE_Byte;		break;
	case 2:		Type	= SG_DATATYPE_Char;		break;
	case 3:		Type	= SG_DATATYPE_Word;		break;
	case 4:		Type	= SG_DATATYPE_Short;	break;
	case 5:		Type	= SG_DATATYPE_DWord;	break;
	case 6:		Type	= SG_DATATYPE_Int;		break;
	case 7: 	Type	= SG_DATATYPE_Float;	break;
	case 8:		Type	= SG_DATATYPE_Double;	break;
	}

	if( (m_pMosaic = m_Grid_Target.Get_Grid(Type)) == NULL )
	{
		return( false );
	}

	if( m_pMosaic->Get_Type() != Type && !m_pMosaic->Create(m_pMosaic->Get_System(), Type) )
	{
		return( false );
	}

	//-----------------------------------------------------
	m_pMosaic->Set_Name(Parameters("NAME")->asString());

	m_pMosaic->Assign_NoData();

	//-----------------------------------------------------
	switch( m_Overlap )
	{
	case 4:	// mean
		if( !m_Weights.Create(m_pMosaic->Get_System(), m_pGrids->Get_Count() < 256 ? SG_DATATYPE_Byte : SG_DATATYPE_Word) )
		{
			Error_Set(_TL("could not create weights grid"));

			return( false );
		}

		break;

	case 6:	// feathering
		if( !m_Weights.Create(m_pMosaic->Get_System(), SG_DATATYPE_Word) )
		{
			Error_Set(_TL("could not create weights grid"));

			return( false );
		}

		m_Weights.Set_Scaling(m_pMosaic->Get_Cellsize());

		break;
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Merge::Set_Target(CSG_Parameters *pParameters, CSG_Parameter_Grid_List *pGrids)
{
	if( pParameters && pGrids && pGrids->Get_Count() > 0 )
	{
		double		d	= pGrids->asGrid(0)->Get_Cellsize();
		CSG_Rect	r	= pGrids->asGrid(0)->Get_Extent();

		for(int i=1; i<pGrids->Get_Count(); i++)
		{
			if( d > pGrids->asGrid(i)->Get_Cellsize() )
			{
				d	= pGrids->asGrid(i)->Get_Cellsize();
			}

			r.Union(pGrids->asGrid(i)->Get_Extent());
		}

		int	nx	= 1 + (int)(r.Get_XRange() / d);
		int	ny	= 1 + (int)(r.Get_YRange() / d);

		m_Grid_Target.Set_User_Defined(pParameters, r.Get_XMin(), r.Get_YMin(), d, nx, ny);
	}

	return( false );
}

//---------------------------------------------------------
bool CGrid_Merge::is_Aligned(CSG_Grid *pGrid)
{
	return(	pGrid->Get_Cellsize() == m_pMosaic->Get_Cellsize()
		&&	fabs(fmod(pGrid->Get_XMin() - m_pMosaic->Get_XMin(), m_pMosaic->Get_Cellsize())) <= 0.001 * m_pMosaic->Get_Cellsize()
		&&	fabs(fmod(pGrid->Get_YMin() - m_pMosaic->Get_YMin(), m_pMosaic->Get_Cellsize())) <= 0.001 * m_pMosaic->Get_Cellsize()
	);
}

//---------------------------------------------------------
inline void CGrid_Merge::Set_Value(int x, int y, double Value, double Weight)
{
	if( m_Match.Get_N() == 2 )
	{
		Value	= m_Match[0] + m_Match[1] * Value;
	}

	switch( m_Overlap )
	{
	case 0:	// first
		if( m_pMosaic->is_NoData(x, y) )
		{
			m_pMosaic->Set_Value(x, y, Value);
		}
		break;

	case 1:	// last
		{
			m_pMosaic->Set_Value(x, y, Value);
		}
		break;

	case 2:	// minimum
		if( m_pMosaic->is_NoData(x, y) || m_pMosaic->asDouble(x, y) > Value )
		{
			m_pMosaic->Set_Value(x, y, Value);
		}
		break;

	case 3:	// maximum
		if( m_pMosaic->is_NoData(x, y) || m_pMosaic->asDouble(x, y) < Value )
		{
			m_pMosaic->Set_Value(x, y, Value);
		}
		break;

	case 4:	// mean
		if( m_pMosaic->is_NoData(x, y) )
		{
			m_pMosaic->Set_Value(x, y, Value);
			m_Weights .Set_Value(x, y, 1);
		}
		else
		{
			m_pMosaic->Add_Value(x, y, Value);
			m_Weights .Add_Value(x, y, 1);
		}
		break;

	case 5:	// blend
		if( m_pMosaic->is_NoData(x, y) )
		{
			m_pMosaic->Set_Value(x, y, Value);
		}
		else
		{
			m_pMosaic->Set_Value(x, y, (1.0 - Weight) * m_pMosaic->asDouble(x, y) + Weight * Value);
		}
		break;

	case 6:	// feathering
		if( m_pMosaic->is_NoData(x, y) )
		{
			m_pMosaic->Set_Value(x, y, Value);
			m_Weights .Set_Value(x, y, Weight);
		}
		else
		{
			double	d	= (Weight - m_Weights.asDouble(x, y)) / m_dBlend;

			if( d >= 1.0 )
			{
				m_pMosaic->Set_Value(x, y, Value);
				m_Weights .Set_Value(x, y, Weight);
			}
			else if( d > -1.0 )
			{
				d	= 0.5 * (1.0 + d);

				m_pMosaic->Set_Value(x, y, (1.0 - d) * m_pMosaic->asDouble(x, y) + d * Value);

				if( d > 0.5 )
				{
					m_Weights .Set_Value(x, y, Weight);
				}
			}
		}
		break;
	}
}

//---------------------------------------------------------
inline void CGrid_Merge::Set_Value(int x, int y, CSG_Grid *pGrid, double px, double py)
{
	double	z;

	if( pGrid->Get_Value(px, py, z, m_Resampling) )
	{
		if( m_Weight.is_Valid() )
		{
			double	w;

			if( m_Weight.Get_Value(px, py, w, GRID_RESAMPLING_BSpline, true) )
			{
				Set_Value(x, y, z, w);
			}
		}
		else
		{
			Set_Value(x, y, z, 1.0);
		}
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline double CGrid_Merge::Get_Weight(int x, int y)
{
	return( m_Weight.is_Valid() ? m_Weight.asDouble(x, y) : 1.0 );
}

//---------------------------------------------------------
bool CGrid_Merge::Set_Weight(CSG_Grid *pGrid)
{
	int	dBlend;

	switch( m_Overlap )
	{
	default:	// only feathering/blending
		return( true );

	case 5:		// blending
		dBlend	= 1 + (int)(m_dBlend / pGrid->Get_Cellsize());
		break;

	case 6:		// feathering
		dBlend	= 0;
		break;
	}

	//-----------------------------------------------------
	if( !m_Weight.Get_System().is_Equal(pGrid->Get_System()) )
	{
		if( !m_Weight.Create(pGrid->Get_System(), dBlend > 0 && dBlend < 255 ? SG_DATATYPE_Byte : SG_DATATYPE_Word) )
		{
			Error_Set(_TL("could not create distance grid"));

			return( false );
		}
	}

	//-----------------------------------------------------
	int		x, y, d;

	for(y=0; y<pGrid->Get_NY() && Process_Get_Okay(); y++)
	{
		for(x=0, d=1; x<pGrid->Get_NX(); x++)
		{
			if( pGrid->is_NoData(x, y) )
				m_Weight.Set_Value(x, y, d = 0);
			else //if( m_Weight.asInt(x, y) > d )
				m_Weight.Set_Value(x, y, d);

			if( dBlend <= 0 || d < dBlend )	d++;
		}

		for(x=pGrid->Get_NX()-1, d=1; x>=0; x--)
		{
			if( pGrid->is_NoData(x, y) )
				m_Weight.Set_Value(x, y, d = 0);
			else if( m_Weight.asInt(x, y) > d )
				m_Weight.Set_Value(x, y, d);
			else
				d	= m_Weight.asInt(x, y);

			if( dBlend <= 0 || d < dBlend )	d++;
		}
	}

	for(x=0; x<pGrid->Get_NX() && Process_Get_Okay(); x++)
	{
		for(y=0, d=1; y<pGrid->Get_NY(); y++)
		{
			if( pGrid->is_NoData(x, y) )
				m_Weight.Set_Value(x, y, d = 0);
			else if( m_Weight.asInt(x, y) > d )
				m_Weight.Set_Value(x, y, d);
			else
				d	= m_Weight.asInt(x, y);

			if( dBlend <= 0 || d < dBlend )	d++;
		}

		for(y=pGrid->Get_NY()-1, d=1; y>=0; y--)
		{
			if( pGrid->is_NoData(x, y) )
				m_Weight.Set_Value(x, y, d = 0);
			else if( m_Weight.asInt(x, y) > d )
				m_Weight.Set_Value(x, y, d);
			else
				d	= m_Weight.asInt(x, y);

			if( dBlend <= 0 || d < dBlend )	d++;
		}
	}

	//-----------------------------------------------------
	switch( m_Overlap )
	{
	case 5:	// blending
		m_Weight.Set_Scaling(1.0 / dBlend);	// normalize (0 <= z <= 1)
		break;

	case 6:	// feathering
		m_Weight.Set_Scaling(m_Weight.Get_Cellsize());
		break;
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CGrid_Merge::Get_Match(CSG_Grid *pGrid)
{
	if( pGrid && Parameters("MATCH")->asInt() )
	{
		Process_Set_Text(CSG_String::Format("%s: %s", _TL("matching histogram"), pGrid->Get_Name()));

		int	ax	= (int)((pGrid->Get_XMin() - m_pMosaic->Get_XMin()) / m_pMosaic->Get_Cellsize());	if( ax < 0 )	ax	= 0;
		int	ay	= (int)((pGrid->Get_YMin() - m_pMosaic->Get_YMin()) / m_pMosaic->Get_Cellsize());	if( ay < 0 )	ay	= 0;

		int	nx	= 1 + m_pMosaic->Get_System().Get_xWorld_to_Grid(pGrid->Get_XMax()); if( nx > m_pMosaic->Get_NX() )	nx	= m_pMosaic->Get_NX();
		int	ny	= 1 + m_pMosaic->Get_System().Get_yWorld_to_Grid(pGrid->Get_YMax()); if( ny > m_pMosaic->Get_NY() )	ny	= m_pMosaic->Get_NY();

		CSG_Vector	Z[2];

		for(int y=ay; y<ny && Set_Progress(y-ay, ny-ay); y++)
		{
			double	py	= m_pMosaic->Get_YMin() + y * m_pMosaic->Get_Cellsize();

			for(int x=ax; x<nx; x++)
			{
				if( !m_pMosaic->is_NoData(x, y) )
				{
					double	z, px	= m_pMosaic->Get_XMin() + x * m_pMosaic->Get_Cellsize();

					if( pGrid->Get_Value(px, py, z, GRID_RESAMPLING_NearestNeighbour) )
					{
						Z[0].Add_Row(z);
						Z[1].Add_Row(m_pMosaic->asDouble(x, y));
					}
				}
			}
		}

		CSG_Regression	r;

		if( r.Calculate((int)Z[0].Get_Size(), Z[0].Get_Data(), Z[1].Get_Data()) )
		{
			m_Match.Create(2);

			m_Match[0]	= r.Get_Constant();
			m_Match[1]	= r.Get_Coefficient();

			Message_Add("histogram stretch:\n", false);
			Message_Add(r.asString(), false);

			return;
		}
	}

	m_Match.Destroy();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
