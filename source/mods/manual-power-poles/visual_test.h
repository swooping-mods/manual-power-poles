#ifndef MOON_POLE_VISUAL_TEST_H
#define MOON_POLE_VISUAL_TEST_H
/* Explicit private-fixture controls only. They move the real walker/camera;
 * native rays must still select the pole and native previews remain genuine. */
static struct {DRoot builder;unsigned keys;int pose,aim_after,shader_logged;float next;DVec target;} pp_visual_test;
static void pp_visual_test_reset(void){ddrop(&pp_visual_test.builder);dzero(&pp_visual_test,sizeof(pp_visual_test));pp_visual_baseline=0;if(pp_resources.ready)pp_material_finish(0);}
static int pp_visual_stage(PTR walker){
    if(pp_auto.stage!=2||!pp_auto.ids[3])return 0;int pose=pp_visual_test.pose++%3,ok=1;
    DVec point=pp_auto.points[3],right=pp_auto.right,normal={0},at={0},zero={0};
    float distance=pose==1?2.2f:3.8f;if(pose==2){right.x=-right.x;right.y=-right.y;right.z=-right.z;}
    DVec guess={point.x-distance*right.x,point.y-distance*right.y,point.z-distance*right.z};
    if(!pp_auto_surface(guess,&at,&normal))return 0;
    at.x+=.3f*normal.x;at.y+=.3f*normal.y;at.z+=.3f*normal.z;
    PTR body=bfield(walker,"body");void *position[]={&at},*stop[]={&zero};
    call(dmethod("UnityEngine","Rigidbody","set_position","UnityEngine.Vector3"),body,position,&ok);
    call(dmethod("UnityEngine","Rigidbody","set_linearVelocity","UnityEngine.Vector3"),body,stop,&ok);
    DVec facing={right.x,right.y,right.z};float pitch=0;void *place[]={&at,&facing,&pitch};
    call(dmethod("MoonCleaners","MoonWalker","Place","UnityEngine.Vector3;UnityEngine.Vector3;System.Single"),walker,place,&ok);
    DVec actual=dvector(dmethod("UnityEngine","Rigidbody","get_position",""),body,&ok);
    DVec delta={actual.x-at.x,actual.y-at.y,actual.z-at.z};
    if(!ok||delta.x*delta.x+delta.y*delta.y+delta.z*delta.z>.0625f)return 0;
    float height=pose==1?.5f:1.1f;pp_visual_test.target=(DVec){point.x+height*pp_auto.up.x,point.y+height*pp_auto.up.y,point.z+height*pp_auto.up.z};
    pp_visual_test.aim_after=bm.last_frame+4;pp_auto_record("visual-camera-staged",pp_auto.ids[3],pose,1);return 1;
}
static void pp_visual_test_update(PTR walker){
    if(!bm.host_ready||!pp_resources.ready)return;int ok=1;
    if(!pp_visual_test.builder){void *type[]={dtype("MoonCleaners","MoonDepotBuilder")};pp_visual_test.builder=dhold(call(dmethod("UnityEngine","Component","GetComponent","System.Type"),walker,type,&ok));}
    if(!ok||!pp_visual_test.builder)return;
    unsigned keys=0;if(bm.key(0x72)&0x8000)keys|=1;if(bm.key(0x71)&0x8000)keys|=2;
    unsigned edge=keys&~pp_visual_test.keys;pp_visual_test.keys=keys;
    if(edge&1){int next=!pp_visual_baseline,finished=pp_material_finish(next);if(finished)pp_visual_baseline=next;pp_auto_record("visual-baseline",next,finished,1);}
    if(edge&2&&!pp_visual_stage(walker))bevent("visual-camera-failed");
    if(pp_visual_test.aim_after&&bm.last_frame>=pp_visual_test.aim_after){
        DVec origin,direction;if(!baim(&origin,&direction)){bevent("visual-aim-failed");return;}
        direction=(DVec){pp_visual_test.target.x-origin.x,pp_visual_test.target.y-origin.y,pp_visual_test.target.z-origin.z};
        void *look[]={&direction};call(dmethod("MoonCleaners","MoonWalker","TestLook","UnityEngine.Vector3"),walker,look,&ok);
        pp_visual_test.aim_after=0;pp_auto_record("visual-camera-ready",pp_auto.ids[3],bm.last_frame,ok);
    }
    float now=dnumber(dmethod("UnityEngine","Time","get_realtimeSinceStartup",""),0,&ok);
    if(now<pp_visual_test.next)return;pp_visual_test.next=now+.3f;
    PTR builder=dget(pp_visual_test.builder),material=su_get(pp_resources.materials[0]);
    if(!pp_visual_test.shader_logged){
        PTR shader=call(dmethod("UnityEngine","Material","get_shader",""),material,0,&ok);
        DRoot name=dhold(call(dmethod("UnityEngine","Object","get_name",""),shader,0,&ok));
        int (*length)(PTR)=(int(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_length");
        const unsigned short *(*chars)(PTR)=(const unsigned short *(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_chars");
        if(ok&&name&&length&&chars){char line[384],*p=cat(line,"{\"event\":\"visual-shader\",\"name\":\"");int n=length(dget(name));const unsigned short *s=chars(dget(name));
            for(int i=0;i<n&&i<160;i++)if(s[i]>=32&&s[i]<127&&s[i]!='"'&&s[i]!='\\')*p++=(char)s[i];
            p=cat(p,"\"}\n");DWORD written=0;WriteFile(bm.log,line,(DWORD)(p-line),&written,0);FlushFileBuffers(bm.log);pp_visual_test.shader_logged=1;}
        ddrop(&name);
    }
    const char *properties[]={"_Smoothness","_BumpScale","_Metallic"};int values[3]={0};
    for(unsigned i=0;i<3;i++){void *a[]={su.string(properties[i])};PTR v=call(su_method("UnityEngine","Material","GetFloat","System.String"),material,a,&ok);float n=0;if(v)dcopy(&n,unbox(v),4);values[i]=(int)(n*1000+.5f);}
    int ghost_renderers=0;PTR ghost=bfield(builder,"ghost");
    /* The release player strips the non-generic convenience overload. Query
     * its retained native implementation, excluding inactive ghost children. */
    if(ghost){unsigned char no=0,yes=1;void *a[]={dtype("UnityEngine","MeshRenderer"),&no,&yes,&no,&no,0};DRoot renderers=dhold(call(dmethod("UnityEngine","GameObject","GetComponentsInternal","System.Type;System.Boolean;System.Boolean;System.Boolean;System.Boolean;System.Object"),ghost,a,&ok));
        U64 count=0;if(renderers)dcopy(&count,(unsigned char *)dget(renderers)+24,8);
        if(count>32)ok=0;else for(U64 i=0;i<count;i++){PTR renderer=*(PTR *)darray(dget(renderers),(int)i);if(renderer&&boolean(dmethod("UnityEngine","Renderer","get_enabled",""),renderer,&ok))ghost_renderers++;}ddrop(&renderers);}
    char line[640],*p=cat(line,"{\"event\":\"visual-state\",\"baseline\":");p=num(p,pp_visual_baseline);
#define PPV(K,V) p=cat(p,",\"" K "\":");p=num(p,(unsigned)(V))
    PPV("target",bint(builder,"moveTarget"));PPV("calls",pp_overlay_calls);PPV("suppressed",pp_overlay_suppressed);PPV("preview_passes",pp_overlay_preview_passes);
    PPV("placing",boolean(dmethod("MoonCleaners","MoonDepotBuilder","get_IsPlacing",""),builder,&ok));
    PPV("moving",boolean(dmethod("MoonCleaners","MoonDepotBuilder","get_IsMoving",""),builder,&ok));
    PPV("removing",boolean(dmethod("MoonCleaners","MoonDepotBuilder","get_IsRemoving",""),builder,&ok));
    PPV("carried",bint(builder,"carried"));PPV("ghost_renderers",ghost_renderers);PPV("first_person",boolean(bm.first,walker,&ok));
    PPV("smoothness_milli",values[0]);PPV("normal_milli",values[1]);PPV("metallic_milli",values[2]);PPV("ok",ok);
#undef PPV
    p=cat(p,"}\n");DWORD written=0;WriteFile(bm.log,line,(DWORD)(p-line),&written,0);FlushFileBuffers(bm.log);
}
#endif
