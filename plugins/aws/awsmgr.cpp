#include "cssysdef.h"
#include "isys/plugin.h"
#include "aws.h"
#include "awsprefs.h"
#include "ivideo/txtmgr.h"
#include "iengine/engine.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <stdio.h>

awsManager::awsComponentFactoryMap::~awsComponentFactoryMap ()
{
  factory->DecRef ();
}

awsManager::awsManager(iBase *p):prefmgr(NULL), 
               top(NULL), ptG2D(NULL), ptG3D(NULL), object_reg(NULL), 
               UsingDefaultContext(false), DefaultContextInitialized(false)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  canvas.DisableAutoUpdate();
}

awsManager::~awsManager()
{
  SCF_DEC_REF (prefmgr);

  void *p = component_factories.GetFirstItem();
  while ((p=component_factories.GetCurrentItem ()))
  {
    delete (awsComponentFactoryMap *)p;
    component_factories.RemoveItem ();
  }
}

bool 
awsManager::Initialize(iObjectRegistry *object_reg)
{   
  awsManager::object_reg = object_reg;
//   iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
    
  printf("aws-debug: getting preference manager.\n");  
  iAwsPrefs *prefs =  SCF_CREATE_INSTANCE ("crystalspace.window.preferencemanager", iAwsPrefs);
  
  if (!prefs)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
        "AWS could not create an instance of the default preference manager.  This is a serious error.");
    return false;
  }
  else
  {
    printf("aws-debug: initing and setting the internal preference manager.\n");
    
    prefs->Setup(object_reg);

    printf("aws-debug: inited pref manager, now setting.\n");

    SetPrefMgr(prefs);

    printf("aws-debug: decRefing the prefs manager.\n");
    prefs->DecRef();
  }
      
  printf("aws-debug: left aws initialize.\n");
  return true;
}

iAwsPrefs *
awsManager::GetPrefMgr()
{
  return prefmgr;
}
 
void
awsManager::SetPrefMgr(iAwsPrefs *pmgr)
{
   if (prefmgr && pmgr)
   {
      prefmgr->DecRef();
      pmgr->IncRef();
      prefmgr=pmgr;
   }
   else if (pmgr)
   {
      pmgr->IncRef();
      prefmgr=pmgr;
   }
}

void
awsManager::RegisterComponentFactory(awsComponentFactory *factory, char *name)
{
   awsComponentFactoryMap *cfm = new awsComponentFactoryMap;

   factory->IncRef();

   cfm->factory= factory;
   cfm->id=prefmgr->NameToId(name);

   component_factories.AddItem(cfm);
}

awsComponentFactory *
awsManager::FindComponentFactory(char *name)
{
  void *p = component_factories.GetFirstItem();
  unsigned long id = prefmgr->NameToId(name);
  
  do 
  {
    awsComponentFactoryMap *cfm = (awsComponentFactoryMap *)p;
    
    if (cfm->id == id)
      return cfm->factory;
      
    p = component_factories.GetNextItem();
  } while(p!=component_factories.PeekFirstItem());
  
  return NULL;
}

awsWindow *
awsManager::GetTopWindow()
{ return top; }
    
void 
awsManager::SetTopWindow(awsWindow *_top)
{ top = _top; }

void 
awsManager::SetContext(iGraphics2D *g2d, iGraphics3D *g3d)
{
   if (g2d && g3d)
   {
       ptG2D = g2d;
       ptG3D = g3d;
       
       frame.Set(0,0,ptG2D->GetWidth(), ptG2D->GetHeight());
       
       UsingDefaultContext=false;

       Mark(frame);
   }
}

