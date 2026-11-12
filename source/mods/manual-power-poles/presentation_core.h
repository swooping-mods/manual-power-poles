#ifndef MOON_POLE_PRESENTATION_CORE_H
#define MOON_POLE_PRESENTATION_CORE_H
/* These offsets are admitted against live metadata before the native callback
 * is installed. Reading the captured shared mesh avoids world/record scans. */
static void pp_visual_copy(void *to,const void *from,unsigned size){
    unsigned char *d=to;const unsigned char *s=from;for(unsigned i=0;i<size;i++)d[i]=s[i];
}
static int pp_visual_int(const unsigned char *p){int value;pp_visual_copy(&value,p,4);return value;}
static const unsigned char *pp_visual_pointer(const unsigned char *p){const unsigned char *value;pp_visual_copy(&value,p,sizeof(value));return value;}
static int pp_visual_owns_idle_mesh(const unsigned char *builder,const void *mesh){
    if(!builder||!mesh||!builder[262]||builder[416]||builder[1008]||pp_visual_int(builder+264))return 0;
    const unsigned char *group=pp_visual_pointer(builder+272);
    if(group&&pp_visual_int(group+24)!=0)return 0;
    const unsigned char *parts=pp_visual_pointer(builder+304);
    if(!parts||pp_visual_int(parts+24)!=1)return 0;
    const unsigned char *items=pp_visual_pointer(parts+16);if(!items)return 0;
    unsigned long long length=0;pp_visual_copy(&length,items+24,8);
    if(length<1)return 0;
    return pp_visual_pointer(items+32)==mesh;
}
#endif
