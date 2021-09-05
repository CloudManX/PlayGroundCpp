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

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, )
SHADER_PARAMETER(FVector4, ColorOne)
SHADER_PARAMETER(FVector4, ColorTwo)
SHADER_PARAMETER(FVector4, ColorThree)
SHADER_PARAMETER(FVector4, ColorFour)
SHADER_PARAMETER(int32, ColorIndex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, "FMyUniform");
 
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
        TestTextureVal.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
        TestTextureSampler.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
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
    void SetParameters(
        FRHICommandListImmediate& RHICmdList,
        TShaderRHIParamRef ShaderRHI,
        const FLinearColor& MyColor,
        FRHITexture* MyTexture,
        const FMyShaderStructData& ShaderStructData)
    {
        SetShaderValue(RHICmdList, ShaderRHI, SimpleColorVal, MyColor);

        SetTextureParameter(
            RHICmdList,
            ShaderRHI,
            TestTextureVal,
            TestTextureSampler,
            TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(),  
            MyTexture);  

        FMyUniformStructData UniformData;
        UniformData.ColorOne = ShaderStructData.ColorOne;
        UniformData.ColorTwo = ShaderStructData.ColorTwo;
        UniformData.ColorThree = ShaderStructData.ColorThree;
        UniformData.ColorFour = ShaderStructData.ColorFour;
        UniformData.ColorIndex = ShaderStructData.ColorIndex;
        
        SetUniformBufferParameterImmediate(RHICmdList, ShaderRHI, GetUniformBufferParameter<FMyUniformStructData>(), UniformData);
    } 

private:   
    // FShaderParameter SimpleColorVal;  
    LAYOUT_FIELD(FShaderParameter, SimpleColorVal, /*MYMODULE_API*/); 
    LAYOUT_FIELD(FShaderResourceParameter, TestTextureVal, /*MYMODULE_API*/); 
    LAYOUT_FIELD(FShaderResourceParameter, TestTextureSampler, /*MYMODULE_API*/); 
};  

class FMyComputeShader : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FMyComputeShader, Global, /*MYMODULE_API*/)
public:
    FMyComputeShader()
    {
    }
    FMyComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        OutputSurface.Bind(Initializer.ParameterMap, TEXT("RWOutputSurface"));
    }

    static bool ShouldCache(EShaderPlatform Platform)
    {
        return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
    }

    void SetParameters(
        FRHICommandList& RHICmdList,
        FUnorderedAccessViewRHIRef& OutputSurfaceUAV,
        FMyShaderStructData& ShaderStructData)
    {
        FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();
        if (OutputSurface.IsBound())
        {
            RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputSurface.GetUAVIndex(), OutputSurfaceUAV);
        }
		// RHICmdList.Transition(FRHITransitionInfo(OutputSurfaceUAV, ERHIAccess::Unknown, ERHIAccess::UAVCompute));
		// OutputSurface.SetTexture(RHICmdList, ComputeShaderRHI, InOutputSurfaceValue, OutputSurfaceUAV);

        FMyUniformStructData UniformData;
        UniformData.ColorOne = ShaderStructData.ColorOne;
        UniformData.ColorTwo = ShaderStructData.ColorTwo;
        UniformData.ColorThree = ShaderStructData.ColorThree;
        UniformData.ColorFour = ShaderStructData.ColorFour;
        UniformData.ColorIndex = ShaderStructData.ColorIndex;
        
        SetUniformBufferParameterImmediate(RHICmdList, ComputeShaderRHI, GetUniformBufferParameter<FMyUniformStructData>(), UniformData);
    } 

    void UnsetParameters(FRHICommandList& RHICmdList)
    {
        FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();

        if (OutputSurface.IsBound())
        {
            OutputSurface.UnsetUAV(RHICmdList, ComputeShaderRHI);
        }
    }
private:
    LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};
 
IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/Plugin/ShadertestPlugin/Private/MySimpleShader.usf"), TEXT("MainVS"), SF_Vertex)  
IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/Plugin/ShadertestPlugin/Private/MySimpleShader.usf"), TEXT("MainPS"), SF_Pixel)  
IMPLEMENT_SHADER_TYPE(, FMyComputeShader, TEXT("/Plugin/ShadertestPlugin/Private/MySimpleShader.usf"), TEXT("MainCS"), SF_Compute)  

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
        uint32 Stride = sizeof(FMyTextureVertex);
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, Position), VET_Float4, 0, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, UV), VET_Float2, 1, Stride));
        VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
    }

    virtual void ReleaseRHI() override
    {
        VertexDeclarationRHI->Release();
    }
};

