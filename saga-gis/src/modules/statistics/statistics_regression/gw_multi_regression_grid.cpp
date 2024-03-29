/**********************************************************
 * Version $Id: gw_multi_regression_grid.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                 statistics_regression                 //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//             gw_multi_regression_grid.cpp              //
//                                                       //
//                 Copyright (C) 2010 by                 //
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
#include "gw_multi_regression_grid.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGW_Multi_Regression_Grid::CGW_Multi_Regression_Grid(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("GWR for Multiple Predictor Grids"));

	Set_Author		("O.Conrad (c) 2010");

	Set_Description	(_TW(
		"References:\n"
	) + GWR_References);

	//-----------------------------------------------------
	pNode = Parameters.Add_Shapes(
		NULL	, "POINTS"		, _TL("Points"),
		_TL(""),
		PARAMETER_INPUT, SHAPE_TYPE_Point
	);

	Parameters.Add_Table_Field(
		pNode	, "DEPENDENT"	, _TL("Dependent Variable"),
		_TL("")
	);

	Parameters.Add_Shapes(
		NULL	, "RESIDUALS"	, _TL("Residuals"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL, SHAPE_TYPE_Point
	);

	//-----------------------------------------------------
	Parameters.Add_Grid_List(
		NULL	, "PREDICTORS"	, _TL("Predictors"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid(
		NULL	, "REGRESSION"	, _TL("Regression"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_Grid(
		NULL	, "QUALITY"		, _TL("Coefficient of Determination"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_Grid_List(
		NULL	, "MODEL"		, _TL("Regression Parameters"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL, false
	);

	Parameters.Add_Value(
		NULL	, "MODEL_OUT"	, _TL("Output of Regression Parameters"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	//-----------------------------------------------------
	pNode	= Parameters.Add_Choice(
		NULL	, "RESOLUTION"	, _TL("Model Resolution"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("same as predictors"),
			_TL("user defined")
		), 1
	);

	Parameters.Add_Value(
		NULL	, "RESOLUTION_VAL"	, _TL("Resolution"),
		_TL("map units"),
		PARAMETER_TYPE_Double	, 1.0, 0.0, true
	);

	//-----------------------------------------------------
	m_Weighting.Set_Weighting(SG_DISTWGHT_GAUSS);
	m_Weighting.Create_Parameters(&Parameters, false);

	//-----------------------------------------------------
	m_Search.Create(&Parameters, Parameters.Add_Node(NULL, "NODE_SEARCH", _TL("Search Options"), _TL("")), 16);

	Parameters("SEARCH_RANGE"     )->Set_Value(1);
	Parameters("SEARCH_POINTS_ALL")->Set_Value(1);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGW_Multi_Regression_Grid::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "POINTS") && pParameter->asShapes() )
	{
		m_Search.On_Parameter_Changed(pParameters, pParameter);

		pParameters->Set_Parameter("RESOLUTION_VAL", GWR_Fit_To_Density(pParameter->asShapes(), 4.0, 1));
		pParameters->Set_Parameter("DW_BANDWIDTH"  , GWR_Fit_To_Density(pParameter->asShapes(), 4.0, 1));
	}

	return( 1 );
}

//---------------------------------------------------------
int CGW_Multi_Regression_Grid::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	!SG_STR_CMP(pParameter->Get_Identifier(), SG_T("RESOLUTION")) )
	{
		pParameters->Get_Parameter("RESOLUTION_VAL")->Set_Enabled(pParameter->asInt() == 1);
	}

	m_Search.On_Parameters_Enable(pParameters, pParameter);

	m_Weighting.Enable_Parameters(pParameters);

	return( 1 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::On_Execute(void)
{
	int		i;

	//-----------------------------------------------------
	CSG_Parameter_Grid_List	*pPredictors	= Parameters("PREDICTORS")->asGridList();

	if( !Initialize(Parameters("POINTS")->asShapes(), Parameters("DEPENDENT")->asInt(), pPredictors) )
	{
		Finalize();

		return( false );
	}

	//-----------------------------------------------------
	CSG_Grid	Quality;

	m_dimModel	= *Get_System();

	if( Parameters("RESOLUTION")->asInt() == 1 && Parameters("RESOLUTION_VAL")->asDouble() > Get_Cellsize() )
	{
		CSG_Rect	r(Get_System()->Get_Extent()); r.Inflate(0.5 * Parameters("RESOLUTION_VAL")->asDouble(), false);

		m_dimModel.Assign(Parameters("RESOLUTION_VAL")->asDouble(), r);

		Quality.Create(m_dimModel);
		m_pQuality	= &Quality;
	}
	else
	{
		m_pQuality	= Parameters("QUALITY")->asGrid();
	}

	//-----------------------------------------------------
	Process_Set_Text(_TL("upsetting model domain"));

	m_pPredictors	= (CSG_Grid **)SG_Calloc(m_nPredictors    , sizeof(CSG_Grid *));
	m_pModel		= (CSG_Grid **)SG_Calloc(m_nPredictors + 1, sizeof(CSG_Grid *));

	for(i=0; i<m_nPredictors; i++)
	{
		if( m_dimModel.Get_Cellsize() > Get_Cellsize() )	// scaling
		{
			m_pPredictors[i]	= SG_Create_Grid(m_dimModel);
			m_pPredictors[i]	->Assign(pPredictors->asGrid(i), GRID_RESAMPLING_NearestNeighbour);	// GRID_RESAMPLING_Mean_Cells
		}
		else
		{
			m_pPredictors[i]	= pPredictors->asGrid(i);
		}

		m_pModel     [i]	= SG_Create_Grid(m_dimModel);
		m_pModel     [i]	->Set_Name(CSG_String::Format(SG_T("%s [%s]"), pPredictors->asGrid(i)->Get_Name(), _TL("Factor")));
	}

	m_pModel[m_nPredictors]	= SG_Create_Grid(m_dimModel);
	m_pModel[m_nPredictors]	->Set_Name(_TL("Intercept"));

	//-----------------------------------------------------
	Process_Set_Text(_TL("model creation"));

	bool	bResult	= Get_Model();

	//-----------------------------------------------------
	if( m_dimModel.Get_Cellsize() > Get_Cellsize() )	// scaling
	{
		for(i=0; i<m_nPredictors; i++)
		{
			delete(m_pPredictors[i]);

			m_pPredictors[i]	= pPredictors->asGrid(i);
		}
	}

	//-----------------------------------------------------
	if( bResult )
	{
		Process_Set_Text(_TL("model application"));

		bResult	= Set_Model();
	}

	//-----------------------------------------------------
	if( Parameters("MODEL_OUT")->asBool() )
	{
		CSG_Parameter_Grid_List	*pModel	= Parameters("MODEL")->asGridList();

		pModel->Del_Items();
		pModel->Add_Item(m_pModel[m_nPredictors]);

		for(i=0; i<m_nPredictors; i++)
		{
			pModel->Add_Item(m_pModel[i]);
		}
	}
	else
	{
		for(i=0; i<=m_nPredictors; i++)
		{
			delete(m_pModel[i]);
		}
	}

	SG_FREE_SAFE(m_pModel);
	SG_FREE_SAFE(m_pPredictors);

	Finalize();

	return( bResult );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Initialize(CSG_Shapes *pPoints, int iDependent, CSG_Parameter_Grid_List *pPredictors)
{
	//-----------------------------------------------------
	if( (m_nPredictors = pPredictors->Get_Count()) <= 0 )
	{
		return( false );
	}

	if( !pPoints->Get_Extent().Intersects(Get_System()->Get_Extent()) )
	{
		return( false );
	}

	//-----------------------------------------------------
	int	iPredictor;
	
	TSG_Grid_Resampling	Resampling	= GRID_RESAMPLING_BSpline;

	m_Points.Create   (SHAPE_TYPE_Point);
	m_Points.Set_Name (Parameters("DEPENDENT")->asString());
	m_Points.Add_Field(Parameters("DEPENDENT")->asString(), SG_DATATYPE_Double);

	for(iPredictor=0; iPredictor<pPredictors->Get_Count(); iPredictor++)
	{
		m_Points.Add_Field(pPredictors->asGrid(iPredictor)->Get_Name(), SG_DATATYPE_Double);
	}

	//-----------------------------------------------------
	for(int iPoint=0; iPoint<pPoints->Get_Count() && Set_Progress(iPoint, pPoints->Get_Count()); iPoint++)
	{
		CSG_Shape	*pPoint	= pPoints->Get_Shape(iPoint);

		if( !pPoint->is_NoData(iDependent) )
		{
			CSG_Vector	z(1 + m_nPredictors);	z[0]	= pPoint->asDouble(iDependent);
			TSG_Point	Point	= pPoint->Get_Point(0);
			bool		bAdd	= true;

			for(iPredictor=0; bAdd && iPredictor<m_nPredictors; iPredictor++)
			{
				if( !pPredictors->asGrid(iPredictor)->Get_Value(Point, z[iPredictor + 1], Resampling) )
				{
					bAdd	= false;
				}
			}

			if( bAdd )
			{
				(pPoint	= m_Points.Add_Shape())->Add_Point(Point);

				for(iPredictor=0; iPredictor<=m_nPredictors; iPredictor++)
				{
					pPoint->Set_Value(iPredictor, z[iPredictor]);
				}
			}
		}
	}

	//-----------------------------------------------------
	m_Weighting.Set_Parameters(&Parameters);

	return( m_Points.Get_Count() > m_nPredictors && m_Search.Initialize(&m_Points, -1) );
}

//---------------------------------------------------------
void CGW_Multi_Regression_Grid::Finalize(void)
{
	m_Search.Finalize();
	m_Points.Destroy();
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Get_Model(void)
{
	//-----------------------------------------------------
	for(int y=0; y<m_dimModel.Get_NY() && Set_Progress(y, m_dimModel.Get_NY()); y++)
	{
		for(int x=0; x<m_dimModel.Get_NX(); x++)
		{
			CSG_Regression_Weighted	Model;

			if( Get_Model(x, y, Model) )
			{
				m_pQuality->Set_Value(x, y, Model.Get_R2());

				m_pModel[m_nPredictors]->Set_Value(x, y, Model[0]);

				for(int i=0; i<m_nPredictors; i++)
				{
					m_pModel[i]->Set_Value(x, y, Model[i + 1]);
				}
			}
			else
			{
				m_pQuality->Set_NoData(x, y);

				for(int i=0; i<=m_nPredictors; i++)
				{
					m_pModel[i]->Set_NoData(x, y);
				}
			}
		}
	}

	//-----------------------------------------------------
	return( true );
}

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Get_Model(int x, int y, CSG_Regression_Weighted &Model)
{
	//-----------------------------------------------------
	TSG_Point	Point	= m_dimModel.Get_Grid_to_World(x, y);
	int			nPoints = m_Search.Set_Location(Point);

	CSG_Vector	Predictors(m_nPredictors);

	Model.Destroy();

	for(int iPoint=0; iPoint<nPoints; iPoint++)
	{
		double	ix, iy, iz;

		CSG_Shape	*pPoint = m_Search.Do_Use_All() && m_Search.Get_Point(iPoint, ix, iy, iz)
			? m_Points.Get_Shape((int)iz)
			: m_Points.Get_Shape(iPoint);

		for(int iPredictor=0; iPredictor<m_nPredictors; iPredictor++)
		{
			Predictors[iPredictor]	= pPoint->asDouble(1 + iPredictor);
		}

		Model.Add_Sample(
			m_Weighting.Get_Weight(SG_Get_Distance(Point, pPoint->Get_Point(0))),
			pPoint->asDouble(0),
			Predictors
		);
	}

	//-----------------------------------------------------
	return( Model.Calculate() );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Set_Model(void)
{
	CSG_Grid	*pRegression	= Parameters("REGRESSION")->asGrid();
	CSG_Grid	*pQuality		= Parameters("QUALITY"   )->asGrid();

	pRegression->Set_Name(CSG_String::Format(SG_T("%s [%s]"    ), m_Points.Get_Name(), _TL("GWR")));
	pQuality   ->Set_Name(CSG_String::Format(SG_T("%s [%s, %s]"), m_Points.Get_Name(), _TL("GWR"), _TL("Quality")));

	if( m_pQuality == Parameters("QUALITY")->asGrid() )
	{
		pQuality	= NULL;
	}

	//-----------------------------------------------------
	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		double	p_y	= Get_YMin() + y * Get_Cellsize();

		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			double	Value, p_x	= Get_XMin() + x * Get_Cellsize();

			if( Set_Model(p_x, p_y, Value) )
			{
				SG_GRID_PTR_SAFE_SET_VALUE(pRegression, x, y, Value);
				SG_GRID_PTR_SAFE_SET_VALUE(pQuality   , x, y, m_pQuality->Get_Value(p_x, p_y));
			}
			else
			{
				SG_GRID_PTR_SAFE_SET_NODATA(pRegression, x, y);
				SG_GRID_PTR_SAFE_SET_NODATA(pQuality   , x, y);
			}
		}
	}

	//-----------------------------------------------------
	Set_Residuals();

	return( true );
}

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Set_Model(double x, double y, double &Value)
{
	if( !m_pModel[m_nPredictors]->Get_Value(x, y, Value, GRID_RESAMPLING_BSpline) )
	{
		return( false );
	}

	double	Model, Predictor;

	for(int i=0; i<m_nPredictors; i++)
	{
		if( !m_pModel     [i]->Get_Value(x, y, Model    , GRID_RESAMPLING_BSpline)
		||  !m_pPredictors[i]->Get_Value(x, y, Predictor, GRID_RESAMPLING_NearestNeighbour) )
		{
			return( false );
		}

		Value	+= Model * Predictor;
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGW_Multi_Regression_Grid::Set_Residuals(void)
{
	CSG_Shapes	*pResiduals	= Parameters("RESIDUALS")->asShapes();

	if( !pResiduals )
	{
		return( false );
	}

	//-----------------------------------------------------
	pResiduals->Create(SHAPE_TYPE_Point, CSG_String::Format(SG_T("%s [%s]"), m_Points.Get_Name(), _TL("Residuals")));
	pResiduals->Add_Field(m_Points.Get_Field_Name(0), SG_DATATYPE_Double);
	pResiduals->Add_Field("TREND"	, SG_DATATYPE_Double);
	pResiduals->Add_Field("RESIDUAL", SG_DATATYPE_Double);

	//-------------------------------------------------
	for(int iShape=0; iShape<m_Points.Get_Count() && Set_Progress(iShape, m_Points.Get_Count()); iShape++)
	{
		CSG_Shape	*pShape	= m_Points.Get_Shape(iShape);
		double		 zShape	= pShape->asDouble(0);

		for(int iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
		{
			for(int iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
			{
				double	zRegression;

				TSG_Point	Point	= pShape->Get_Point(iPoint, iPart);

				if( Set_Model(Point.x, Point.y, zRegression) )
				{
					CSG_Shape	*pResidual	= pResiduals->Add_Shape();

					pResidual->Add_Point(Point);
					pResidual->Set_Value(0, zShape);
					pResidual->Set_Value(1, zRegression);
					pResidual->Set_Value(2, zShape - zRegression);
				}
			}
		}
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
