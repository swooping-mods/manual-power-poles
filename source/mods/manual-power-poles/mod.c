/* Native construction and power solving own pole lifetime and connections. */
#include "../../runtime/native/gameplay.h"
#include "building.h"
#include "../build-menu-framework/sdk/client.h"
#include "pole_model.h"
#include "assets/generated/store_icon.h"
#define PP_KIND 193
static const MoonBuildMenu *build_menu;
static int pp_ready,pp_test;
static int before(int hook,Context *context);
static void pp_update(void *walker){if(pp_ready){Context c;dzero(&c,sizeof(c));c.rcx=(U64)walker;before(0,&c);}}
int _fltused=0;
static void pp_test_aim_cycle(PTR walker){
    int ok=1;DRoot records=dhold(call(dmethod("MoonCleaners","MoonWorld","get_BuiltDepots",""),dget(bm.world),0,&ok));
    DRoot it=benum(dget(records),&ok);PTR record;
    while((record=bnext(it,&ok))){
        if(bint(record,"kind")!=PP_KIND)continue;
        int id=bint(record,"id");DVec point={0},origin={0},direction={0};void *a[]={&id,&point};
        PTR found=call(dmethod("MoonCleaners","MoonWorld","PowerAnchor","System.Int32;UnityEngine.Vector3&"),dget(bm.world),a,&ok);
        if(!ok||!found||!*(unsigned char *)unbox(found)||!baim(&origin,&direction))break;
        direction=(DVec){point.x-origin.x,point.y-origin.y,point.z-origin.z};void *look[]={&direction};
        call(dmethod("MoonCleaners","MoonWalker","TestLook","UnityEngine.Vector3"),walker,look,&ok);
        bevent(ok?"test-auto-pole-aimed":"test-auto-pole-aim-failed");break;
    }
    bend(&it);ddrop(&records);
}
static const MoonBuildDefinition pole_definition={
    .abi=MB_ABI,.size=sizeof(MoonBuildDefinition),.key="manual-power-poles.power-pole",
    .name="Power Pole",.category="Power",.description="Automatic passive 15 m power relay.",
    .icon_png=pp_icon_png,.icon_size=sizeof(pp_icon_png)-1,.icon_width=384,.icon_height=384,
    .kind=PP_KIND,.template_kind=93,.price=75,.power_node=1,.power_range=15,
    .configure=pp_configure,.release=pp_release,.update=pp_update
};
#include "test_tools.h"
#include "automatic_test.h"
#include "shop_admission.h"
static int before(int hook,Context *c){
    (void)hook;if(!pp_test){pp_shop_update(c);return 0;}
    int state=bframe(c),buy=moon_shop_poll();
    if(state<0){pp_auto_reset();bevent("session-reset");return 0;}
    if(state<=0)return 0;
    if(pp_test){pp_test_controls((PTR)c->rcx);pp_auto_test_update((PTR)c->rcx);}
    if(buy&&bm.eligible)build_menu->begin_placement(dget(bm.walker),PP_KIND);
    return 0;
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
    (void)prepare_hook;(void)install_hooks;
    if(!bstart(domain,game,"manual-power-poles"))return 0;
    char flag[8];pp_test=GetEnvironmentVariableA("MOONLAB_TEST_TOOLS",flag,sizeof(flag))&&flag[0]=='1';
    build_menu=mb_connect(domain,game);if(!build_menu||!build_menu->register_item(&pole_definition))return 0;
    before_hook=before;pp_ready=1;bevent("automatic-power-ready");return 1;
}