void 
awsManager::SetDefaultContext(iEngine* engine, iTextureManager* txtmgr)
{
  if (!DefaultContextInitialized)
  {
    canvas.SetSize(512, 512);
    canvas.SetKeyColor(255,0,255);
    if (!canvas.Initialize(object_reg, engine, txtmgr, "awsCanvas"))
      printf("aws-debug: SetDefaultContext failed to initialize the memory canvas.\n");
    else
      printf("aws-debug: Memory canvas initialized!\n");
    
    if (!canvas.PrepareAnim())
      printf("aws-debug: Prepare anim failed!\n");
    else
      printf("aws-debug: Prepare anim succeeded.\n");
   
    if (engine!=NULL)
    {
//       iTextureWrapper *tw = engine->GetTextureList()->NewTexture(canvas.GetTextureWrapper()->GetTextureHandle());
//       iMaterialWrapper *canvasMat = engine->CreateMaterial("awsCanvasMat", tw);
    }
    
    DefaultContextInitialized=true;
  }
            
  ptG2D = canvas.G2D();
  ptG3D = canvas.G3D();
  
  printf("aws-debug: G2D=%p G3D=%p\n", ptG2D, ptG3D);

  if (txtmgr)
    GetPrefMgr()->SetTextureManager(txtmgr);

  if (ptG2D)
    GetPrefMgr()->SetFontServer(ptG2D->GetFontServer());
    
  if (ptG2D && ptG3D) 
  {
    ptG2D->DoubleBuffer(false);
    ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
    ptG2D->Clear(txtmgr->FindRGB(255,0,255));
    ptG3D->FinishDraw();
    ptG3D->Print(NULL);
    
    frame.Set(0,0,ptG2D->GetWidth(), ptG2D->GetHeight());
    
    Mark(frame);
    UsingDefaultContext=true;
  }
}

void
awsManager::Mark(csRect &rect)
{
  dirty.Include(rect);
}

void
awsManager::Unmark(csRect &rect)
{
  dirty.Exclude(rect);  
}

bool
awsManager::WindowIsDirty(awsWindow *win)
{
  int i;
   
  for(i=0; i<dirty.Count(); ++i)
    if (win->Overlaps(dirty.RectAt(i))) return true;
  
  return false;
}

void
awsManager::Print(iGraphics3D *g3d)
{
  g3d->DrawPixmap(canvas.GetTextureWrapper()->GetTextureHandle(), 
  		  0,0,512,480,//g3d->GetWidth(), g3d->GetHeight(),
		  0,0,512,480,0);
  
}

void       
awsManager::Redraw()
{
   static unsigned redraw_tag = 0;
   static csRect bounds(0,0,512,512);
   int    erasefill = GetPrefMgr()->GetColor(AC_TRANSPARENT);

   redraw_tag++;
   
   ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
   
   ptG2D->SetClipRect(0,0,512,512);

   if (redraw_tag%2) ptG2D->DrawBox( 0,  0,25, 25, GetPrefMgr()->GetColor(AC_SHADOW));
   else              ptG2D->DrawBox( 0,  0,25, 25, GetPrefMgr()->GetColor(AC_HIGHLIGHT));
       
   // check to see if there is anything to redraw.
   if (dirty.Count() == 0) 
      return;
   
   /******* The following code is only executed if there is something to redraw *************/
   
   awsWindow *curwin=top, *oldwin = 0;
   
   // check to see if any part of this window needs redrawn
   while(curwin)
   {
      if (WindowIsDirty(curwin)) {
        curwin->SetRedrawTag(redraw_tag);
      }

      oldwin=curwin;
      curwin = curwin->WindowBelow();
   }

   /*  At this point in time, oldwin points to the bottom most window.  That means that we take curwin, set it
    * equal to oldwin, and then follow the chain up to the top, redrawing on the way.  This makes sure that we 
    * only redraw each window once.
    */
   
   curwin=oldwin;
   while(curwin)
   {
      if (curwin->RedrawTag() == redraw_tag) 
      {
        int i;
          
        for(i=0; i<dirty.Count(); ++i)
        {
          // Find out if we need to erase.
          csRect lo(dirty.RectAt(i));
          csRect dr(dirty.RectAt(i));
          lo.Subtract(curwin->Frame());

          if (!lo.IsEmpty())            
            ptG2D->DrawBox(dr.xmin, dr.ymin, dr.xmax, dr.ymax, erasefill);
          
          RedrawWindow(curwin, dr);
        }
      }
      curwin=curwin->WindowAbove();
   }

   // Reset the dirty region
   dirty.makeEmpty();

   // This only needs to happen when drawing to the default context.
   if (UsingDefaultContext)
   {
     ptG3D->FinishDraw ();
     ptG3D->Print (&bounds);
   }

   // done with the redraw!
}

