//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageLabelScreen.h"
#include "UiSignalBlocker.h"
#include "ScreenMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "LabelMgr.h"
#include "StlHelper.h"
#include "GuiDeviceQt.h"
#include "VspScreenQt_p.h"
#include "ui_LabelScreen.h"
#include <string>

class ManageLabelScreen::Private : public QDialog, public VspScreenQt::Private
{
    Q_OBJECT
    VSP_DECLARE_PUBLIC( ManageLabelScreen )
    Q_PRIVATE_SLOT( self(), void SetUpdateFlag() )

    Ui::LabelScreen Ui;

    Slider RedSlider;
    Slider GreenSlider;
    Slider BlueSlider;
    Slider SizeSlider;

    std::vector<DrawObj> LabelList;
    std::vector<DrawObj> PickList;
    std::vector<std::string> Current;

    Private( ManageLabelScreen * q );
    QWidget * widget() Q_DECL_OVERRIDE { return this; }
    bool Update() Q_DECL_OVERRIDE;
    void LoadDrawObjs( const std::vector< DrawObj* > & draw_obj_vec );

    std::string GenerateName();

    DrawObj * Find( const std::string & geomID );

    void UpdateDrawObjs();
    void UpdatePickList();
    void UpdateNameInput();

    void UpdateRulerStartDO( DrawObj * targetDO, Ruler * ruler );
    void UpdateRulerEndDO( DrawObj * targetDO, Ruler * ruler );

    std::string GenerateRuler();
    void RemoveRuler( const std::string & geomId );

    Q_SLOT void on_addLabelButton_clicked()
    {
        Current.clear();
        Current.push_back( GenerateRuler() );
        UpdateNameInput();
    }
    Q_SLOT void on_removeLabelButton_clicked()
    {
        if( ! Current.empty() )
        {
            const_foreach( auto & current, Current )
            {
                RemoveRuler( current );
            }
            Current.clear();
        }
        UpdateNameInput();
    }
    Q_SLOT void on_selAllButton_clicked()
    {
        Current.clear();
        const_foreach( auto & label, LabelList )
        {
            Current.push_back( label.m_GeomID );
        }
        UpdateNameInput();
    }
    Q_SLOT void on_labelBrowser_itemSelectionChanged()
    {
        std::vector<Label*> labels = veh()->getVGuiDraw()->getLabelMgr()->GetVec();
        Q_ASSERT( labels.size() == Ui.labelBrowser->count() );

        Current.clear();
        for ( int i = 0; i < (int)Ui.labelBrowser->count(); i++ )
        {
            if( Ui.labelBrowser->item(i)->isSelected() )
            {
                Current.push_back( labels[i]->GetID() );
            }
        }
        UpdateNameInput();
    }
    Q_SLOT void on_nameInput_textChanged( const QString & val )
    {
        const_foreach( auto & current, Current )
        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );
            Q_ASSERT(label);
            label->SetName( val.toStdString() );
        }
    }
    Q_SLOT void on_redSlider_valueChanged( double val )
    {
        const_foreach( auto & current, Current )
        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );
            Q_ASSERT(label);
            label->m_Red = val;
        }
    }
    Q_SLOT void on_greenSlider_valueChanged( double val )
    {
        const_foreach( auto & current, Current )
        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );
            Q_ASSERT(label);
            label->m_Green = val;
        }
    }
    Q_SLOT void on_blueSlider_valueChanged( double val )
    {
        const_foreach( auto & current, Current )        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );
            Q_ASSERT(label);
            label->m_Blue = val;
        }
    }
    Q_SLOT void on_textSizeSlider_valueChanged( double val )
    {
        const_foreach( auto & current, Current )
        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );
            Q_ASSERT(label);
            label->m_Size = val;
        }
    }
    Q_SLOT void on_attachRulerButton_clicked()
    {
        if ( Current.size() == 1 )
        {
            Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( Current[0] );
            Q_ASSERT(label);
            label->Reset();
        }
    }
};
VSP_DEFINE_PRIVATE( ManageLabelScreen )

