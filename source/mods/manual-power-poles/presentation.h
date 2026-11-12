#ifndef MOON_POLE_PRESENTATION_H
#define MOON_POLE_PRESENTATION_H
/* A quiet painted/ceramic finish. Keep the supplied maps and diffuse colour;
 * the multiplier also bounds the scalar fallback in a stripped player shader. */
#define PP_SMOOTHNESS .45f
#define PP_NORMAL_STRENGTH .55f
#define PP_FALLBACK_METALLIC .03f
static int pp_visual_baseline;
static int pp_material_finish(int baseline){
    if(!pp_resources.ready)return 0;int ok=1;
    for(unsigned i=0;i<pp_resources.count;i++){
        PTR material=su_get(pp_resources.materials[i]);if(!material)return 0;
        if(!sm_property(material,"_Metallic",baseline?.5f:PP_FALLBACK_METALLIC)||
           !sm_property(material,"_Smoothness",baseline?1:PP_SMOOTHNESS)||
           !sm_property(material,"_GlossMapScale",baseline?1:PP_SMOOTHNESS)||
           !sm_property(material,"_Glossiness",baseline?.3f:PP_SMOOTHNESS)||
           !sm_property(material,"_BumpScale",baseline?1:PP_NORMAL_STRENGTH)||
           !sm_colour(material,"_EmissionColor",(SUColor){0,0,0,1}))return 0;
        void *keyword[]={su.string("_EMISSION")};
        call(su_method("UnityEngine","Material","DisableKeyword","System.String"),material,keyword,&ok);
    }
    return ok;
}
#endif
