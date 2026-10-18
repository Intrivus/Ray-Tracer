struct Output {
    float4 color : TEXCOORD0;
    float4 pos   : SV_Position;
};

Output main(uint id : SV_VertexID) {
    float2 positions[3] = { float2(-0.5, -0.5), float2(0.5, -0.5), float2(0.0, 0.5) };
    float3 colors[3]    = { float3(1,0,0),      float3(0,1,0),     float3(0,0,1) };

    Output o;
    o.pos = float4(positions[id], 0.0, 1.0);
    o.color = float4(colors[id], 1.0);
    return o;
}