#version 300 es

in highp vec3 vertexPosition;
in highp vec4 vertexColor;
in highp int vertexOptions;

out highp vec4 color;
flat out highp int options;

uniform highp mat4 mvp;
uniform lowp float pointSize;

void main()
{
    color = vertexColor;
    gl_Position = mvp * vec4( vertexPosition, 1.0 );
    gl_PointSize = 1.0 + pointSize*0.5 / gl_Position.w;

    // Passing options
    options = vertexOptions;
    // Discard if point is invalid (z position is 0)
    //options |= (vertexPosition.z == 0.0 ? 1 << 15 : 0);
}
