#include "cssysdef.h"
#include "awsscrbr.h"
#include "aws3dfrm.h"
#include "awskcfct.h"
#include "awsslot.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsScrollBar)
SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsScrollBar::signalChanged=0x1;

const int awsScrollBar::fsVertical =0x0;
const int awsScrollBar::fsHorizontal =0x1;

awsScrollBar::awsScrollBar():is_down(false), mouse_is_over(false), 
was_down(false), tex(NULL),
frame_style(0), alpha_level(92),
decVal(NULL), incVal(NULL), 
sink(NULL), slot(NULL),
value(0), max(1), min(0), amntvis(0),
value_delta(0.1), value_page_delta(0.25)
{
}

awsScrollBar::~awsScrollBar()
{
  slot->Disconnect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  slot->Disconnect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));

  SCF_DEC_REF(incVal);
  SCF_DEC_REF(decVal);
  SCF_DEC_REF(sink);
  SCF_DEC_REF(slot);
}

char *
awsScrollBar::Type() 
{
  return "Scroll Bar";
}

bool
awsScrollBar::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup(_wmgr, settings)) return false;

  iAwsPrefManager *pm=WindowManager()->GetPrefMgr();

  pm->LookupIntKey("OverlayTextureAlpha", alpha_level); // global get
  pm->GetInt(settings, "Style", frame_style);
  pm->GetInt(settings, "Alpha", alpha_level);          // local overrides, if present.
  tex=pm->GetTexture("Texture");

  // Setup embedded buttons
  incVal = new awsCmdButton;
  decVal = new awsCmdButton;

  awsKeyFactory incinfo, decinfo;

  decinfo.Initialize(new scfString("decVal"), new scfString("Command Button"));
  incinfo.Initialize(new scfString("incVal"), new scfString("Command Button"));

  decinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);
  incinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);

  switch (frame_style)
  {
  case fsVertical:
    {

      incimg = pm->GetTexture("ScrollBarDn");
      decimg = pm->GetTexture("ScrollBarUp");

      // Abort if the images are not found
      if (!incimg || !decimg)
        return false;

      int img_w, img_h;

      incimg->GetOriginalDimensions(img_w, img_h);

      decinfo.AddRectKey(new scfString("Frame"), 
                         csRect(Frame().xmin, Frame().ymin,
                                Frame().xmax, Frame().ymin+img_h+5));

      incinfo.AddRectKey(new scfString("Frame"), 
                         csRect(Frame().xmin, Frame().ymax-img_h-5,
                                Frame().xmax, Frame().ymax));
    } break;


  default:  
    {

      incimg = pm->GetTexture("ScrollBarRt");
      decimg = pm->GetTexture("ScrollBarLt");

      // Abort if the images are not found
      if (!incimg || !decimg)
        return false;

      int img_w, img_h;

      incimg->GetOriginalDimensions(img_w, img_h);

      decinfo.AddRectKey(new scfString("Frame"), 
                         csRect(Frame().xmin, Frame().ymin,
                                Frame().xmin+img_w+5, Frame().ymax));

      incinfo.AddRectKey(new scfString("Frame"), 
                         csRect(Frame().xmax-img_w-5, Frame().ymin,
                                Frame().xmax, Frame().ymax));
    } break;
  } // end switch framestyle

  decVal->Setup(_wmgr, decinfo.GetThisNode());
  incVal->Setup(_wmgr, incinfo.GetThisNode());

  sink = new awsSink(this);

  sink->RegisterTrigger("DecValue", &DecClicked);
  sink->RegisterTrigger("IncValue", &IncClicked);

  slot = new awsSlot();

  slot->Connect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  slot->Connect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));

  return true;
}

bool 
awsScrollBar::GetProperty(char *name, void **parm)
{
  if (awsComponent::GetProperty(name, parm)) return true;

  return false;
}

bool 
awsScrollBar::SetProperty(char *name, void *parm)
{
  if (awsComponent::SetProperty(name, parm)) return true;

  return false;
}

void 
awsScrollBar::IncClicked(void *sk, iAwsSource *source)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value+=sb->value_delta;

  /// Check floor and ceiling
  sb->value = ( sb->value < sb->min ? sb->min : 
               ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged); 
  sb->Invalidate();
}

void 
awsScrollBar::DecClicked(void *sk, iAwsSource *source)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value-=sb->value_delta;

  /// Check floor and ceiling
  sb->value = ( sb->value < sb->min ? sb->min : 
                ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged); 
  sb->Invalidate();
}

void 
awsScrollBar::OnDraw(csRect clip)
{
  aws3DFrame frame3d;

  frame3d.Draw(WindowManager(), Window(), Frame(), frame_style, tex, alpha_level);
}

bool 
awsScrollBar::OnMouseDown(int button , int x , int y)
{ 
  if (!incVal->OnMouseDown(button,x,y))
    return decVal->OnMouseDown(button,x,y);
  
  return false;
}

bool 
awsScrollBar::OnMouseUp(int button, int x, int y)
{
  if (!incVal->OnMouseUp(button,x,y))
    return decVal->OnMouseUp(button,x,y);

  return false;
}

bool
awsScrollBar::OnMouseMove(int button, int x, int y)
{ 
  if (!incVal->OnMouseMove(button,x,y))
    return decVal->OnMouseMove(button,x,y);

  return false;
}

bool
awsScrollBar::OnMouseClick(int ,int ,int )
{
  return false;
}

bool
awsScrollBar::OnMouseDoubleClick(int ,int ,int )
{
  return false;
}

bool 
awsScrollBar::OnMouseExit()
{
  mouse_is_over=false;
  Invalidate();

  if (is_down)
    is_down=false;

  return true;
}

bool
awsScrollBar::OnMouseEnter()
{
  mouse_is_over=true;
  Invalidate();
  return true;
}

bool
awsScrollBar::OnKeypress(int ,int )
{
  return false;
}

bool
awsScrollBar::OnLostFocus()
{
  return false;
}

bool 
awsScrollBar::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsScrollBarFactory)
SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsScrollBarFactory::awsScrollBarFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Scroll Bar");
  RegisterConstant("sbfsVertical",  awsScrollBar::fsVertical);
  RegisterConstant("sbfsHorizontal", awsScrollBar::fsHorizontal);

  RegisterConstant("signalScrollBarChanged",  awsScrollBar::signalChanged);
}

awsScrollBarFactory::~awsScrollBarFactory()
{
  // empty
}

iAwsComponent *
awsScrollBarFactory::Create()
{
  return new awsScrollBar; 
}