void
awsManager::RedrawWindow(awsWindow *win, csRect &dirtyarea)
{
     /// See if this window intersects with this dirty area
     if (!dirtyarea.Intersects(win->Frame()))
       return;

     /// Draw the window first.
     csRect clip(win->Frame());

     /// Clip the window to it's intersection with the dirty rectangle
     clip.Intersect(dirtyarea);
     ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     /// Tell the window to draw
     win->OnDraw(clip);

     /// Now draw all of it's children
     RecursiveDrawChildren(win, dirtyarea);
}

void
awsManager::RecursiveDrawChildren(awsComponent *cmp, csRect &dirtyarea)
{
   awsComponent *child = cmp->GetFirstChild();

   while (child) 
   {
     // Check to see if this component even needs redrawing.
     if (!dirtyarea.Intersects(child->Frame()))
       continue;                                            

     csRect clip(child->Frame());
     clip.Intersect(dirtyarea);
     ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     // Draw the child
     child->OnDraw(clip);

     // If it has children, draw them
     if (child->HasChildren())
       RecursiveDrawChildren(child, dirtyarea);

    child = cmp->GetNextChild();
   }

}

awsWindow *
awsManager::CreateWindowFrom(char *defname)
{
   printf("aws-debug: Searching for window def \"%s\"\n", defname);
   
   // Find the window definition
   awsComponentNode *winnode = GetPrefMgr()->FindWindowDef(defname);
   
   printf("aws-debug: Window definition was %s\n", (winnode ? "found." : "not found."));
   
   // If we couldn't find it, abort
   if (winnode==NULL) return NULL;
   
   // Create a new window
   awsWindow *win = new awsWindow();
   
   // Tell the window to set itself up
   win->Setup(this, winnode);
   
   /* Now recurse through all of the child nodes, creating them and setting them
   up.  Nodes are created via their factory functions.  If a factory cannot be 
   found, then that node and all of it's children are ignored. */
   
   CreateChildrenFromDef(this, win, winnode);
     
   return win;
}

void
awsManager::CreateChildrenFromDef(iAws *wmgr, awsComponent *parent, awsComponentNode *settings)
{
  int i;
  for(i=0; i<settings->GetLength(); ++i)
  {
    awsKey *key = settings->GetItemAt(i); 
    
    if (key->Type() == KEY_COMPONENT)
    {
      awsComponentNode *comp_node = (awsComponentNode *)key;
      awsComponentFactory *factory = FindComponentFactory(comp_node->ComponentTypeName()->GetData());
      
      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
	awsComponent *comp = factory->Create();
		
	// Prepare the component, and add it into it's parent
	comp->Setup(wmgr, comp_node);
	parent->AddChild(comp);
	
	// Process all subcomponents of this component.
	CreateChildrenFromDef(wmgr, comp, comp_node);
      }
      
    }
      
  }
  
  
}

bool 
awsManager::HandleEvent(iEvent& Event)
{
  if (GetTopWindow())
    return GetTopWindow()->HandleEvent(Event);

  return false;
}

//// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas ()
{
  mat_w=512;
  mat_h=512;
  
  texFlags = CS_TEXTURE_2D | CS_TEXTURE_PROC;
   
}

void 
awsManager::awsCanvas::Animate (csTicks current_time)
{
  (void)current_time;
}

void 
awsManager::awsCanvas::SetSize(int w, int h)
{  mat_w=w; mat_h=h; }

