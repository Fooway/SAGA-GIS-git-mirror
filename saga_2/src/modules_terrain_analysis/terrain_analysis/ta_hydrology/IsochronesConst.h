/*******************************************************************************
    IsochronesConst.h
    Copyright (C) Victor Olaya
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/ 

#ifndef HEADER_INCLUDED__IsochronesConst_H
#define HEADER_INCLUDED__IsochronesConst_H

#include "MLB_Interface.h"

class CIsochronesConst : public CModule_Grid_Interactive {

private:
	CGrid *m_pDEM;
	CGrid *m_pTime;	
	void writeTimeOut(int,int,int,int);

public:
	CIsochronesConst(void);
	virtual ~CIsochronesConst(void);

protected:
	virtual bool On_Execute(void);
	virtual bool On_Execute_Finish(void);
	virtual bool On_Execute_Position(CSG_Point ptWorld, TModule_Interactive_Mode Mode);

};

#endif // #ifndef HEADER_INCLUDED__IsochronesConst_H
