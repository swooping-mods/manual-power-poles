#ifndef MOON_AUTO_POLE_TEST_H
#define MOON_AUTO_POLE_TEST_H
/* Opt-in private-fixture controls. All construction, moves, removal, inventory
 * acceptance and electrical state remain native; no Power/Running setters. */
static struct {
    MoonBuildPlacementApi api;MoonBuildPlacement placement;
    DVec points[5],up,right,away,origin,initial_right,initial_forward,staged_at;int ids[5],index,stage,moving,before_count,site,settle_frame;
    int before_ids[4096];
    float next_sample;unsigned previous;
} pp_auto;
static const int pp_auto_kinds[5]={93,96,96,193,193};
static void pp_auto_record(const char *event,int a,int b,int c){
    char line[320],*p=cat(line,"{\"event\":\"");p=cat(p,event);p=cat(p,"\",\"a\":");p=num(p,(U64)(unsigned)a);p=cat(p,",\"b\":");p=num(p,(U64)(unsigned)b);p=cat(p,",\"c\":");p=num(p,(U64)(unsigned)c);p=cat(p,"}\n");DWORD n=0;WriteFile(bm.log,line,(DWORD)(p-line),&n,0);FlushFileBuffers(bm.log);
}
static void pp_auto_reason_value(PTR value){
    int (*length)(PTR)=(int(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_length");
    const unsigned short *(*chars)(PTR)=(const unsigned short *(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_chars");
    if(!value||!length||!chars)return;int n=length(value);const unsigned short *s=chars(value);if(n<0||!s)return;
    char line[768],*p=cat(line,"{\"event\":\"auto-fixture-native-reason\",\"reason\":\"");
    for(int i=0;i<n&&i<240;i++){unsigned short c=s[i];if(c=='"'||c=='\\')*p++='\\';*p++=c>=32&&c<127?(char)c:'?';}
    p=cat(p,"\"}\n");DWORD written=0;WriteFile(bm.log,line,(DWORD)(p-line),&written,0);FlushFileBuffers(bm.log);
}
static void pp_auto_reason(PTR check){
    int ok=1;PTR value=call(get_method(object_class(check),"get_Reason",0),check,0,&ok);if(ok)pp_auto_reason_value(value);
}
static int pp_auto_commit(PTR walker){
    MoonBuildPlacement *p=&pp_auto.placement;
    if(!p->ready||p->attempted||!p->check||p->foundation||dget(p->world)!=dget(bm.world))return 0;
    p->attempted=1;int ok=1;PTR reason=0;void *a[]={walker,&p->point,&p->yaw,&reason,dget(p->check),&p->kind};
    PTR value=call(pp_auto.api.build_depot,dget(bm.world),a,&ok);int accepted=ok&&value&&*(unsigned char *)unbox(value);
    pp_auto_record("auto-fixture-native-commit",pp_auto.index,accepted,ok);
    if(!accepted)pp_auto_reason_value(reason);return accepted;
}
static int pp_auto_capture_ids(int kind){
    int ok=1;pp_auto.before_count=0;
    DRoot records=dhold(call(dmethod("MoonCleaners","MoonWorld","get_BuiltDepots",""),dget(bm.world),0,&ok));
    DRoot it=benum(dget(records),&ok);PTR record;
    while((record=bnext(it,&ok)))if(bint(record,"kind")==kind){
        if(pp_auto.before_count==4096){ok=0;break;}
        pp_auto.before_ids[pp_auto.before_count++]=bint(record,"id");
    }
    bend(&it);ddrop(&records);if(!ok)bevent("auto-fixture-id-snapshot-failed");return ok;
}
static int pp_auto_new_id(int kind){
    int ok=1,id=0,count=0;
    DRoot records=dhold(call(dmethod("MoonCleaners","MoonWorld","get_BuiltDepots",""),dget(bm.world),0,&ok));
    DRoot it=benum(dget(records),&ok);PTR record;
    while((record=bnext(it,&ok)))if(bint(record,"kind")==kind){
        int candidate=bint(record,"id"),known=0;for(int i=0;i<pp_auto.before_count;i++)if(pp_auto.before_ids[i]==candidate){known=1;break;}
        if(!known){id=candidate;count++;pp_auto_record("auto-fixture-new-record",kind,candidate,count);}
    }
    bend(&it);ddrop(&records);
    /* Authored landmarks have reserved IDs above the player's nextDepotId.
     * Identify the one actual addition; numerical maximum is not chronology. */
    return ok&&count==1&&id>0?id:0;
}
static int pp_auto_unit(DVec value,DVec *out){
    int ok=1;void *a[]={&value};PTR p=call(dmethod("UnityEngine","Vector3","Normalize","UnityEngine.Vector3"),0,a,&ok);
    if(!ok||!p)return 0;dcopy(out,unbox(p),sizeof(*out));float norm=out->x*out->x+out->y*out->y+out->z*out->z;return norm>.99f&&norm<1.01f;
}
static int pp_auto_radial(DVec point,DVec *out){
    int ok=1;PTR tr=call(bm.transform,dget(bm.world),0,&ok);DVec center=dvector(bm.position,tr,&ok);
    return ok&&pp_auto_unit((DVec){point.x-center.x,point.y-center.y,point.z-center.z},out);
}
static int pp_auto_surface(DVec guess,DVec *point,DVec *normal){
    int ok=1;DVec radial;if(!pp_auto_radial(guess,&radial))return 0;unsigned char on_structure=0;
    void *a[]={&radial,normal,&on_structure};PTR p=call(dmethod("MoonCleaners","MoonWorld","SurfaceAt","UnityEngine.Vector3;UnityEngine.Vector3&;System.Boolean&"),dget(bm.world),a,&ok);
    if(!ok||!p)return 0;dcopy(point,unbox(p),sizeof(*point));return 1;
}
static int pp_auto_stage(PTR walker,DVec point,int kind){
    int ok=1;void *definition_args[]={&kind};PTR definition=call(dmethod("MoonCleaners","MoonWorld","BuildDefinition","System.Int32"),dget(bm.world),definition_args,&ok);
    if(!ok||!definition)return 0;float half[2]={0},scale=0,reach=0;PTR klass=object_class(definition);
    PTR hf=get_field(klass,"halfFootprint"),sf=get_field(klass,"scale"),rf=get_field(klass,"reach");if(!hf||!sf||!rf)return 0;
    field_value(definition,hf,half);field_value(definition,sf,&scale);field_value(definition,rf,&reach);
    pp_auto_record("auto-native-build-spec",kind,bint(definition,"priceDollars"),(int)(reach*1000));
    float squared=half[0]*half[0]+half[1]*half[1];void *root_args[]={&squared};PTR root=call(dmethod("UnityEngine","Mathf","Sqrt","System.Single"),0,root_args,&ok);float radius=0;if(root)dcopy(&radius,unbox(root),4);
    float distance=radius*scale+1.25f;if(!ok||!(distance>1&&distance<reach+1.5f)){bevent("auto-fixture-stage-clearance-failed");return 0;}
    DVec guess={point.x-distance*pp_auto.right.x,point.y-distance*pp_auto.right.y,point.z-distance*pp_auto.right.z},at={0},up={0},zero={0};
    if(!pp_auto_surface(guess,&at,&up))return 0;at.x+=.3f*up.x;at.y+=.3f*up.y;at.z+=.3f*up.z;
    PTR body=bfield(walker,"body");void *a[]={&at},*b[]={&zero};
    call(dmethod("UnityEngine","Rigidbody","set_position","UnityEngine.Vector3"),body,a,&ok);
    call(dmethod("UnityEngine","Rigidbody","set_linearVelocity","UnityEngine.Vector3"),body,b,&ok);
    DVec facing={point.x-at.x,point.y-at.y,point.z-at.z};float dot=facing.x*up.x+facing.y*up.y+facing.z*up.z;
    facing=(DVec){facing.x-dot*up.x,facing.y-dot*up.y,facing.z-dot*up.z};
    if(!pp_auto_unit(facing,&facing)){bevent("auto-fixture-stage-facing-failed");return 0;}
    float pitch=0;void *place[]={&at,&facing,&pitch};
    call(dmethod("MoonCleaners","MoonWalker","Place","UnityEngine.Vector3;UnityEngine.Vector3;System.Single"),walker,place,&ok);
    DVec observed=dvector(dmethod("UnityEngine","Rigidbody","get_position",""),body,&ok);
    DVec delta={observed.x-at.x,observed.y-at.y,observed.z-at.z};float error=delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
    int placed=ok&&error>=0&&error<.0625f;pp_auto_record("auto-fixture-stage-readback",kind,error>=0&&error<100000?(int)(error*1000000):2147483647,placed);
    pp_auto.staged_at=at;pp_auto.settle_frame=integer(bm.frame,0,&ok)+2;
    if(!placed||!ok)bevent("auto-fixture-stage-failed");return placed&&ok;
}
static int pp_auto_begin_check(PTR walker,DVec point,int kind,int moving){
    if(!pp_auto_stage(walker,point,kind))return 0;pp_auto.moving=moving;if(!pp_auto_capture_ids(kind))return 0;
    DVec radial;if(!pp_auto_radial(point,&radial))return 0;
    unsigned xyz[3];dcopy(xyz,&point,sizeof(xyz));pp_auto_record("auto-fixture-position-bits",(int)xyz[0],(int)xyz[1],(int)xyz[2]);
    return moon_build_placement_begin(&pp_auto.placement,&pp_auto.api,dget(bm.world),kind,0,(MoonBuildPoint){radial.x,radial.y,radial.z},0);
}
/* Test-only bounded native preflight. Never move/erase existing structures or
 * weaken native collision, slope, unlock or payment admission to make a site fit. */
static int pp_auto_pick_site(PTR walker){
    const float sites[12][2]={{0,250},{100,250},{-100,250},{0,-250},{100,-250},{-100,-250},{250,0},{-250,0},{250,100},{-250,-100},{250,-100},{-250,100}};
    const float offsets[5][2]={{0,0},{42,0},{24,10},{14,0},{28,0}};
    int ok=1;float x=sites[pp_auto.site][0],y=sites[pp_auto.site][1];
    DVec guess={pp_auto.origin.x+x*pp_auto.initial_right.x+y*pp_auto.initial_forward.x,pp_auto.origin.y+x*pp_auto.initial_right.y+y*pp_auto.initial_forward.y,pp_auto.origin.z+x*pp_auto.initial_right.z+y*pp_auto.initial_forward.z},radial,origin,normal;
    if(!pp_auto_radial(guess,&radial))return 0;void *a[]={&radial};PTR gentle=call(dmethod("MoonCleaners","MoonWorld","GentleGround","UnityEngine.Vector3"),dget(bm.world),a,&ok);
    if(!ok||!gentle)return 0;dcopy(&radial,unbox(gentle),sizeof(radial));
    /* SurfaceAt accepts a direction, unlike pp_auto_surface's world-point input. */
    unsigned char on_structure=0;void *surface[]={&radial,&normal,&on_structure};PTR p=call(dmethod("MoonCleaners","MoonWorld","SurfaceAt","UnityEngine.Vector3;UnityEngine.Vector3&;System.Boolean&"),dget(bm.world),surface,&ok);
    if(!ok||!p)return 0;dcopy(&origin,unbox(p),sizeof(origin));pp_auto.up=radial;
    float dot=pp_auto.initial_right.x*radial.x+pp_auto.initial_right.y*radial.y+pp_auto.initial_right.z*radial.z;
    if(!pp_auto_unit((DVec){pp_auto.initial_right.x-dot*radial.x,pp_auto.initial_right.y-dot*radial.y,pp_auto.initial_right.z-dot*radial.z},&pp_auto.right))return 0;
    DVec forward={pp_auto.right.y*radial.z-pp_auto.right.z*radial.y,pp_auto.right.z*radial.x-pp_auto.right.x*radial.z,pp_auto.right.x*radial.y-pp_auto.right.y*radial.x};
    for(int i=0;i<5;i++){
        guess=(DVec){origin.x+offsets[i][0]*pp_auto.right.x+offsets[i][1]*forward.x,origin.y+offsets[i][0]*pp_auto.right.y+offsets[i][1]*forward.y,origin.z+offsets[i][0]*pp_auto.right.z+offsets[i][1]*forward.z};
        if(!pp_auto_surface(guess,pp_auto.points+i,&normal))return 0;
    }
    guess=(DVec){pp_auto.points[3].x+40*forward.x,pp_auto.points[3].y+40*forward.y,pp_auto.points[3].z+40*forward.z};
    if(!pp_auto_surface(guess,&pp_auto.away,&normal))return 0;
    pp_auto.index=0;pp_auto.stage=6;pp_auto_record("auto-fixture-site-preflight",pp_auto.site,0,1);
    return pp_auto_begin_check(walker,pp_auto.points[0],93,0);
}
static int pp_auto_contains(int wanted){
    int ok=1,found=0;DRoot records=dhold(call(dmethod("MoonCleaners","MoonWorld","get_BuiltDepots",""),dget(bm.world),0,&ok));DRoot it=benum(dget(records),&ok);PTR r;while((r=bnext(it,&ok)))if(bint(r,"id")==wanted)found=1;bend(&it);ddrop(&records);return ok?found:-1;
}
static void pp_auto_begin(PTR walker){
    moon_build_placement_cancel(&pp_auto.placement);pp_auto.stage=0;
    char resume[96];DWORD resumed=GetEnvironmentVariableA("MOONLAB_POWER_IDS",resume,sizeof(resume));
    if(resumed&&resumed<sizeof(resume)){
        const char *p=resume;int valid=1;for(int i=0;i<5;i++){U64 id=0;int digits=0;while(*p>='0'&&*p<='9'){id=id*10+(unsigned)(*p++-'0');if(++digits>10||id>2147483647u){valid=0;break;}}if(!valid||!digits||!id||(*p!=(i==4?0:','))){valid=0;break;}pp_auto.ids[i]=(int)id;if(i<4)p++;}
        if(valid){pp_auto.stage=2;bevent("auto-fixture-resumed");return;}bevent("auto-fixture-resume-ids-invalid");return;
    }
    if(!moon_build_placement_bind(&pp_auto.api)){bevent("auto-fixture-bind-failed");return;}
    int ok=1;PTR tr=call(bm.transform,walker,0,&ok);pp_auto.origin=dvector(bm.position,tr,&ok);
    pp_auto.initial_right=dvector(dmethod("UnityEngine","Transform","get_right",""),tr,&ok);
    pp_auto.initial_forward=dvector(bm.forward,tr,&ok);
    /* Choose naturally sunlit terrain without overriding the sun or energy.
     * The old prepared factory can be on the dark side of the Moon. */
    DVec sun=dvector(dmethod("MoonCleaners","MoonWorld","get_SunDirection",""),dget(bm.world),&ok),radial,normal;
    if(!ok||!pp_auto_unit(sun,&radial)){bevent("auto-fixture-sun-direction-failed");return;}
    PTR world_tr=call(bm.transform,dget(bm.world),0,&ok);DVec center=dvector(bm.position,world_tr,&ok);
    DVec lit={center.x+2000*radial.x,center.y+2000*radial.y,center.z+2000*radial.z};
    if(!ok||!pp_auto_surface(lit,&pp_auto.origin,&normal)){bevent("auto-fixture-origin-failed");return;}
    pp_auto.initial_forward=radial;
    /* Deterministic tangent axes keep the search away from the developed hub. */
    DVec axis=radial.y>-.8f&&radial.y<.8f?(DVec){0,1,0}:(DVec){1,0,0};
    if(!pp_auto_unit((DVec){axis.y*radial.z-axis.z*radial.y,axis.z*radial.x-axis.x*radial.z,axis.x*radial.y-axis.y*radial.x},&pp_auto.initial_right)){bevent("auto-fixture-sun-basis-failed");return;}
    pp_auto.initial_forward=(DVec){pp_auto.initial_right.y*radial.z-pp_auto.initial_right.z*radial.y,pp_auto.initial_right.z*radial.x-pp_auto.initial_right.x*radial.z,pp_auto.initial_right.x*radial.y-pp_auto.initial_right.y*radial.x};
    /* Disposable fixture budget and input stock use the game's developer APIs. */
    int budget=10000000;DRoot player=dhold(call(bm.player,0,0,&ok));void *conversion[]={&budget};
    DRoot amount=dhold(call(dmethod("System","Decimal","op_Implicit","System.Int32"),0,conversion,&ok));
    if(!ok||!amount){ddrop(&amount);ddrop(&player);bevent("auto-fixture-budget-conversion-failed");return;}
    void *money[]={unbox(dget(amount))};
    call(dmethod("MoonCleaners","MoonWorld","SetTeamDollars","System.Decimal"),dget(bm.world),money,&ok);
    void *wallet_args[]={dget(player)};DRoot wallet=dhold(call(bm.wallet,dget(bm.world),wallet_args,&ok));
    int balance=0;if(wallet){void *a[]={unbox(dget(wallet))};PTR number=call(bm.decimal_number,0,a,&ok);if(number)dcopy(&balance,unbox(number),sizeof(balance));}
    ddrop(&wallet);ddrop(&amount);ddrop(&player);
    pp_auto_record("auto-fixture-wallet",balance,budget,ok);
    if(!ok||balance<budget){bevent("auto-fixture-budget-failed");return;}
    pp_auto_record("auto-fixture-budget-granted",budget,balance,1);
    pp_auto.site=0;
    if(!pp_auto_pick_site(walker)){pp_auto.stage=0;bevent("auto-fixture-site-failed");return;}
    bevent("auto-fixture-started");
}
static int pp_auto_load(int id){
    int ok=1;PTR player=call(bm.player,0,0,&ok);double dust=10000;void *stock[]={player,&dust};
    call(dmethod("MoonCleaners","MoonWorld","DevSetCarriedDust","System.String;System.Double"),dget(bm.world),stock,&ok);
    PTR reason=0;void *a[]={player,&id,&reason};PTR value=call(dmethod("MoonCleaners","MoonWorld","LoadProcessor","System.String;System.Int32;System.String&"),dget(bm.world),a,&ok);
    double loaded=0;if(value)dcopy(&loaded,unbox(value),sizeof(loaded));
    pp_auto_record(ok&&loaded>0?"auto-fixture-native-input-loaded":"auto-fixture-native-input-failed",id,(int)(loaded*1000),ok);
    return ok&&loaded>0;
}
static void pp_auto_sample(void){
    int ok=1,mask=0,links=0;DRoot collection=dhold(call(dmethod("MoonCleaners","MoonWorld","get_PowerLinks",""),dget(bm.world),0,&ok));
    DRoot it=benum(dget(collection),&ok);PTR pair;
    const int ends[6][2]={{0,3},{3,4},{4,1},{3,2},{0,1},{0,2}};
    int external=0;
    while((pair=bnext(it,&ok))){
        if(++links>16384){ok=0;break;}int a=bint(pair,"Item1"),b=bint(pair,"Item2");
        int known_a=0,known_b=0;for(int i=0;i<5;i++)if(pp_auto.ids[i]){known_a|=a==pp_auto.ids[i];known_b|=b==pp_auto.ids[i];}
        if(known_a!=known_b)external++;
        for(int i=0;i<6;i++){int x=pp_auto.ids[ends[i][0]],y=pp_auto.ids[ends[i][1]];if(x&&y&&((a==x&&b==y)||(a==y&&b==x)))mask|=1<<i;}
    }
    bend(&it);ddrop(&collection);
    pp_auto_record("auto-power-links",mask,links,ok);
    int frame=integer(bm.frame,0,&ok);float clock=dnumber(dmethod("UnityEngine","Time","get_realtimeSinceStartup",""),0,&ok);
    pp_auto_record("auto-power-tick",frame,(int)(clock*1000),ok);
    pp_auto_record("auto-power-external",external,links,ok);
    int kinds[2]={93,193};for(int i=0;i<2;i++){unsigned char spec[32]={0};void *a[]={kinds+i,spec};PTR r=call(dmethod("MoonCleaners","MoonWorld","TryPanel","System.Int32;MoonCleaners.MoonWorld.PanelSpec&"),0,a,&ok);float peak=0,battery=0,range=0;dcopy(&peak,spec+8,4);dcopy(&battery,spec+12,4);dcopy(&range,spec+16,4);pp_auto_record("auto-native-spec",kinds[i],(int)(peak*1000),(int)(battery*1000));pp_auto_record("auto-native-range",kinds[i],(int)(range*1000),ok&&r&&*(unsigned char *)unbox(r));}
    for(int i=0;i<5;i++)if(pp_auto.ids[i]){
        int id=pp_auto.ids[i];void *a[]={&id};DRoot state=dhold(call(dmethod("MoonCleaners","MoonWorld","ProductionOf","System.Int32"),dget(bm.world),a,&ok));
        if(!state){pp_auto_record("auto-power-state-missing",id,i,0);continue;}
        float power=dnumber(dmethod("MoonCleaners","MoonProductionState","get_Power",""),dget(state),&ok);
        float demand=dnumber(dmethod("MoonCleaners","MoonProductionState","get_DemandKw",""),dget(state),&ok);
        float used=dnumber(dmethod("MoonCleaners","MoonProductionState","get_UsedKw",""),dget(state),&ok);
        int running=boolean(dmethod("MoonCleaners","MoonProductionState","get_Running",""),dget(state),&ok);
        float progress=dnumber(dmethod("MoonCleaners","MoonProductionState","get_Progress",""),dget(state),&ok);int cycles=integer(dmethod("MoonCleaners","MoonProductionState","get_Cycles",""),dget(state),&ok);
        char line[400],*p=cat(line,"{\"event\":\"auto-power-state\",\"id\":");p=num(p,id);p=cat(p,",\"index\":");p=num(p,i);p=cat(p,",\"power_milli\":");p=num(p,power>=0&&power<100000?(U64)(power*1000):0);p=cat(p,",\"demand_milli\":");p=num(p,demand>=0&&demand<100000?(U64)(demand*1000):0);p=cat(p,",\"used_milli\":");p=num(p,used>=0&&used<100000?(U64)(used*1000):0);p=cat(p,",\"progress_micro\":");p=num(p,progress>=0&&progress<1000?(U64)(progress*1000000):0);p=cat(p,",\"cycles\":");p=num(p,(unsigned)cycles);p=cat(p,",\"running\":");p=num(p,running);p=cat(p,",\"ok\":");p=num(p,ok);p=cat(p,"}\n");DWORD n=0;WriteFile(bm.log,line,(DWORD)(p-line),&n,0);FlushFileBuffers(bm.log);ddrop(&state);
        DVec anchor={0};void *q[]={&id,&anchor};PTR found=call(dmethod("MoonCleaners","MoonWorld","PowerAnchor","System.Int32;UnityEngine.Vector3&"),dget(bm.world),q,&ok);
        pp_auto_record("auto-native-anchor",id,found&&*(unsigned char *)unbox(found),ok);
    }
}
static void pp_auto_test_update(PTR walker){
    if(!bm.host_ready)return;unsigned now=0;for(int i=0;i<3;i++)if(bm.key(0x79+i)&0x8000)now|=1u<<i;
    unsigned edge=now&~pp_auto.previous;pp_auto.previous=now;
    if(edge&1)pp_auto_begin(walker);
    if(edge&2&&pp_auto.stage==2){
        DVec target=pp_auto.moving?pp_auto.points[3]:pp_auto.away;
        if(pp_auto_begin_check(walker,target,193,pp_auto.moving?2:1))pp_auto.stage=3;else bevent("auto-fixture-move-check-failed");
    }
    if(edge&4&&pp_auto.stage==4){pp_auto.index=3;if(pp_auto_begin_check(walker,pp_auto.points[3],193,0))pp_auto.stage=5;}
    else if(edge&4&&pp_auto.stage==2){
        int ok=1,id=pp_auto.ids[3];PTR reason=0;if(!pp_auto_stage(walker,pp_auto.points[3],193)){bevent("auto-fixture-remove-stage-failed");return;}void *a[]={walker,&id,&reason};
        PTR result=call(dmethod("MoonCleaners","MoonWorld","TryRemoveStructure","MoonCleaners.MoonWalker;System.Int32;System.String&"),dget(bm.world),a,&ok);
        int removed=ok&&result&&*(unsigned char *)unbox(result);pp_auto_record("auto-fixture-removed",id,removed,ok);
        if(removed){pp_auto_record("auto-fixture-record-removed",id,pp_auto_contains(id),1);pp_auto.ids[3]=0;pp_auto.stage=4;}
    }
    if(pp_auto.stage==1||pp_auto.stage==3||pp_auto.stage==5||pp_auto.stage==6){
        int ok=1;if(integer(bm.frame,0,&ok)<pp_auto.settle_frame)return;float dt=dnumber(bm.delta,0,&ok);int state=moon_build_placement_advance(&pp_auto.placement,dt,20,1);
        if(state<0){pp_auto_reason(dget(pp_auto.placement.check));
            if(pp_auto.stage==6){pp_auto_record("auto-fixture-site-refused",pp_auto.site,pp_auto.index,-state);moon_build_placement_cancel(&pp_auto.placement);
                if(++pp_auto.site<12&&pp_auto_pick_site(walker))return;bevent("auto-fixture-no-valid-site-failed");pp_auto.stage=0;return;}
            pp_auto_record("auto-fixture-check-rejected",pp_auto.index,-state,pp_auto.stage);moon_build_placement_cancel(&pp_auto.placement);pp_auto.stage=0;return;}
        if(state==MOON_BUILD_READY){
            if(pp_auto.stage==6){moon_build_placement_cancel(&pp_auto.placement);
                if(++pp_auto.index<5){if(!pp_auto_begin_check(walker,pp_auto.points[pp_auto.index],pp_auto_kinds[pp_auto.index],0)){bevent("auto-fixture-preflight-failed");pp_auto.stage=0;}return;}
                pp_auto_record("auto-fixture-site-valid",pp_auto.site,5,1);pp_auto.index=0;pp_auto.stage=1;
                if(!pp_auto_begin_check(walker,pp_auto.points[0],93,0)){bevent("auto-fixture-build-check-failed");pp_auto.stage=0;}return;}
            int accepted=0;
            if(pp_auto.stage==3){int id=pp_auto.ids[3];PTR reason=0;MoonBuildPlacement *p=&pp_auto.placement;void *a[]={walker,&id,&p->point,&p->yaw,dget(p->check),&reason};PTR r=call(dmethod("MoonCleaners","MoonWorld","TryMoveDepot","MoonCleaners.MoonWalker;System.Int32;UnityEngine.Vector3;System.Single;MoonCleaners.MoonWorld.DepotPlacementCheck;System.String&"),dget(bm.world),a,&ok);accepted=ok&&r&&*(unsigned char *)unbox(r);pp_auto_record("auto-fixture-moved",id,pp_auto.moving,accepted);if(accepted){pp_auto.moving=pp_auto.moving==1?1:0;pp_auto.stage=2;}}
            else{accepted=pp_auto_commit(walker);int id=accepted?pp_auto_new_id(pp_auto_kinds[pp_auto.index]):0;pp_auto_record("auto-fixture-identity",pp_auto.index,id,pp_auto.before_count);if(!accepted||!id)accepted=0;else{pp_auto.ids[pp_auto.index]=id;pp_auto_record("auto-fixture-built",pp_auto.index,id,pp_auto_kinds[pp_auto.index]);}}
            moon_build_placement_cancel(&pp_auto.placement);
            if(!accepted){bevent("auto-fixture-commit-failed");pp_auto.stage=0;return;}
            if(pp_auto.stage==5){pp_auto.stage=2;pp_auto_record("auto-fixture-replaced",3,pp_auto.ids[3],193);}
            else if(pp_auto.stage==1){if(++pp_auto.index<5){if(!pp_auto_begin_check(walker,pp_auto.points[pp_auto.index],pp_auto_kinds[pp_auto.index],0)){bevent("auto-fixture-check-failed");pp_auto.stage=0;}}
                else{pp_auto.stage=7;pp_auto.index=1;if(!pp_auto_stage(walker,pp_auto.points[1],96)){bevent("auto-fixture-input-stage-failed");pp_auto.stage=0;}}}
        }
    }
    if(pp_auto.stage==7){int ok=1;if(integer(bm.frame,0,&ok)>=pp_auto.settle_frame){
        if(!pp_auto_load(pp_auto.ids[pp_auto.index])){pp_auto.stage=0;return;}
        if(pp_auto.index==1){pp_auto.index=2;if(!pp_auto_stage(walker,pp_auto.points[2],96)){bevent("auto-fixture-input-stage-failed");pp_auto.stage=0;}}
        else{pp_auto.stage=2;bevent("auto-fixture-complete");}
    }}
    if(pp_auto.stage==2||pp_auto.stage==4){int ok=1;float now_time=dnumber(dmethod("UnityEngine","Time","get_realtimeSinceStartup",""),0,&ok);if(now_time>=pp_auto.next_sample){pp_auto.next_sample=now_time+.5f;pp_auto_sample();}}
}
static void pp_auto_reset(void){moon_build_placement_cancel(&pp_auto.placement);dzero(&pp_auto,sizeof(pp_auto));}
#endif
