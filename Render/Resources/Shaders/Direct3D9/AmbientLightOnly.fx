uniform shared extern float4x4 CameraVP;
uniform shared extern float SceneHeight;

struct OutputVS {
    float4 posH : POSITION0;
    float4 posW : POSITION1;
};

OutputVS BasicVertexShader(
    float3 ObjectSpacePosition : POSITION0,
    float4 VertexColour : COLOR0,
    float4 WorldMatrixRow1 : TEXCOORD0,
    float4 WorldMatrixRow2 : TEXCOORD1,
    float4 WorldMatrixRow3 : TEXCOORD2,
    float4 WorldMatrixRow4 : TEXCOORD3
) {
    OutputVS output;
    float4x4 World = { WorldMatrixRow1, WorldMatrixRow2, WorldMatrixRow3, WorldMatrixRow4 };
    output.posW = mul(float4(ObjectSpacePosition, 1.0f), World);
    output.posH = mul(output.posW, CameraVP);
    return output;
}

float3 Hue(float H)
{
    float R = abs(H * 6 - 3) - 1;
    float G = 2 - abs(H * 6 - 2);
    float B = 2 - abs(H * 6 - 4);
    return saturate(float3(R,G,B));
}

float3 HSVtoRGB(in float3 HSV)
{
    return ((Hue(HSV.x) - 1) * HSV.y + 1) * HSV.z;
}

float4 BasicPixelShader(
    float4 WorldPos : POSITION1
) : COLOR0 {
    WorldPos /= WorldPos.w; // perspective divide
    float H = WorldPos.y / SceneHeight;
    float S = 1;
    float V = 1;
    return float4(HSVtoRGB(float3(H, S, V)), 1);
}

technique PrimaryTechnique
{
      pass P0
      {
            // Specify the vertex and pixel shader associated
            // with this pass.
            vertexShader = compile vs_3_0 BasicVertexShader();
            pixelShader  = compile ps_3_0 BasicPixelShader();
      }
}