ManageLabelScreen::Private::Private( ManageLabelScreen * q ) :
    VspScreenQt::Private( q )
{
    Ui.setupUi( this );
    Ui.redSlider->setColorization( Qt::red );
    Ui.greenSlider->setColorization( Qt::green );
    Ui.blueSlider->setColorization( Qt::blue );
    RedSlider.Init( q, Ui.redSlider, 256 );
    GreenSlider.Init( q, Ui.greenSlider, 256 );
    BlueSlider.Init( q, Ui.blueSlider, 256 );
    SizeSlider.Init( q, Ui.textSizeSlider, 50 );
    BlockSignalsInUpdates();
    ConnectUpdateFlag();
}

ManageLabelScreen::ManageLabelScreen(ScreenMgr * mgr) :
    VspScreenQt( *new ManageLabelScreen::Private( this ), mgr )
{
}

bool ManageLabelScreen::Private::Update()
{
    std::vector<Label*> labels = veh()->getVGuiDraw()->getLabelMgr()->GetVec();

    // Add labels to browser.
    Ui.labelBrowser->clear();
    for (int i = 0; i < (int)labels.size(); i++)
    {
        Ui.labelBrowser->addItem( labels[i]->GetName().c_str() );
        const_foreach( auto & current, Current )
        {
            if ( labels[i]->GetID() == current )
            {
                Ui.labelBrowser->setCurrentRow( i ); // select?
            }
        }
    }

    // Ruler / Text Panel.
    if (Current.size() == 1)
    {
        Ruler * rulerType = dynamic_cast<Ruler*>(veh()->getVGuiDraw()->getLabelMgr()->Get(Current[0]));
        if (rulerType)
        {
            Ui.pageStack->setCurrentWidget( Ui.rulerUIGroup );
        }
    }
    else
    {
        Ui.pageStack->setCurrentWidget( Ui.noGroup );
    }

    const_foreach( auto & current, Current )
    {
        Label * label = veh()->getVGuiDraw()->getLabelMgr()->Get( current );

        RedSlider.Update( label->m_Red.GetID() );
        GreenSlider.Update( label->m_Green.GetID() );
        BlueSlider.Update( label->m_Blue.GetID() );

        SizeSlider.Update( label->m_Size.GetID() );
    }

    return true;
}

void ManageLabelScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    V_D( ManageLabelScreen );
    d->UpdateDrawObjs();
    for ( auto it = d->LabelList.begin(); it != d->LabelList.end(); ++it )
    {
        DrawObj & label = *it;
        // Render completed rulers.
        if ( label.m_Ruler.Step == DrawObj::VSP_RULER_STEP_COMPLETE )
        {
            draw_obj_vec.push_back( &label );
        }
        // Render current selected ruler.
        else if ( d->Current.size() == 1 && label.m_GeomID == d->Current[0] )
        {
            draw_obj_vec.push_back( &label );
        }
    }

    d->UpdatePickList();
    for ( auto it = d->PickList.begin(); it != d->PickList.end(); ++it )
    {
        draw_obj_vec.push_back( &*it );
    }
}

void ManageLabelScreen::Set(const vec3d &placement, const std::string & targetGeomId)
{
    V_D( ManageLabelScreen );

    if( d->Current.size() == 1 )
    {
        vec2d uw;

        Label * currLabel = d->veh()->getVGuiDraw()->getLabelMgr()->Get( d->Current[0] );

        Ruler * rulerLabel = dynamic_cast<Ruler*>(currLabel);
        if(rulerLabel)
        {
            if(rulerLabel->m_Stage == STAGE_ZERO)
            {
                rulerLabel->m_Stage = STAGE_ONE;
                rulerLabel->m_OriginGeomID = targetGeomId;

                uw = Ruler::MapToUW( targetGeomId, placement );

                rulerLabel->m_OriginU = uw.x();
                rulerLabel->m_OriginW = uw.y();
            }
            else if(rulerLabel->m_Stage == STAGE_ONE)
            {
                rulerLabel->m_Stage = STAGE_TWO;
                rulerLabel->m_RulerEndGeomID = targetGeomId;

                uw = Ruler::MapToUW( targetGeomId, placement );

                rulerLabel->m_RulerEndU = uw.x();
                rulerLabel->m_RulerEndW = uw.y();
            }
            else if(rulerLabel->m_Stage == STAGE_TWO)
            {
                rulerLabel->m_Stage = STAGE_COMPLETE;

                rulerLabel->m_XOffset = placement.x();
                rulerLabel->m_YOffset = placement.y();
                rulerLabel->m_ZOffset = placement.z();
            }
        }
    }
}

