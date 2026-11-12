/* The private supplied model, PBR resources and its world-local factory. */
#ifndef MANUAL_POWER_POLE_MODEL_H
#define MANUAL_POWER_POLE_MODEL_H
#include "../../runtime/sdk/scene/static_model_pbr.h"
#include "assets/generated/model_data.h"
static DRoot pp_template;
static SMResources pp_resources;
#include "presentation.h"
static PTR pp_field(PTR obj,const char *name){return bfield(obj,name);}
static int pp_ref(PTR obj,const char *name,PTR value){
    PTR f=get_field(object_class(obj),name);if(!f)return 0;PTR game=GetModuleHandleA("GameAssembly.dll");
    int (*offset)(PTR)=(int(*)(PTR))GetProcAddress(game,"il2cpp_field_get_offset");
    void (*barrier)(PTR,PTR *,PTR)=(void(*)(PTR,PTR *,PTR))GetProcAddress(game,"il2cpp_gc_wbarrier_set_field");
    int n=offset?offset(f):0;if(n<16||!barrier)return 0;barrier(obj,(PTR *)((unsigned char *)obj+n),value);return 1;
}
static int pp_set(PTR obj,const char *name,void *value){PTR f=get_field(object_class(obj),name);if(!f)return 0;bm.set_field(obj,f,value);return 1;}
static PTR pp_transform(PTR go,int *ok){return call(dmethod("UnityEngine","GameObject","get_transform",""),go,0,ok);}
static void pp_release(void *definition,void *world){
    (void)definition;(void)world;su_destroy(dget(pp_template));ddrop(&pp_template);sm_release(&pp_resources);
}
static int pp_configure(void *definition,void *world){
    (void)world;if(!pp_field(definition,"prefab")||!su_bind(GetModuleHandleA("GameAssembly.dll")))return 0;
    if(!sm_prepare_pbr(&pp_model,&pp_resources,pp_pbr,1))return 0;
    if(!pp_material_finish(0)){sm_release(&pp_resources);return 0;}
    PTR go=sm_attach(&pp_model,&pp_resources,0,0,"MoonLab PBR Power Pole template");
    pp_template=dhold(go);if(!pp_template){su_destroy(go);sm_release(&pp_resources);return 0;}
    int ok=1;PTR parent=pp_transform(go,&ok);if(!ok||!parent)goto fail;
    /* Preserve solid placement/player contact without triangle mesh collision. */
    for(unsigned i=0;i<sizeof(pp_boxes)/sizeof(pp_boxes[0]);i++){
        PTR box=su_add_component(go,"BoxCollider");if(!box)goto fail;
        void *center[]={(void *)pp_boxes[i].center},*size[]={(void *)pp_boxes[i].size};
        call(su_method("UnityEngine","BoxCollider","set_center","UnityEngine.Vector3"),box,center,&ok);
        call(su_method("UnityEngine","BoxCollider","set_size","UnityEngine.Vector3"),box,size,&ok);
        if(!ok)goto fail;
    }
    /* Native placement assigns each clone its actual spherical-world pose. */
    DVec away={0,-10000,0};void *position[]={&away};
    call(dmethod("UnityEngine","Transform","set_position","UnityEngine.Vector3"),parent,position,&ok);
    float scale=1,height=PP_MODEL_HEIGHT,offset=0;
    float footprint[2]={PP_MODEL_HALF_WIDTH,PP_MODEL_HALF_DEPTH};
    if(ok&&pp_set(definition,"scale",&scale)&&pp_set(definition,"height",&height)&&
       pp_set(definition,"floorOffset",&offset)&&pp_set(definition,"halfFootprint",footprint)&&pp_ref(definition,"prefab",go))return 1;
fail:
    pp_release(definition,world);return 0;
}
#endif