static void UseComputeShader_RenderThread(
    FRHICommandListImmediate& RHICmdList,
    FTextureRenderTargetResource* OutputRenderTargetResource,
    FMyShaderStructData ShaderStructData,
    ERHIFeatureLevel::Type FeatureLevel
)
{
    check(IsInRenderingThread());

    TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(FeatureLevel));
    RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

    FTexture2DRHIRef RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
    int32 SizeX = OutputRenderTargetResource->GetSizeX();
    int32 SizeY = OutputRenderTargetResource->GetSizeY();
    uint32 GGroupSize = 32;
    uint32 GroupSizeX = FMath::DivideAndRoundUp((uint32)SizeX,  GGroupSize);
    uint32 GroupSizeY = FMath::DivideAndRoundUp((uint32)SizeY,  GGroupSize);

    FRHIResourceCreateInfo CreateInfo;

    FTexture2DRHIRef GSurfaceTexture2D = RHICreateTexture2D(
        SizeX,
        SizeY,
        PF_FloatRGBA,
        1,
        1,
        TexCreate_ShaderResource | TexCreate_UAV,
        CreateInfo
    );

    FUnorderedAccessViewRHIRef GSurfaceTextureUAV = RHICreateUnorderedAccessView(GSurfaceTexture2D);
    ComputeShader->SetParameters(RHICmdList, GSurfaceTextureUAV, ShaderStructData);
    RHICmdList.DispatchComputeShader(GroupSizeX, GroupSizeY, 1);
    ComputeShader->UnsetParameters(RHICmdList);

    FRHICopyTextureInfo CopyInfo;
    RHICmdList.CopyTexture(GSurfaceTexture2D, RenderTargetTexture, CopyInfo);
}
 
