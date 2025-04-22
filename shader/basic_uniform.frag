#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;


layout (binding = 0) uniform sampler2D Tex1;
layout (binding = 1) uniform sampler2D Tex2;
layout (binding = 2) uniform sampler2D Texture2;

layout (location = 0) out vec4 FragColour;

uniform int pass;
uniform float weight[5];

uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
    vec3 Direction;
    float Exponent;
    float Cutoff;
}Light;

uniform struct MaterialInfo {
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

const int levels = 3;
const float scaleFactor = 1.0 / levels;

//uniform struct FogInfo{
//    float MaxDist;
//    float MinDist;
//    vec3 Colour;
//}Fog;


vec3 Blinnphong( vec3 position, vec3 n) {
    vec3 diffuse=vec3(0), spec=vec3(0);

    vec4 pengTexColour = texture(Tex1, TexCoord);
    vec4 dirtTexColour = texture(Tex2, TexCoord);
    vec3 texColour = mix(pengTexColour.rgb, dirtTexColour.rgb, dirtTexColour.a);
    vec3 ambient = Light.La * texColour;

    vec3 s=normalize(Light.Position.xyz-position);

    float cosAng = dot(-s, normalize(Light.Direction));
    float angle = acos(cosAng);
    float spotScale;

    if (angle >= 0.0 && angle < Light.Cutoff) {
        spotScale = pow(cosAng, Light.Exponent);
        float sDotN=max(dot(s,n),0.0);
        diffuse = texColour * floor(sDotN * levels) * scaleFactor;

        if (sDotN > 0.00){
            vec3 v = normalize(-position.xyz);
            vec3 h = normalize(v + s);
            spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
        }
    }

   
    return ambient + spotScale * (diffuse+spec) * Light.L;
}


vec4 pass1() {
    return vec4(Blinnphong(Position, normalize(Normal)), 1.0);
}

vec4 pass2() {
    ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(Texture2, pix, 0) * weight[0];

    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, 1)) * weight[1];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, -1)) * weight[1];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, 2)) * weight[2];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, -2)) * weight[2];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, 3)) * weight[3];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, -3)) * weight[3];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, 4)) * weight[4];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(0, -4)) * weight[4];

    return sum;
}

vec4 pass3() {
    ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(Texture2, pix, 0) * weight[0];

    sum += texelFetchOffset(Texture2, pix, 0, ivec2(1, 0)) * weight[1];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(-1, 0)) * weight[1];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(2, 0)) * weight[2];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(-2, 0)) * weight[2];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(3, 0)) * weight[3];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(-3, 0)) * weight[3];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(4, 0)) * weight[4];
    sum += texelFetchOffset(Texture2, pix, 0, ivec2(-4, 0)) * weight[4];

    return sum;
}



void main() {
    if (pass == 1) FragColour = pass1();
    else if (pass == 2) FragColour = pass2();
    else if (pass == 3) FragColour = pass3();
    

    //float dist = abs(Position.z);
    //float fogFactor = (Fog.MaxDist-dist)/(Fog.MaxDist-Fog.MinDist);
    //fogFactor = clamp(fogFactor, 0.0, 1.0);
    //vec3 shaderColour = Blinnphong(Position, normalize(Normal));
    //vec3 colour = mix(Fog.Colour, FragColour, fogFactor);
}