std::string ManageLabelScreen::getFeedbackGroupName()
{
    return std::string("LabelGUIGroup");
}

std::string ManageLabelScreen::Private::GenerateRuler()
{
    DrawObj rulerObj;
    rulerObj.m_GeomID = veh()->getVGuiDraw()->getLabelMgr()->CreateAndAddRuler(GenerateName());
    rulerObj.m_Type = DrawObj::VSP_RULER;
    rulerObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    rulerObj.m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;
    rulerObj.m_TextColor = vec3d(0, 0, 0);
    rulerObj.m_TextSize = 0;
    rulerObj.m_Visible = false;
    LabelList.push_back(rulerObj);

    return rulerObj.m_GeomID;
}

void ManageLabelScreen::Private::RemoveRuler(const string &geomId)
{
    veh()->getVGuiDraw()->getLabelMgr()->Remove( geomId );
    for ( int i = 0; i < (int)LabelList.size(); i++ )
    {
        if( LabelList[i].m_GeomID == geomId )
        {
            LabelList.erase( LabelList.begin() + i );
        }
    }
}

std::string ManageLabelScreen::Private::GenerateName()
{
    static unsigned int uniqueIndex = 0;
    auto name = QString("Ruler_%1").arg( uniqueIndex );
    uniqueIndex++;
    return name.toStdString();
}

DrawObj * ManageLabelScreen::Private::Find( const std::string & geomID )
{
    for ( int i = 0; i < (int)LabelList.size(); i++ )
    {
        if ( LabelList[i].m_GeomID == geomID )
        {
            return &LabelList[i];
        }
    }
    return NULL;
}

void ManageLabelScreen::Private::UpdateDrawObjs()
{
    std::vector<Label*> labelList = veh()->getVGuiDraw()->getLabelMgr()->GetVec();
    for(int i = 0; i < (int)labelList.size(); i++)
    {
        // Find out label type.
        Ruler * rulerType = dynamic_cast<Ruler*>(labelList[i]);

        // Find draw object of this Label.  If no match, create one.
        DrawObj * match = Find(labelList[i]->GetID());
        if(!match)
        {
            DrawObj newDO;
            newDO.m_GeomID = labelList[i]->GetID();
            newDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

            LabelList.push_back( newDO );

            // Get matching draw object.
            match = Find(labelList[i]->GetID());
            Q_ASSERT(match);
        }

        match->m_Visible = labelList[i]->m_Visible.Get();

        match->m_TextColor = vec3d(labelList[i]->m_Red.Get() / 255,
            labelList[i]->m_Green.Get() / 255,
            labelList[i]->m_Blue.Get() / 255);

        match->m_TextSize = labelList[i]->m_Size.Get();

        // Set label base on type.
        if(rulerType)
        {
            match->m_Type = DrawObj::VSP_RULER;

            // Set label stage.  Load stage data.
            match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;

            if(rulerType->m_Stage == STAGE_ONE)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ONE;

                // Load ruler's starting info to DrawObj.
                UpdateRulerStartDO(match, rulerType);
            }
            else if(rulerType->m_Stage == STAGE_TWO)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_TWO;

                // Load ruler's starting and ending info to DrawObj. 
                UpdateRulerStartDO(match, rulerType);
                UpdateRulerEndDO(match, rulerType);
            }
            else if(rulerType->m_Stage == STAGE_COMPLETE)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_COMPLETE;

                // Load ruler's starting and ending info to DrawObj. 
                UpdateRulerStartDO(match, rulerType);
                UpdateRulerEndDO(match, rulerType);

                // Load placement info to DrawObj.
                match->m_Ruler.Offset = vec3d(rulerType->m_XOffset.Get(), 
                    rulerType->m_YOffset.Get(), 
                    rulerType->m_ZOffset.Get());
            }
        }
    }
}

