// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.  
 
#include "MyShaderTest.h"  
 
#include "Engine/TextureRenderTarget2D.h"  
#include "Engine/World.h"  
#include "GlobalShader.h"  
#include "PipelineStateCache.h"  
#include "RHIStaticStates.h"  
#include "SceneUtils.h"  
#include "SceneInterface.h"  
#include "ShaderParameterUtils.h"  
#include "Logging/MessageLog.h"  
#include "Internationalization/Internationalization.h"  
#include "StaticBoundShaderState.h"  
 
#define LOCTEXT_NAMESPACE "TestShader"  
 
UTestShaderBlueprintLibrary::UTestShaderBlueprintLibrary(const FObjectInitializer& ObjectInitializer)  
    : Super(ObjectInitializer)  
{  
 
}  

class FMyShaderTest : public FGlobalShader  
{  
public:  
 
    FMyShaderTest() {}  
 
    FMyShaderTest(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FGlobalShader(Initializer)  
    {  
    }  
 
    static bool ShouldCache(EShaderPlatform Platform)  
    {  
        return true;  
    }  
    
};  
 
class FShaderTestVS : public FMyShaderTest 
{  
    DECLARE_EXPORTED_SHADER_TYPE(FShaderTestVS, Global, /*MYMODULE_API*/);
 
public:  
    FShaderTestVS() {}  
 
    FShaderTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FMyShaderTest(Initializer)  
    { 
    }  
};  
 
 
class FShaderTestPS : public FMyShaderTest  
{  
    DECLARE_SHADER_TYPE(FShaderTestPS, Global, /*MYMODULE_API*/);
 
public:  
    FShaderTestPS() {}  
 
    FShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FMyShaderTest(Initializer)  
    {  
        SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));   
    }  

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)  
    {  
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);  
        // return true;  
    }  
 
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)  
    {  
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);  
        OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);  
    }

    template<typename TShaderRHIParamRef>
    void SetParameters(FRHICommandListImmediate& RHICmdList, TShaderRHIParamRef ShaderRHI, const FLinearColor& MyColor)
    {
        SetShaderValue(RHICmdList, ShaderRHI, SimpleColorVal, MyColor);
    } 

private:   
    // FShaderParameter SimpleColorVal;  
    LAYOUT_FIELD(FShaderParameter, SimpleColorVal, /*MYMODULE_API*/); 
};  
 

IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/Plugin/ShadertestPlugin/Private/MySimpleShader.usf"), TEXT("MainVS"), SF_Vertex)  
IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/Plugin/ShadertestPlugin/Private/MySimpleShader.usf"), TEXT("MainPS"), SF_Pixel)  
 
static void DrawTestShaderRenderTarget_RenderThread(  
    FRHICommandListImmediate& RHICmdList,   
    FTextureRenderTargetResource* OutputRenderTargetResource,  
    ERHIFeatureLevel::Type FeatureLevel,  
    FName TextureRenderTargetName,  
    FLinearColor MyColor  
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
 
    FRHITexture2D* RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();

    RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));
    FRHIRenderPassInfo RenderPassInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store, OutputRenderTargetResource->TextureRHI);

    RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("MyShaderTest"));
    {
        // TODO: Rendering Codes
        // ÉèÖÃÊÓ¿Ú  
        FIntPoint DrawTargetResolution(OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY());  
        RHICmdList.SetViewport(0, 0, 0.0f, DrawTargetResolution.X, DrawTargetResolution.Y, 1.0f);  

        FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);  
        TShaderMapRef<FShaderTestVS> VertexShader(GlobalShaderMap);  
        TShaderMapRef<FShaderTestPS> PixelShader(GlobalShaderMap);  

        // Set the graphic pipeline state.  
        FGraphicsPipelineStateInitializer GraphicsPSOInit;  
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);  
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();  
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();  
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();  
        GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;  
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();  
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);  

        RHICmdList.SetViewport(0, 0, 0.0f, DrawTargetResolution.X, DrawTargetResolution.Y, 1.0f);  
        PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), MyColor);  
       
        FRHIResourceCreateInfo CreateInfo;
        FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FVector4) * 4, BUF_Static, CreateInfo);
        FVector4* Data = reinterpret_cast<FVector4*>(RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FVector4) * 4, RLM_WriteOnly));
        
        Data[0] = FVector4(-1.0f, 1.0f, 0, 1.0f);
        Data[1] = FVector4(1.0f, 1.0f, 0, 1.0f);
        Data[2] = FVector4(-1.0f, -1.0f, 0, 1.0f);
        Data[3] = FVector4(1.0f, -1.0f, 0, 1.0f);
        RHIUnlockVertexBuffer(VertexBufferRHI);
        
        RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
        RHICmdList.DrawPrimitive(0, 2, 1);
    }
    
    RHICmdList.EndRenderPass();
}  
 
void UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(  
    UTextureRenderTarget2D* OutputRenderTarget,   
    AActor* Ac,  
    FLinearColor MyColor  
)  
{  
    check(IsInGameThread());  
 
    if (!OutputRenderTarget)  
    {  
        return;  
    }  
 
    FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();  
    UWorld* World = Ac->GetWorld();  
    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();  
    FName TextureRenderTargetName = OutputRenderTarget->GetFName();  
    ENQUEUE_RENDER_COMMAND(CaptureCommand)(  
        [TextureRenderTargetResource, FeatureLevel, MyColor, TextureRenderTargetName](FRHICommandListImmediate& RHICmdList)  
        {  
            DrawTestShaderRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, TextureRenderTargetName, MyColor);  
        }  
    );  
 
}  
 
#undef LOCTEXT_NAMESPACE  