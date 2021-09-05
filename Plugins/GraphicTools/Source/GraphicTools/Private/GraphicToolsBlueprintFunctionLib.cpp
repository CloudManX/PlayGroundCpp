#include "GraphicToolsBlueprintFunctionLib.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GlobalShader.h"

#define LOCTEXT_NAMESPACE "GraphicToolsPlugin"

class FCheckerBoardComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCheckerBoardComputeShader, Global, /*MYMODULE_API*/)
public:

	FCheckerBoardComputeShader() 
	{
	}

	FCheckerBoardComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("RWOutputSurface"));
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);  

	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
	}
	
	void SetParameters(
		FRHICommandList& RHICmdList,
		FTexture2DRHIRef& InOutputSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV
	)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();
		
		RHICmdList.Transition(FRHITransitionInfo(UAV, ERHIAccess::Unknown, ERHIAccess::UAVCompute));
		OutputSurface.SetTexture(RHICmdList, ShaderRHI, InOutputSurfaceValue, UAV);
	}
	
	void UnsetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef& UAV)
	{
		RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.UnsetUAV(RHICmdList, RHICmdList.GetBoundComputeShader());
	}

private:
	LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};

IMPLEMENT_SHADER_TYPE(, FCheckerBoardComputeShader, TEXT("/Plugin/GraphicTools/Private/CheckerBoard.usf"), TEXT("MainCS"), SF_Compute);

static void DrawCheckerBoard_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* TextureRenderTargetResource,
	// FMyShaderStructData ShaderStructData,
	ERHIFeatureLevel::Type FeatureLevel
)
{
	check(IsInRenderingThread());

	FTexture2DRHIRef RenderTargetTexture = TextureRenderTargetResource->GetRenderTargetTexture();

	FIntPoint FullResolution = FIntPoint(RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY());
	uint32 GGroupSize = 32;
	uint32 GroupSizeX = FMath::DivideAndRoundUp((uint32)RenderTargetTexture->GetSizeX(), GGroupSize);
	uint32 GroupSizeY = FMath::DivideAndRoundUp((uint32)RenderTargetTexture->GetSizeY(), GGroupSize);

	TShaderMapRef<FCheckerBoardComputeShader> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

	FRHIResourceCreateInfo CreateInfo;

	// Create a temp resource
	FTexture2DRHIRef GSurfaceTexture2D = RHICreateTexture2D(
		RenderTargetTexture->GetSizeX(),
		RenderTargetTexture->GetSizeY(),
		PF_FloatRGBA,
		1,
		1,
		TexCreate_ShaderResource | TexCreate_UAV,
		CreateInfo);
	
	FUnorderedAccessViewRHIRef GUAV = RHICreateUnorderedAccessView(GSurfaceTexture2D);

	ComputeShader->SetParameters(RHICmdList, RenderTargetTexture, GUAV);
	DispatchComputeShader(RHICmdList, ComputeShader, GroupSizeX, GroupSizeY, 1);
	ComputeShader->UnsetParameters(RHICmdList, GUAV);

	FRHICopyTextureInfo CopyInfo;
	RHICmdList.CopyTexture(GSurfaceTexture2D, RenderTargetTexture, CopyInfo);
}

void UGraphicToolsBlueprintLibrary::DrawCheckerBoard(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		FMessageLog("Blueprint").Warning(
			LOCTEXT("UGraphicToolsBlueprintLibrary::DrawCheckerBoard",
				"DrawUVDisplacementToRenderTarget: Output render target is required."));
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	ERHIFeatureLevel::Type FeatureLevel = WorldContextObject->GetWorld()->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[TextureRenderTargetResource, FeatureLevel](FRHICommandListImmediate& RHICmdList)
		{
			DrawCheckerBoard_RenderThread
			(
				RHICmdList,
				TextureRenderTargetResource,
				FeatureLevel
			);
		}
	);
}

#undef LOCTEXT_NAMESPACE
