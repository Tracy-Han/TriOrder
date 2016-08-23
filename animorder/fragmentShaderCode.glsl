#version 430

out vec4 daColor;
layout(binding=0, offset=0) uniform atomic_uint ac_frag;
void main()
{
	uint counter = atomicCounterIncrement(ac_frag);
	daColor = vec4(0.2,0.0,0.0,1.0);

}