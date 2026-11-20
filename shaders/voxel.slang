struct VSInput {
    float3 inPosition;
    float3 inColor;
};

struct VSOutput
{
    float4 pos : SV_Position;
   // float3 color;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

static const float3 faceColors[6] = {
    float3(1, 1, 1),
    float3(0, 1, 1),
    float3(0, 1, 0),
    float3(0, 0, 1),
    float3(1, 0, 1),
    float3(1, 1, 0),
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    //output.color = input.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn, uint primID : SV_PrimitiveID) : SV_TARGET {
    uint faceID = (primID/2)%6;
    return float4(faceColors[faceID], 1.0);
}
