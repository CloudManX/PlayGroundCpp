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
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
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
		FTexture2DRHIRef& InOuputSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV
	)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();

		RHICmdList.Transition(FRHITransitionInfo(UAV, ERHIAccess::ERWBarrier, ERHIAccess::ERWBarrier));
	}

private:
	LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};

#undef LOCTEXT_NAMESPACE

void UGraphicToolsBlueprintLibrary::DrawCheckerBoard(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderOutput)
{
}
