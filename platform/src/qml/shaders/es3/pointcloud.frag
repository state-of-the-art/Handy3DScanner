#version 300 es

in highp vec4 color;
flat in highp int options;

out highp vec4 fragColor;

uniform highp float time;

void main()
{
    bool is_invalid = (options & 1 << 15) == 1 << 15;
    if( is_invalid )
        discard;

    bool is_selected = (options & 1) == 1;
    if( is_selected ) {
        lowp vec4 select_color = dot(color.rgb, vec3(0.299, 0.587, 0.114)) < 0.5 ? vec4(1.0,1.0,1.0,1.0) : vec4(0.0,0.0,0.0,1.0);
        fragColor = mix(select_color, color, sin(time*4.0));
    } else
        fragColor = color;
}
