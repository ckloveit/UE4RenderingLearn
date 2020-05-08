#include "MyShaderTest.h"
#include "Classes/Engine/TextureRenderTarget2D.h"
#include "Classes/Engine/World.h"
#include "Runtime/Engine/Public/SceneInterface.h"
#include "Public/GlobalShader.h"  
#include "Public/PipelineStateCache.h"
#include "Public/RHIStaticStates.h"
#include "Public/SceneUtils.h"
#include "Public/ShaderParameterUtils.h"
#include "Public/Logging/MessageLog.h"  
#include "Public/Internationalization/Internationalization.h"  
#include "Public/StaticBoundShaderState.h" 

#include "RHICommandList.h"
#include "UniformBuffer.h"

#define LOCTEXT_NAMESPACE "TestShader"  

// Uniform Buffer ---> DX11 Constant Buffer
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, )
SHADER_PARAMETER(FVector4, ColorOne)
SHADER_PARAMETER(FVector4, ColorTwo)
SHADER_PARAMETER(FVector4, ColorThree)
SHADER_PARAMETER(FVector4, ColorFour)
SHADER_PARAMETER(uint32, ColorIndex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, "FMyUniform");


struct FMyTextureVertex
{
	FVector4 Position;
	FVector2D UV;
};

class FMyTextureVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 stride = sizeof(FMyTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, Position), VET_Float4, 0, stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, UV), VET_Float2, 1, stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI->Release();
	}
};


class FMyShaderTest : public FGlobalShader
{
public:

	FMyShaderTest() {}

	FMyShaderTest(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));
		TestTextureVal.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
		TestTextureSampler.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		//return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4);  
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);
	}

	void SetParameters(
		FRHICommandListImmediate& RHICmdList,
		const FLinearColor& MyColor,
		FTextureRHIParamRef& MyTexture,
		FMyShaderStructData& ShaderStructData
	)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), SimpleColorVal, MyColor);
		SetTextureParameter(RHICmdList,
			GetPixelShader(),
			TestTextureVal,
			TestTextureSampler,
			TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),
			MyTexture);
		FMyUniformStructData UniformData;
		UniformData.ColorOne = ShaderStructData.ColorOne;
		UniformData.ColorTwo = ShaderStructData.ColorTwo;
		UniformData.ColorThree = ShaderStructData.Colorthree;
		UniformData.ColorFour = ShaderStructData.ColorFour;
		UniformData.ColorIndex = ShaderStructData.ColorIndex;
		UniformData.ColorIndex = ShaderStructData.ColorIndex;
		SetUniformBufferParameterImmediate(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FMyUniformStructData>(), UniformData);
	}

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		Ar << SimpleColorVal;
		Ar << TestTextureVal;
		Ar << TestTextureSampler;
		return bShaderHasOutdatedParameters;
	}

private:

	FShaderParameter SimpleColorVal;

	// Texture
	FShaderResourceParameter TestTextureVal;
	FShaderResourceParameter TestTextureSampler;


};

class FShaderTestVS : public FMyShaderTest
{
	DECLARE_SHADER_TYPE(FShaderTestVS, Global);

public:
	FShaderTestVS() {}

	FShaderTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMyShaderTest(Initializer)
	{

	}
};


class FShaderTestPS : public FMyShaderTest
{
	DECLARE_SHADER_TYPE(FShaderTestPS, Global);

public:
	FShaderTestPS() {}

	FShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMyShaderTest(Initializer)
	{

	}
};

IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/ShaderTest/Private/MyShader.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/ShaderTest/Private/MyShader.usf"), TEXT("MainPS"), SF_Pixel)

static void DrawTestShaderRenderTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FName TextureRenderTargetName,
	FLinearColor MyColor,
	FTextureRHIParamRef MyTexture,
	FMyShaderStructData ShaderStructData
	)
{
	check(IsInRenderingThread());
#if WANTS_DRAW_MESH_EVENTS  
	FString EventName;
	TextureRenderTargetName.ToString(EventName);
	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("ShaderTest %s"), *EventName);
#else  
	SCOPED_DRAW_EVENT(RHICmdList, DrawUVDisplacementToRenderTarget_RenderThread);
#endif 
	//ÉèÖÃäÖÈ¾Ä¿±ê  
	SetRenderTarget(
		RHICmdList,
		OutputRenderTargetResource->GetRenderTargetTexture(),
		FTextureRHIRef(),
		ESimpleRenderTargetMode::EUninitializedColorAndDepth,
		FExclusiveDepthStencil::DepthNop_StencilNop
	);

	TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FShaderTestVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FShaderTestPS> PixelShader(GlobalShaderMap);

	// Vertex Input Layout
	FMyTextureVertexDeclaration VertexDesc;
	VertexDesc.InitRHI();

	//Set the graphics pipeline state
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDesc.VertexDeclarationRHI;// GetVertexDeclarationFVector4();
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	//RHICmdList.SetViewport(0, 0, 0.0f, DrawTargetResolution.X, DrawTargetResolution.Y, 1.0f);  
	PixelShader->SetParameters(RHICmdList, MyColor, MyTexture, ShaderStructData);
	// Draw grid.  
	//uint32 PrimitiveCount = 2;  
	//RHICmdList.DrawPrimitive(PT_TriangleList, 0, PrimitiveCount, 1);  
	FMyTextureVertex Vertices[4];
	Vertices[0].Position.Set(-1.0f, 1.0f, 0, 1.0f);
	Vertices[1].Position.Set(1.0f, 1.0f, 0, 1.0f);
	Vertices[2].Position.Set(-1.0f, -1.0f, 0, 1.0f);
	Vertices[3].Position.Set(1.0f, -1.0f, 0, 1.0f);
	Vertices[0].UV = FVector2D(0.0f, 1.0f);
	Vertices[1].UV = FVector2D(1.0f, 1.0f);
	Vertices[2].UV = FVector2D(0.0f, 0.0f);
	Vertices[3].UV = FVector2D(1.0f, 0.0f);
	static const uint16 Indices[6] =
	{
		0, 1, 2,
		2, 1, 3
	};
	DrawIndexedPrimitiveUP(RHICmdList,
		PT_TriangleList,
		0,
		ARRAY_COUNT(Vertices),
		2,
		Indices,
		sizeof(Indices[0]),
		Vertices,
		sizeof(Vertices[0])
	);
	//Resolve render target
	RHICmdList.CopyToResolveTarget(
		OutputRenderTargetResource->GetRenderTargetTexture(),
		OutputRenderTargetResource->TextureRHI,
		FResolveParams());
}

void UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(
	class UTextureRenderTarget2D* OutputRenderTarget, 
	AActor* Ac,
	FLinearColor MyColor,
	UTexture* MyTexture,
	FMyShaderStructData ShaderStructData)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	FTextureRHIParamRef MyTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;
	UWorld* World = Ac->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, MyColor, TextureRenderTargetName, MyTextureRHI, ShaderStructData](FRHICommandListImmediate& RHICmdList)
		{
			DrawTestShaderRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName, MyColor, MyTextureRHI, ShaderStructData);
		}
	);
}
#undef LOCTEXT_NAMESPACE  