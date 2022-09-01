static constexpr const char* thick_line_shader_frag_source = R"SHADER(

#version 400 core

out vec4 color;
in UvData {
	vec2 uv;
} geomOut;

uniform mat4 uMVP;
uniform vec4 uColor;

void main()
{
	if (dot(geomOut.uv, geomOut.uv) <= 1.0)
		color = uColor;
	else
		color = vec4(0, 0, 0, 0);
}

)SHADER";

static constexpr const char* thick_line_shader_vert_source = R"SHADER(
    
#version 400 core

layout(location = 0) in vec4 vertInPosition;
layout(location = 1) in float vertInRadius;

uniform mat4 uMVP;
uniform vec4 uColor;

out VertexData 
{
	vec4 position;
	float radius;
} vertOut;

void main()
{
	gl_Position = uMVP * vertInPosition;
	vertOut.position = vertInPosition;
	vertOut.radius = vertInRadius;
}

)SHADER";

static constexpr const char* thick_line_shader_geom_source = R"SHADER(

#version 400 core
layout(lines) in;
layout(triangle_strip, max_vertices = 8) out;

uniform mat4 uMVP;
uniform vec4 uColor;

in VertexData 
{
	vec4 position;
	float radius;
} vertOut[];

out UvData {
	vec2 uv;
} geomOut;

void main() {

    vec4 dir = normalize(vertOut[1].position - vertOut[0].position);
    float r1 = vertOut[0].radius;
    float r2 = vertOut[1].radius;

    vec4 a = vertOut[0].position + vec4(-dir.y, dir.x, 0, 0) * r1;
    vec4 b = a + (vertOut[0].position - a) * 2.0;

	vec4 p0 = uMVP * (b - dir * r1);
	vec4 p1 = uMVP * (a - dir * r1);
    vec4 p2 = uMVP * b;
    vec4 p3 = uMVP * a;
    
    a = vertOut[1].position + vec4(-dir.y, dir.x, 0, 0) * r2;
    b = a + (vertOut[1].position - a) * 2.0;
    
    vec4 p4 = uMVP * b;
    vec4 p5 = uMVP * a;
    vec4 p6 = uMVP * (b + dir * r2);
    vec4 p7 = uMVP * (a + dir * r2);

    gl_Position = p0;
    geomOut.uv = vec2(1,1);
    EmitVertex();
    gl_Position = p1;
    geomOut.uv = vec2(-1,1);
    EmitVertex();
    gl_Position = p2;
    geomOut.uv = vec2(1,0);
    EmitVertex();
    gl_Position = p3;
    geomOut.uv = vec2(-1,0);
    EmitVertex();

    gl_Position = p4;
    geomOut.uv = vec2(1,0);
    EmitVertex();
    gl_Position = p5;
    geomOut.uv = vec2(-1,0);
    EmitVertex();
    gl_Position = p6;
    geomOut.uv = vec2(1,1);
    EmitVertex();
    gl_Position = p7;
    geomOut.uv = vec2(-1,1);
    EmitVertex();

    EndPrimitive();
}

)SHADER";