//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef _VSP_GUI_LABEL_MANAGER_SCREEN_H
#define _VSP_GUI_LABEL_MANAGER_SCREEN_H

#include "VspScreenQt.h"
#include <vector>
#include <string>

class vec3d;
class DrawObj;

class ManageLabelScreen : public VspScreenQt
{
    VSP_DECLARE_PRIVATE( ManageLabelScreen )
public:
    ManageLabelScreen(ScreenMgr * mgr);
    ~ManageLabelScreen();

    void LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec);

    /*!
    * Set feedback values.
    * placement - position feedback of label.
    * targetGeomId - Id of geometry where this label is attached.
    */
    void Set(const vec3d & placement, const std::string & targetGeomId = "");

    virtual std::string getFeedbackGroupName();
};

#endif // _VSP_GUI_LABEL_MANAGER_SCREEN_H
