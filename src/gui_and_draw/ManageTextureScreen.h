//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
#define _VSP_GUI_TEXTURE_MANAGER_SCREEN_H

#include "VspScreenQt.h"

class ManageTextureScreenPrivate;
class ManageTextureScreen : public VspScreenQt
{
    VSP_DECLARE_PRIVATE( ManageTextureScreen )
public:
    ManageTextureScreen( ScreenMgr * mgr );
    ~ManageTextureScreen();
};

#endif // _VSP_GUI_TEXTURE_MANAGER_SCREEN_H
