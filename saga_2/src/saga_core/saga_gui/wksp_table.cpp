
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    User Interface                     //
//                                                       //
//                    Program: SAGA                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    WKSP_Table.cpp                     //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
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
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <saga_api/saga_api.h>

#include "res_commands.h"
#include "res_dialogs.h"

#include "helper.h"

#include "wksp_table.h"

#include "view_table.h"
#include "view_table_diagram.h"
#include "view_scatterplot.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Table::CWKSP_Table(CTable *pTable, CWKSP_Base_Item *pOwner)
{
	m_pTable	= pTable;
	m_pOwner	= pOwner;
	m_pView		= NULL;
	m_pDiagram	= NULL;
}

//---------------------------------------------------------
CWKSP_Table::~CWKSP_Table(void)
{
	Set_View	(false);
	Set_Diagram	(false);

	if( m_pOwner->Get_Type() == WKSP_ITEM_Table_Manager )
	{
		MSG_General_Add(wxString::Format("%s: %s...", LNG("[MSG] Close table"), m_pTable->Get_Name()), true, true);

		delete(m_pTable);

		MSG_General_Add(LNG("[MSG] okay"), false);
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
wxString CWKSP_Table::Get_Name(void)
{
	return( m_pTable->Get_Name() );
}

//---------------------------------------------------------
wxString CWKSP_Table::Get_Description(void)
{
	wxString	s;

	//-----------------------------------------------------
	s.Append(wxString::Format("<b>%s</b><table border=\"0\">",
		LNG("[CAP] Shapes")
	));

	s.Append(wxString::Format("<tr><td>%s</td><td>%s</td></tr>",
		LNG("[CAP] Name")					, m_pTable->Get_Name()
	));

	s.Append(wxString::Format("<tr><td>%s</td><td>%s</td></tr>",
		LNG("[CAP] File")					, m_pTable->Get_File_Path()
	));

	s.Append(wxString::Format("<tr><td>%s</td><td>%d</td></tr>",
		LNG("[CAP] Fields")					, m_pTable->Get_Field_Count()
	));

	s.Append(wxString::Format("<tr><td>%s</td><td>%d</td></tr>",
		LNG("[CAP] Records")				, m_pTable->Get_Record_Count()
	));

	s.Append("</table>");

	//-----------------------------------------------------
	s.Append(wxString::Format("<hr><b>%s</b>", LNG("[CAP] Table Description")));
	s.Append(Get_TableInfo_asHTML(m_pTable));

	//-----------------------------------------------------
	s.Append(wxString::Format("<hr><b>%s</b><font size=\"-1\">", LNG("[CAP] Data History")));
	s.Append(m_pTable->Get_History().Get_HTML());
	s.Append(wxString::Format("</font"));

	//-----------------------------------------------------
	return( s );
}

//---------------------------------------------------------
wxMenu * CWKSP_Table::Get_Menu(void)
{
	wxMenu	*pMenu;

	pMenu	= new wxMenu(m_pTable->Get_Name());

	switch( m_pOwner->Get_Type() )
	{
	default:
	case WKSP_ITEM_Table_Manager:
		CMD_Menu_Add_Item(pMenu, false, ID_CMD_WKSP_ITEM_CLOSE);
		CMD_Menu_Add_Item(pMenu, false, ID_CMD_TABLES_SAVE);
		CMD_Menu_Add_Item(pMenu, false, ID_CMD_TABLES_SAVEAS);
		break;

	case WKSP_ITEM_Shapes:
	case WKSP_ITEM_TIN:
		break;
	}

	CMD_Menu_Add_Item(pMenu,  true, ID_CMD_TABLES_SHOW);
	CMD_Menu_Add_Item(pMenu,  true, ID_CMD_TABLES_DIAGRAM);
	CMD_Menu_Add_Item(pMenu, false, ID_CMD_TABLES_SCATTERPLOT);

	return( pMenu );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Table::On_Command(int Cmd_ID)
{
	switch( Cmd_ID )
	{
	default:
		return( CWKSP_Base_Item::On_Command(Cmd_ID) );

	case ID_CMD_TABLES_SAVE:
		Save(m_pTable->Get_File_Path());
		break;

	case ID_CMD_TABLES_SAVEAS:
		Save();
		break;

	case ID_CMD_WKSP_ITEM_RETURN:
		Set_View(true);
		break;

	case ID_CMD_TABLES_SHOW:
		Toggle_View();
		break;

	case ID_CMD_TABLES_DIAGRAM:
		Toggle_Diagram();
		break;

	case ID_CMD_TABLES_SCATTERPLOT:
		Add_ScatterPlot(Get_Table());
		break;
	}

	return( true );
}

//---------------------------------------------------------
bool CWKSP_Table::On_Command_UI(wxUpdateUIEvent &event)
{
	switch( event.GetId() )
	{
	default:
		return( CWKSP_Base_Item::On_Command_UI(event) );

	case ID_CMD_TABLES_SAVE:
		event.Enable(m_pTable->is_Modified() && m_pTable->Get_File_Path() && strlen(m_pTable->Get_File_Path()) > 0);
		break;

	case ID_CMD_TABLES_SHOW:
		event.Check(m_pView != NULL);
		break;

	case ID_CMD_TABLES_DIAGRAM:
		event.Check(m_pDiagram != NULL);
		break;
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Table::Save(void)
{
	bool		bResult	= false;
	wxString	File_Path;

	if( DLG_Save(File_Path, ID_DLG_TABLES_SAVE) )
	{
		bResult	= m_pTable->Save(File_Path);

		PROCESS_Set_Okay();
	}

	return( bResult );
}

//---------------------------------------------------------
bool CWKSP_Table::Save(const char *File_Path)
{
	bool	bResult;

	if( File_Path && strlen(File_Path) > 0 )
	{
		bResult	= m_pTable->Save(File_Path);

		PROCESS_Set_Okay();

		return( bResult );
	}

	return( Save() );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Table::DataObject_Changed(CParameters *pParameters)
{
	Update_Views();

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Table::Set_View(bool bShow)
{
	if( bShow && !m_pView )
	{
		m_pView	= new CVIEW_Table(this);
	}
	else if( !bShow && m_pView )
	{
		m_pView->Destroy();
		delete(m_pView);
	}
}

//---------------------------------------------------------
void CWKSP_Table::Toggle_View(void)
{
	Set_View( m_pView == NULL );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Table::Set_Diagram(bool bShow)
{
	if( bShow && !m_pDiagram )
	{
		m_pDiagram	= new CVIEW_Table_Diagram(this);
	}
	else if( !bShow && m_pDiagram )
	{
		m_pDiagram->Destroy();
		delete(m_pDiagram);
	}
}

//---------------------------------------------------------
void CWKSP_Table::Toggle_Diagram(void)
{
	Set_Diagram( m_pDiagram == NULL );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Table::Update_Views(void)
{
	if( m_pView )
	{
		m_pView->Update_Table();
	}

	if( m_pDiagram )
	{
		m_pDiagram->Update_Diagram();
	}
}

//---------------------------------------------------------
void CWKSP_Table::View_Closes(wxMDIChildFrame *pView)
{
	if		( wxDynamicCast(pView, CVIEW_Table) )
	{
		m_pView		= NULL;
	}
	else if	( wxDynamicCast(pView, CVIEW_Table_Diagram) )
	{
		m_pDiagram	= NULL;
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
