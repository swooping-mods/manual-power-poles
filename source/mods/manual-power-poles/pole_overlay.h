#ifndef MOON_POLE_OVERLAY_H
#define MOON_POLE_OVERLAY_H
#include "presentation_core.h"
static int pp_overlay_bound;
static unsigned pp_overlay_calls,pp_overlay_suppressed,pp_overlay_preview_passes;
static int pp_overlay_bind(PTR game){
    int (*offset)(PTR)=(int(*)(PTR))GetProcAddress(game,"il2cpp_field_get_offset");
    PTR (*field_type)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_field_get_type");
    PTR (*type_class)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_class_from_type");
    int (*type_kind)(PTR)=(int(*)(PTR))GetProcAddress(game,"il2cpp_type_get_type");
    int (*element_size)(PTR)=(int(*)(PTR))GetProcAddress(game,"il2cpp_array_element_size");
    PTR (*element_class)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_class_get_element_class");
    if(!offset||!field_type||!type_class||!type_kind||!element_size||!element_class)return 0;
    PTR klass=find_class("MoonCleaners","MoonDepotBuilder");if(!klass)return 0;
    const struct {const char *name;int offset,kind;} fields[]={
        {"moveMode",262,2},{"carried",264,8},{"carriedSet",272,21},
        {"moveParts",304,21},{"removing",416,2},{"placing",1008,2}};
    for(unsigned i=0;i<sizeof(fields)/sizeof(fields[0]);i++){
        PTR f=get_field(klass,fields[i].name);
        if(!f||offset(f)!=fields[i].offset||type_kind(field_type(f))!=fields[i].kind)return 0;
    }
    const char *lists[]={"carriedSet","moveParts"};
    for(unsigned i=0;i<2;i++){
        PTR list=type_class(field_type(get_field(klass,lists[i])));
        PTR size=list?get_field(list,"_size"):0,items=list?get_field(list,"_items"):0;
        if(!size||!items||offset(size)!=24||offset(items)!=16||type_kind(field_type(size))!=8)return 0;
        PTR array=type_class(field_type(items));if(!array||element_size(array)!=(i?24:4))return 0;
        if(i){
            PTR item=element_class(array),first=item?get_field(item,"Item1"):0;
            if(!first||offset(first)!=16||type_class(field_type(first))!=find_class("UnityEngine","Mesh"))return 0;
        }
    }
    PTR method_id=dmethod("MoonCleaners","MoonDepotBuilder","DrawMoveParts","");
    if(!method_id||method_token(method_id)!=100663824||*(PTR *)method_id!=(unsigned char *)game+0x438770)return 0;
    static const unsigned char guard[]={0x48,0x8b,0xc4,0x48,0x89,0x58,0x10,0x48,0x89,0x70,0x18,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x81,0xec,0x00,0x04,0x00,0x00,0x0f,0x29,0x70,0xc8,0x0f};
    const unsigned char *code=*(PTR *)method_id;
    for(unsigned i=0;i<sizeof(guard);i++)if(code[i]!=guard[i])return 0;
    if(!prepare_hook(method_id,100663824,0,game))return 0;
    pp_overlay_bound=1;return 1;
}
static int pp_overlay_before(Context *context){
    if(!pp_overlay_bound||!pp_resources.ready||!bm.thread||bm.thread!=GetCurrentThreadId())return 0;
    pp_overlay_calls++;
    if(pp_visual_baseline)return 0;
    PTR mesh=su_get(pp_resources.meshes[0]);
    int suppress=pp_visual_owns_idle_mesh((const unsigned char *)context->rcx,mesh);
    if(suppress)pp_overlay_suppressed++;
    else if(context->rcx){
        const unsigned char *builder=(const unsigned char *)context->rcx;
        if(builder[262]||builder[1008]||pp_visual_int(builder+264))pp_overlay_preview_passes++;
    }
    /* Skip only the native idle redraw of this exact owned shared mesh. The
     * real opaque renderer, power UI, placement, carry and removal stay native. */
    return suppress;
}
#endif
