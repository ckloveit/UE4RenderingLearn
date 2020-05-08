#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Classes/Kismet/BlueprintFunctionLibrary.h"
#include "MyShaderTest.generated.h"

USTRUCT(BlueprintType)
struct FMyShaderStructData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShaderData)
		FLinearColor ColorOne;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShaderData)
		FLinearColor ColorTwo;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShaderData)
		FLinearColor Colorthree;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShaderData)
		FLinearColor ColorFour;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShaderData)
		int32 ColorIndex;
};


UCLASS(MinimalAPI, meta = (ScriptName = "TestShaderLibrary"))
class UTestShaderBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin", meta = (WorldContext = "WorldContextObject"))
	static void DrawTestShaderRenderTarget(
		class UTextureRenderTarget2D* OutputRenderTarget, 
		AActor* Ac, 
		FLinearColor MyColor,
		class UTexture* MyTexture,
		FMyShaderStructData ShaderStructData);
};