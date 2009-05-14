/******************************************************************************/
//                                                                        
// DISPLAY.CPP - Source code for the CPhoneDisplay class.
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains the code for driving a phone display.               
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::CPhoneDisplay
//
// Display constructor
//
CPhoneDisplay::CPhoneDisplay() : m_cLF('\n')
{
	m_sizDisplay.cx = m_sizDisplay.cy = 0;
	m_ptCursor.x = m_ptCursor.y = 0;

}// CPhoneDisplay::CPhoneDisplay

//////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::~CPhoneDisplay
//
// Display Destructor
//
CPhoneDisplay::~CPhoneDisplay()
{
}// CPhoneDisplay::~CPhoneDisplay

///////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::Init
//
// Initialize our display
//
void CPhoneDisplay::Init(int iColumnCount, int iRowCount, TCHAR cLF)
{
    // Allocate a buffer if we have coumns/rows
    if (iColumnCount > 0 && iRowCount > 0)
    {
        m_lpsDisplay.reset(new TCHAR[iColumnCount*iRowCount+1]);
        m_sizDisplay.cx = iColumnCount;
        m_sizDisplay.cy = iRowCount;
		m_cLF = cLF;
		Reset();
    }
   
}// CPhoneDisplay::Init

////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::AddCharacter
//
// Add a character at the cursor position.
//
void CPhoneDisplay::AddCharacter(TCHAR cChar)
{
    // If the character is our LINEFEED, then adjust the cursor position.
    if (cChar == m_cLF)
    {
        if (++m_ptCursor.y >= m_sizDisplay.cy)
            m_ptCursor.y = 0;
        m_ptCursor.x = 0;
    }
    else // Normal character.
    {   
        // Place the character into the buffer.
        SetCharacterAtPosition(-1, -1, cChar);
        // Increment our cursor position.
        if (++m_ptCursor.x >= m_sizDisplay.cx)
        {
            m_ptCursor.x = 0;
            if (++m_ptCursor.y >= m_sizDisplay.cy)
                m_ptCursor.y = 0;
        }
    }

}// CPhoneDisplay::AddCharacter

////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::SetCharacterAtPosition
//
// Set a character into the display at a particular position.
//
void CPhoneDisplay::SetCharacterAtPosition(int iColumn, int iRow, TCHAR cChar)
{
	_TSP_ASSERTE(m_lpsDisplay.get() != NULL);
	_TSP_ASSERTE(cChar != _T('\0'));
	_TSP_ASSERTE(cChar != m_cLF);

    if (iColumn == -1)
        iColumn = m_ptCursor.x;
    if (iRow == -1)
        iRow = m_ptCursor.y;
    
    // Validate the character.
    if (cChar && cChar != m_cLF &&
        iColumn >= 0 && iColumn < m_sizDisplay.cx &&
        iRow >= 0 && iRow < m_sizDisplay.cy)
    {
		LPTSTR pszBuffer = m_lpsDisplay.get();
        *(pszBuffer+(iRow*m_sizDisplay.cx+iColumn)) = cChar;
    }

}// CPhoneDisplay::SetCharacterAtPosition

///////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::SetCursorPosition
//
// Set the current cursor position for the display
//
void CPhoneDisplay::SetCursorPosition(int iColumn, int iRow)
{
    if (iColumn >= 0 && iColumn < m_sizDisplay.cx)
        m_ptCursor.x = iColumn;
    if (iRow >= 0 && iRow < m_sizDisplay.cy)
        m_ptCursor.y = iRow;

}// CPhoneDisplay::SetCursorPosition

////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::Reset
//
// Reset the contents of the display and clear the buffer.
// 
void CPhoneDisplay::Reset()
{
    m_ptCursor.x = m_ptCursor.y = 0;
    if (m_lpsDisplay.get())
    {
		LPTSTR lpsBuffer = m_lpsDisplay.get();
		for (int cy = 0; cy < m_sizDisplay.cy; cy++)
		{
			for (int cx = 0; cx < m_sizDisplay.cx; cx++)
				*lpsBuffer++ = _T(' ');
		}
        *lpsBuffer = _T('\0');
    }        

}// CPhoneDisplay::Reset

/////////////////////////////////////////////////////////////////////////////////
// CPhoneDisplay::ClearRow
//
// Clear a single row of our display.  Do NOT change the cursor
// positioning
//
void CPhoneDisplay::ClearRow(int iRow)
{
    if (iRow >= 0 && iRow < m_sizDisplay.cy)
        for (int x = 0; x < m_sizDisplay.cx; x++)
            SetCharacterAtPosition(x, iRow, _T(' '));

}// CPhoneDisplay::ClearRow