void ManageLabelScreen::Private::UpdatePickList()
{
    V_Q( ManageLabelScreen );
    // Load all geom.
    vector< Geom* > geom_vec = veh()->FindGeomVec( veh()->GetGeomVec( false ) );

    PickList.clear();
    if ( Current.size() == 1 )
    {
        DrawObj * currDrawObj = Find( Current[0] );
        if(currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_COMPLETE)
        {
            // Do nothing.
        }
        else if(currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_TWO)
        {
            DrawObj pickDO;
            pickDO.m_Type = DrawObj::VSP_PICK_LOCATION;
            pickDO.m_GeomID = PICKLOCHEADER + currDrawObj->m_GeomID;
            pickDO.m_PickSourceID = "";
            pickDO.m_FeedbackGroup = q->getFeedbackGroupName();
            PickList.push_back( pickDO );
        }
        else if(currDrawObj)
        {
            for(int i = 0; i < (int)geom_vec.size(); i++)
            {
                vector<DrawObj*> geom_drawObj_vec;
                geom_vec[i]->LoadDrawObjs(geom_drawObj_vec);

                for(int j = 0; j < (int)geom_drawObj_vec.size(); j++)
                {
                    if(geom_drawObj_vec[j]->m_Visible)
                    {
                        // Ignore bounding boxes.
                        if(geom_drawObj_vec[j]->m_GeomID.compare(0, string(BBOXHEADER).size(), BBOXHEADER) != 0)
                        {
                            DrawObj pickDO;
                            pickDO.m_Type = DrawObj::VSP_PICK_VERTEX;
                            pickDO.m_GeomID = PICKVERTEXHEADER + geom_drawObj_vec[j]->m_GeomID;
                            pickDO.m_PickSourceID = geom_drawObj_vec[j]->m_GeomID;
                            pickDO.m_FeedbackGroup = q->getFeedbackGroupName();

                            PickList.push_back( pickDO );
                        }
                    }
                }
            }
        }
    }
}

void ManageLabelScreen::Private::UpdateNameInput()
{
    UiSignalBlocker block( Ui.nameInput );
    if ( Current.size() > 1 )
    {
        Ui.nameInput->setText( "<multiple>" );
    }
    else if( Current.size() == 1 )
    {
        Label * currLabel = veh()->getVGuiDraw()->getLabelMgr()->Get( Current[0] );
        Q_ASSERT(currLabel);
        Ui.nameInput->setText( currLabel->GetName().c_str() );
    }
    else
    {
        Ui.nameInput->clear();
    }
}

void ManageLabelScreen::Private::UpdateRulerStartDO( DrawObj * targetDO, Ruler * ruler )
{
    targetDO->m_Ruler.Start = Ruler::MapToXYZ(
        ruler->m_OriginGeomID, 
        vec2d(ruler->m_OriginU.Get(), ruler->m_OriginW.Get()));
}

void ManageLabelScreen::Private::UpdateRulerEndDO( DrawObj * targetDO, Ruler * ruler )
{
    targetDO->m_Ruler.End = Ruler::MapToXYZ(
        ruler->m_RulerEndGeomID, 
        vec2d( ruler->m_RulerEndU.Get(), ruler->m_RulerEndW.Get()) );
}

ManageLabelScreen::~ManageLabelScreen() {}

#include "ManageLabelScreen.moc"