static void DrawTestShaderRenderTarget_RenderThread(  
    FRHICommandListImmediate& RHICmdList,   
    FTextureRenderTargetResource* OutputRenderTargetResource,  
    ERHIFeatureLevel::Type FeatureLevel,  
    FName TextureRenderTargetName,  
    const FLinearColor& MyColor,  
    FRHITexture* MyTexture,
    const FMyShaderStructData& ShaderStructData
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
        // RHICmdList.SetViewport(0, 0, 0.0f, DrawTargetResolution.X, DrawTargetResolution.Y, 1.0f);  

        FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);  
        TShaderMapRef<FShaderTestVS> VertexShader(GlobalShaderMap);  
        TShaderMapRef<FShaderTestPS> PixelShader(GlobalShaderMap);  

        FMyTextureVertexDeclaration VertexDec;
        VertexDec.InitRHI();

        // Set the graphic pipeline state.  
        FGraphicsPipelineStateInitializer GraphicsPSOInit;  
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);  
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();  
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();  
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();  
        GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;  

        // Bind the Texture 
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDec.VertexDeclarationRHI;

        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);  

        RHICmdList.SetViewport(0, 0, 0.0f, DrawTargetResolution.X, DrawTargetResolution.Y, 1.0f);  
        PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), MyColor, MyTexture, ShaderStructData);  
       
        FRHIResourceCreateInfo CreateInfo;
        FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FMyTextureVertex) * 4, BUF_Volatile, CreateInfo);
        FMyTextureVertex* Data = reinterpret_cast<FMyTextureVertex*>(RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FMyTextureVertex) * 4, RLM_WriteOnly));
        
        Data[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
        Data[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
        Data[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
        Data[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
        Data[0].UV = FVector2D(0.0f, 0.0f);
        Data[1].UV = FVector2D(1.0f, 0.0f);
        Data[2].UV = FVector2D(0.0f, 1.0f);
        Data[3].UV = FVector2D(1.0f, 1.0f);

        RHIUnlockVertexBuffer(VertexBufferRHI);
        
        RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
        RHICmdList.DrawPrimitive(0, 2, 1);
    }
    
    RHICmdList.EndRenderPass();
}  
 
void UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(  
    UTextureRenderTarget2D* OutputRenderTarget,   
    AActor* Ac,  
    FLinearColor MyColor,
    UTexture* MyTexture,
    FMyShaderStructData ShaderStructData
)  
{  
    check(IsInGameThread());  
 
    if (!OutputRenderTarget)  
    {  
        return;  
    }  

    if (!MyTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("The Texture is Missing. Custom shader failed."));
        return;
    }
 
    FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();  
    FTextureReferenceRHIRef MyTextureReferenceRHI = MyTexture->TextureReference.TextureReferenceRHI;
    FRHITexture* MyTextureRHI = MyTextureReferenceRHI->GetTextureReference()->GetReferencedTexture();
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

static void TextureWriting_RenderingThread(
    FRHICommandListImmediate& RHICmdList,
    ERHIFeatureLevel::Type FeatureLevel,
    UTexture2D* MyTexture
)
{
    check(IsInRenderingThread());

    if (MyTexture == nullptr)
    {
        return;
    }
    
    FTextureReferenceRHIRef MyTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;
    FRHITexture2D* MyTexRef2D = MyTextureRHI->GetTextureReference()->GetReferencedTexture()->GetTexture2D();


    TArray<FColor> BitMap;
    uint32 DestStride = 0;

    uint8* MyTextureData = (uint8*)RHICmdList.LockTexture2D(MyTexRef2D, 0, EResourceLockMode::RLM_ReadOnly, DestStride, false);

    for (uint32 Row = 0; Row < MyTexRef2D->GetSizeY(); ++Row)
	{
		for (uint32 Col = 0; Col < MyTexRef2D->GetSizeX(); ++Col)
		{
            uint32 EncodedPixel = *(uint32*)(MyTextureData + Col * sizeof(uint32) + Row * DestStride);
            uint8 R = (EncodedPixel & 0x000000FF);
            uint8 G = (EncodedPixel & 0x0000FF00) >> 8;
            uint8 B = (EncodedPixel & 0x00FF0000) >> 16;
            uint8 A = (EncodedPixel & 0xFF000000) >> 24;
			BitMap.Add(FColor(R, G, B, A));
		}
	}
    RHICmdList.UnlockTexture2D(MyTexRef2D, 0, false);

    TArray<FColor> colorData;
    colorData.Init(FColor(0, 0, 255, 255), 512 * 512);
    
    if (BitMap.Num() != 0) 
    {
        IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
        const FString ScreenShotFileName(FPaths::ScreenShotDir() / TEXT("VisualTexture"));
        uint32 ExtendXWithMSAA = BitMap.Num() / MyTexture->GetSizeY();
        UE_LOG(LogTemp, Warning, TEXT("ExtendXWithMSAA: %d"), ExtendXWithMSAA);
        FFileHelper::CreateBitmap(*ScreenShotFileName, MyTexture->GetSizeX(), MyTexture->GetSizeY(), BitMap.GetData());
        UE_LOG(LogConsoleResponse, Warning, TEXT("Content was saved to \"%s\" as size of %d x %d"), *FPaths::ScreenShotDir(), 
                MyTexture->GetSizeX(), MyTexture->GetSizeY());
    }
    else
    {
        UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
    }
}

void UTestShaderBlueprintLibrary::TextureWriting(UTexture2D* TextureToBeWritten, AActor* SelfRef)
{
    check(IsInGameThread());

    if (TextureToBeWritten == nullptr || SelfRef == nullptr)
    {
        return;
    }

    //TextureToBeWritten->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
    //TextureToBeWritten->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
    //TextureToBeWritten->SRGB = false;
    //TextureToBeWritten->UpdateResource();

    //FTexture2DMipMap& MipMap = TextureToBeWritten->PlatformData->Mips[0];
    //void* Data = MipMap.BulkData.Lock(LOCK_READ_WRITE);

    //int32 TextureX = TextureToBeWritten->PlatformData->SizeX;
    //int32 TextureY = TextureToBeWritten->PlatformData->SizeY;

    //TArray<FColor> Colors;
    //for (int32 i = 0; i < TextureX * TextureY; ++i)
    //{
    //    Colors.Add(FColor::Red);
    //}

    //// int32 Stride = static_cast<int32>(sizeof(uint8) * 4);
    //int32 Stride = static_cast<int32>(sizeof(FColor));

    //FMemory::Memcpy(Data, Colors.GetData(), TextureX * TextureY * Stride);
    //MipMap.BulkData.Unlock();
    //TextureToBeWritten->UpdateResource();

    UWorld* World = SelfRef->GetWorld();
    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
    
    ENQUEUE_RENDER_COMMAND(CaptureCommand)(
        [FeatureLevel, TextureToBeWritten](FRHICommandListImmediate& RHICmdList)
	    {
	    	TextureWriting_RenderingThread
	    	(
	    		RHICmdList,
	    		FeatureLevel,
	    		TextureToBeWritten
	    	);
	    }
	);
}

void UTestShaderBlueprintLibrary::DrawComputeShaderResult(
    UTextureRenderTarget2D* ComputedRenderTarget,
    AActor* Ac,
    FMyShaderStructData ShaderStructData
)
{
    check(IsInGameThread());

    if (Ac == nullptr || ComputedRenderTarget == nullptr)
    {
        return;
    }

    UWorld* World = Ac->GetWorld();
    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

    FTextureRenderTargetResource* TextureRenderTargetResource = ComputedRenderTarget->GameThread_GetRenderTargetResource();

    ENQUEUE_RENDER_COMMAND(CaptureCommand)(
        [TextureRenderTargetResource, ShaderStructData, FeatureLevel](FRHICommandListImmediate& RHICmdList)
        {
            UseComputeShader_RenderThread
            (
                RHICmdList,
                TextureRenderTargetResource,
                ShaderStructData,
				FeatureLevel
			);
	}
	);

}
 
#undef LOCTEXT_NAMESPACE  