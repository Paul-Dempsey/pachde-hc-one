#include "components.hpp"

namespace pachde {

// draw #d logo
void DrawLogo(NVGcontext * vg, float x, float y, NVGcolor fill, float scale)
{
    bool scaling = scale != 1.0f;
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    if (scaling) {
        nvgScale(vg, scale, scale);
    }
    nvgFillColor(vg, fill);

    // #
    nvgBeginPath(vg);
    nvgMoveTo(vg, 3.0225791, 5.3090902);
    nvgLineTo(vg, 3.9164512, 3.5427485);
    nvgLineTo(vg, 5.0031212, 3.8339209);
    nvgLineTo(vg, 4.1042182, 5.5989145);
    nvgLineTo(vg, 5.7593776, 6.0424131);
    nvgLineTo(vg, 6.6582806, 4.2774195);
    nvgLineTo(vg, 7.7399197, 4.5672439);
    nvgLineTo(vg, 6.8460476, 6.3335855);
    nvgLineTo(vg, 7.6308648, 6.5438766);
    nvgLineTo(vg, 7.3396924, 7.6305466);
    nvgLineTo(vg, 6.3284856, 7.3595946);
    nvgLineTo(vg, 5.6659891, 8.6649013);
    nvgLineTo(vg, 6.9689869, 9.0140385);
    nvgLineTo(vg, 6.6778145, 10.100708);
    nvgLineTo(vg, 5.1484271, 9.6909104);
    nvgLineTo(vg, 4.254555, 11.457252);
    nvgLineTo(vg, 3.1678851, 11.16608);
    nvgLineTo(vg, 4.0617572, 9.399738);
    nvgLineTo(vg, 2.4065978, 8.9562394);
    nvgLineTo(vg, 1.5127257, 10.722581);
    nvgLineTo(vg, 0.4260557, 10.431409);
    nvgLineTo(vg, 1.3199278, 8.665067);
    nvgLineTo(vg, 0.54014147, 8.4561239);
    nvgLineTo(vg, 0.83131382, 7.3694539);
    nvgLineTo(vg, 1.8425206, 7.640406);
    nvgLineTo(vg, 2.4999863, 6.3337512);
    nvgLineTo(vg, 1.2020194, 5.985962);
    nvgLineTo(vg, 1.4931917, 4.899292);
    nvgClosePath(vg);
    nvgPathWinding(vg, NVGsolidity::NVG_HOLE);
    nvgMoveTo(vg, 5.2418157, 7.0684222);
    nvgLineTo(vg, 3.5866563, 6.6249236);
    nvgLineTo(vg, 2.9241597, 7.9302303);
    nvgLineTo(vg, 4.5793191, 8.3737289);
    nvgClosePath(vg);
    nvgFill(vg);

    //d
    nvgBeginPath(vg);
    nvgMoveTo(vg, 13.386835,10.647075);
    nvgLineTo(vg, 13.086226,11.768961);
    nvgLineTo(vg, 10.731774,11.138087);
    nvgLineTo(vg, 10.886797,10.559536);

    nvgQuadTo(vg, 10.063919,11.034625, 9.3042558,10.831074);
    nvgQuadTo(vg, 8.3936666,10.587083, 7.9931509,9.6871288);
    nvgQuadTo(vg, 7.5926352,8.7871751, 7.8743714,7.7357213);
    nvgQuadTo(vg, 8.1601517,6.6691748, 8.9693819,6.1041558);
    nvgQuadTo(vg, 9.7799602,5.5341058, 10.665395,5.7713573);
    nvgQuadTo(vg, 11.515614,5.9991727, 11.89242,6.8065002);
    nvgLineTo(vg, 12.306262,5.2620202);
    nvgLineTo(vg, 11.546599,5.0584691);
    nvgLineTo(vg, 11.847208,3.936583);
    nvgLineTo(vg, 14.161412,4.5566722);
    nvgLineTo(vg, 12.586925,10.43274);
    nvgClosePath(vg);

    nvgPathWinding(vg, NVGsolidity::NVG_HOLE);
    nvgMoveTo(vg, 11.402005,8.6971241);
    nvgQuadTo(vg, 11.58938,7.9978318, 11.36706,7.6201287);
    nvgQuadTo(vg, 11.149771,7.2437736, 10.767424,7.1413241);
    nvgQuadTo(vg, 10.309614,7.0186543, 9.962361,7.3084457);
    nvgQuadTo(vg, 9.6214865,7.5945543, 9.4664642,8.1731054);
    nvgQuadTo(vg, 9.31953,8.7214713, 9.4649416,9.1648403);
    nvgQuadTo(vg, 9.612,9.603, 10.08,9.729);
    nvgQuadTo(vg, 10.467,9.833, 10.84,9.609);
    nvgQuadTo(vg, 11.219,9.382, 11.402,8.697);
    nvgClosePath(vg);
    nvgFill(vg);

    nvgRestore(vg);
}

void heart_path(NVGcontext* vg)
{
    nvgBeginPath(vg);
    nvgMoveTo(vg, 50.f, 14.5f);
    nvgQuadTo(vg, 51.88014,11.675824, 54.28907,9.0659343);
    nvgQuadTo(vg, 56.698,6.456044, 59.518215,4.4299451 );
    nvgQuadTo(vg, 62.338425,2.4038461, 65.481785,1.2019231 );
    nvgQuadTo(vg, 68.625145,0, 71.97415,0 );
    nvgQuadTo(vg, 77.790836,0, 82.873093,2.5755494 );
    nvgQuadTo(vg, 87.955343,5.151099, 91.774386,9.6153843 );
    nvgQuadTo(vg, 95.593421,14.079671, 97.796707,20.020604 );
    nvgQuadTo(vg, 100,25.961539, 100,32.760989 );
    nvgQuadTo(vg, 100,35.302198, 99.383071,39.010989);
    nvgQuadTo(vg, 98.766143,42.71978, 96.915386,47.458791 );
    nvgQuadTo(vg, 95.064621,52.1978, 91.656871,57.967035 );
    nvgQuadTo(vg, 88.249114,63.736265, 82.667443,70.32967 );
    nvgQuadTo(vg, 77.085779,76.923071, 69.00705,84.375 );
    nvgQuadTo(vg, 60.92832,91.826921, 49.706228,100 );
    nvgQuadTo(vg, 38.366627,91.620879, 30.287896,84.203293 );
    nvgQuadTo(vg, 22.209166,76.785714, 16.686251,70.26099 );
    nvgQuadTo(vg, 11.163337,63.736265, 7.8437136,58.138735 );
    nvgQuadTo(vg, 4.5240893,52.54121, 2.7908343,47.870879 );
    nvgQuadTo(vg, 1.0575793,43.200549, 0.52878964,39.423077 );
    nvgQuadTo(vg, 0,35.645604, 0,32.760989 );
    nvgQuadTo(vg, 0,25.961539, 2.2326674,20.020604 );
    nvgQuadTo(vg, 4.4653349,14.079671, 8.2843714,9.6153843 );
    nvgQuadTo(vg, 12.103408,5.151099, 17.244419,2.5755494 );
    nvgQuadTo(vg, 22.385429,0, 28.202115,0);
    nvgQuadTo(vg, 31.492362,0, 34.606346,1.2019231 );
    nvgQuadTo(vg, 37.720329,2.4038461, 40.511164,4.4299451 );
    nvgQuadTo(vg, 43.301997,6.456044, 45.710929,9.0659343 );
    nvgQuadTo(vg, 48.119859,11.675824, 50,14.491758);
    nvgClosePath(vg);
}

void FillHeart(NVGcontext* vg, float x, float y, float square, NVGcolor fill)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgFillColor(vg, fill);
    heart_path(vg);
    nvgFill(vg);
    nvgRestore(vg);
}

void StrokeHeart(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width)
{
    nvgSave(vg);
    nvgTranslate(vg, x, y);
    float scale = square / 100.f;
    nvgScale(vg, scale, scale);
    nvgStrokeColor(vg, stroke);
    nvgStrokeWidth(vg, stroke_width * 10.f);
    heart_path(vg);
    nvgStroke(vg);
    nvgRestore(vg);
}

}