static constexpr const char* triangle_shader_frag_source = R"SHADER(

#version 400 core

in VertexData
{
	vec4 position;
	vec4 color;
} fragIn;
out vec4 color;

void main()
{
	color = fragIn.color;
}

)SHADER";

static constexpr const char* triangle_shader_vert_source = R"SHADER(
    
#version 400 core

layout(location = 0) in vec4 vertInPosition;

uniform mat4 uMVP;
uniform vec4 uColor;

out VertexData 
{
	vec4 position;
	vec4 color;
} vertOut;

void main()
{
	gl_Position = uMVP * vertInPosition;

	vertOut.color = uColor;
	vertOut.position = vertInPosition;
}

)SHADER";