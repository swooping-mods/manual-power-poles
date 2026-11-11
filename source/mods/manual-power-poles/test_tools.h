/* Main-thread staging only in the disposable test harness. Native placement,
 * footprint checks, prices and structure creation are still the game path. */
static void pp_test_controls(PTR walker){
    static unsigned previous;unsigned now=0;const int keys[]={0x74,0x75,0x76,0x77,0x78};
    for(int i=0;i<5;i++)if(bm.key(keys[i])&0x8000)now|=1u<<i;
    unsigned edge=now&~previous;previous=now;if(!edge||!bm.host_ready)return;int ok=1;
    void *type[]={dtype("MoonCleaners","MoonDepotBuilder")};PTR builder=call(dmethod("UnityEngine","Component","GetComponent","System.Type"),walker,type,&ok);if(!builder||!ok)return;
    if(edge&16)pp_test_aim_cycle(walker);
    if(edge&1){unsigned char yes=1;void *a[]={&yes};call(dmethod("MoonCleaners","MoonDepotBuilder","BeginPlacement",""),builder,0,&ok);call(dmethod("MoonCleaners","MoonDepotBuilder","SetOpen","System.Boolean"),builder,a,&ok);bevent(ok?"test-build-opened":"test-build-failed");}
    if(edge&2)bevent(build_menu->begin_placement(walker,PP_KIND)?"test-pole-placement-started":"test-pole-placement-failed");
    if(edge&4){
        PTR tr=call(dmethod("UnityEngine","Component","get_transform",""),walker,0,&ok),body=bfield(walker,"body");DVec p=dvector(dmethod("UnityEngine","Transform","get_position",""),tr,&ok),up=dvector(dmethod("UnityEngine","Transform","get_up",""),tr,&ok),forward=dvector(dmethod("UnityEngine","Transform","get_forward",""),tr,&ok),right=dvector(dmethod("UnityEngine","Transform","get_right",""),tr,&ok);
        const float directions[8][2]={{1,0},{0,1},{-1,0},{0,-1},{.707f,.707f},{-.707f,.707f},{.707f,-.707f},{-.707f,-.707f}};
        for(int i=0;i<8;i++){
            DVec origin={p.x+35*(forward.x*directions[i][0]+right.x*directions[i][1])+25*up.x,p.y+35*(forward.y*directions[i][0]+right.y*directions[i][1])+25*up.y,p.z+35*(forward.z*directions[i][0]+right.z*directions[i][1])+25*up.z},down={-up.x,-up.y,-up.z};float distance=60;int mask=~(1<<2),triggers=1;union{U64 align;unsigned char data[96];}hit;dzero(&hit,sizeof(hit));void *a[]={&origin,&down,hit.data,&distance,&mask,&triggers};PTR found=call(dmethod("UnityEngine","Physics","Raycast","UnityEngine.Vector3;UnityEngine.Vector3;UnityEngine.RaycastHit&;System.Single;System.Int32;UnityEngine.QueryTriggerInteraction"),0,a,&ok);if(!ok||!found||!*(unsigned char *)unbox(found))continue;
            int (*offset)(PTR)=(int(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_field_get_offset");PTR klass=find_class("UnityEngine","RaycastHit"),pf=get_field(klass,"m_Point"),nf=get_field(klass,"m_Normal");if(!offset||!pf||!nf||offset(pf)!=16||offset(nf)!=28)return;DVec point,normal;dcopy(&point,hit.data,12);dcopy(&normal,hit.data+12,12);if(normal.x*up.x+normal.y*up.y+normal.z*up.z<.8f)continue;
            point.x+=up.x*.2f;point.y+=up.y*.2f;point.z+=up.z*.2f;void *to[]={&point};call(dmethod("UnityEngine","Rigidbody","set_position","UnityEngine.Vector3"),body,to,&ok);DVec zero={0};void *stop[]={&zero};call(dmethod("UnityEngine","Rigidbody","set_linearVelocity","UnityEngine.Vector3"),body,stop,&ok);bevent(ok?"test-outdoor-staged":"test-stage-failed");break;
        }
    }
    if(edge&8){PTR tr=call(dmethod("UnityEngine","Component","get_transform",""),walker,0,&ok);DVec up=dvector(dmethod("UnityEngine","Transform","get_up",""),tr,&ok),forward=dvector(dmethod("UnityEngine","Transform","get_forward",""),tr,&ok);DVec dir={forward.x-.7f*up.x,forward.y-.7f*up.y,forward.z-.7f*up.z};void *a[]={&dir};call(dmethod("MoonCleaners","MoonWalker","TestLook","UnityEngine.Vector3"),walker,a,&ok);bevent(ok?"test-camera-pitched":"test-camera-failed");}
}
