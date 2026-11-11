#ifndef MOON_POLE_SHOP_ADMISSION_H
#define MOON_POLE_SHOP_ADMISSION_H
/* Shop events request native placement; they do not participate in power flow.
 * Retain a request only for its receipt-time managed world/walker. */
static struct {DRoot world,walker;int pending;} pp_shop_request;
static void pp_shop_clear(void){
    ddrop(&pp_shop_request.world);ddrop(&pp_shop_request.walker);pp_shop_request.pending=0;
}
static void pp_shop_update(Context *context){
    if(bm.disabled)return;
    DWORD thread=GetCurrentThreadId();
    if(bm.thread&&bm.thread!=thread)return;
    if(!bm.thread)bm.thread=thread;
    int buy=moon_shop_poll();
    if(!buy&&!pp_shop_request.pending&&!bm.world&&!bm.walker)return;
    PTR walker=(PTR)context->rcx,world=0;
    if(walker)field_value(walker,bm.world_field,&world);
    if(pp_shop_request.pending&&(world!=dget(pp_shop_request.world)||walker!=dget(pp_shop_request.walker)))pp_shop_clear();
    if(world!=dget(bm.world)||walker!=dget(bm.walker)||!world){
        ddrop(&bm.world);ddrop(&bm.walker);bm.eligible=bm.host_ready=0;
    }
    if(!world){pp_shop_clear();return;}
    if(buy){
        pp_shop_clear();pp_shop_request.world=dhold(world);pp_shop_request.walker=dhold(walker);
        if(!pp_shop_request.world||!pp_shop_request.walker){pp_shop_clear();return;}
        pp_shop_request.pending=1;
    }
    if(!pp_shop_request.pending)return;
    /* Context adoption and frame deduplication must not consume the first
     * request. A completed eligibility decision consumes it even when denied. */
    int state=bframe(context);if(state<=0)return;
    pp_shop_clear();
    if(bm.eligible)build_menu->begin_placement(walker,PP_KIND);
}
#endif
