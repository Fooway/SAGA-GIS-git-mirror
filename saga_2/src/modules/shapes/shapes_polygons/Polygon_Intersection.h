
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                    shapes_polygons                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                 Polygon_Intersection.h                //
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
//                                                       //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef HEADER_INCLUDED__Polygon_Intersection_H
#define HEADER_INCLUDED__Polygon_Intersection_H


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "MLB_Interface.h"
#include "Polygon_Intersection_GPC.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CPolygon_Intersection : public CModule  
{
public:
	CPolygon_Intersection(void);
	virtual ~CPolygon_Intersection(void);


protected:

	virtual bool			On_Execute					(void);


private:

	bool					bSplitParts;


	bool					Get_Polygon_Intersection	(CShapes *pShapes_A, CShapes *pShapes_B, CShapes *pShapes_AB);
	bool					Get_Difference				(CShapes *pShapes_A, CShapes *pShapes_B, CShapes *pShapes_AB, int bTargetID);

	bool					GPC_Polygon_Intersection	(CShape *pShape_A, CShape *pShape_B, CShape *pShape_AB);
	bool					GPC_Difference				(CShape *pShape_A, CShape *pShape_B, CShape *pShape_AB);

	bool					GPC_Create_Polygon			(CShape *pShape, gpc_polygon *pPolygon);

	void					Add_Polygon					(CShapes *pShapes, CShape *pShape, int ID_A, int ID_B);
	void					Add_Polygon					(CShapes *pShapes, CShape *pShape, int ID);

};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__Polygon_Intersection_H
