static constexpr const char* circle_shader_frag_source = R"SHADER(

#version 400 core

out vec4 color;
in UvData {
	vec2 uv;
	float radius;
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

static constexpr const char* circle_shader_vert_source = R"SHADER(
    
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

static constexpr const char* circle_shader_geom_source = R"SHADER(

#version 400 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 uMVP;
uniform vec4 uColor;

in VertexData 
{
	vec4 position;
	float radius;
} vertOut[];

out UvData {
	vec2 uv;
	float radius;
} geomOut;

void main() {
	gl_Position = uMVP * (vertOut[0].position + vec4(-vertOut[0].radius, vertOut[0].radius, 0, 0));
	geomOut.uv = vec2(-1, 1);
	geomOut.radius = vertOut[0].radius;
	EmitVertex();
	
	gl_Position = uMVP * (vertOut[0].position + vec4(vertOut[0].radius, vertOut[0].radius, 0, 0));
	geomOut.uv = vec2(1, 1);
	geomOut.radius = vertOut[0].radius;
	EmitVertex();
	
	gl_Position = uMVP * (vertOut[0].position + vec4(-vertOut[0].radius, -vertOut[0].radius, 0, 0));
	geomOut.uv = vec2(-1, -1);
	geomOut.radius = vertOut[0].radius;
	EmitVertex();
	
	gl_Position = uMVP * (vertOut[0].position + vec4(vertOut[0].radius, -vertOut[0].radius, 0, 0));
	geomOut.uv = vec2(1, -1);
	geomOut.radius = vertOut[0].radius;
	EmitVertex();
	EndPrimitive();
}

)SHADER";