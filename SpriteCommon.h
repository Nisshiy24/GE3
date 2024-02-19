#pragma once
#include <Windows.h>
#include <wrl.h>
#include <string>
#include <dxcapi.h>
#include <d3d12.h>
#include "DirectXCommon.h"

#include <DirectXTex.h>

//‹¤’Ê
class SpriteCommon
{
private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	

public:
	//‰Šú‰»
	void Initialize(DirectXCommon* dxCommon);
	void SpritePreDraw();

	//Getter
	ID3D12RootSignature* GetRootSignature() { return rootSignature.Get(); }
	ID3D12PipelineState* GetPiplineState() { return pipelineState.Get(); }
	DirectXCommon* GetDirectXCommon() { return dxCommon_; }


	////‰æ‘œ“Ç‚İ‚İ
	//DirectX::ScratchImage LoadTexture(const std::wstring& filePath);

	////“Ç‚İ‚ñ‚¾‰æ‘œ‚ğGPU(ƒVƒƒ[‚¾‚É‘—‚é)
	//void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

private:
	static IDxcBlob* CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3 * dxcCompiler,
		IDxcIncludeHandler* includeHandler
		);

private:
	DirectXCommon* dxCommon_ = nullptr;

	ComPtr<ID3D12RootSignature> rootSignature;

	ComPtr<ID3D12PipelineState> pipelineState;
};

	 