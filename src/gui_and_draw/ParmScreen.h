//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef PARMSCREEN_H__INCLUDED__
#define PARMSCREEN_H__INCLUDED__

#include "VspScreenQt.h"

class Parm;

class ParmScreen : public VspScreenQt
{
    VSP_DECLARE_PRIVATE( ParmScreen )
public:
    ParmScreen( ScreenMgr* mgr );
    ~ParmScreen();

    /// \todo This method is not used anywhere.
    void Update( const std::string & parm );
    /// \todo This method is not used anywhere.
    void Show( const std::string & parm );
};

#endif // PARMSCREEN_H__INCLUDED__
