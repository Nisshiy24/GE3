#include "SpriteCommon.h"


#include <cassert>


#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	HRESULT result{};
	dxCommon_ = dxCommon;


	//DXC初期化
	ComPtr<IDxcUtils> dxcUils;
	ComPtr<IDxcCompiler3> dxcCompiler;
	result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUils));
	assert(SUCCEEDED(result));
	result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(result));

	ComPtr<IDxcIncludeHandler> includeHandler;
	result = dxcUils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(result));


	//RootSignature
	//作成
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	//画像用のディスクリプタ範囲
	D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParamters[3]{};
	//色
	rootParamters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamters[0].Descriptor.ShaderRegister - 0;

	//行列
	rootParamters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParamters[1].Descriptor.ShaderRegister = 0;

	//画像
	rootParamters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParamters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParamters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);



	descriptorRootSignature.pParameters = rootParamters;
	descriptorRootSignature.NumParameters = _countof(rootParamters);

	//Sanplen設定
	D3D12_STATIC_SAMPLER_DESC staticSamples[1] = {};
	staticSamples[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamples[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamples[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamples[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamples[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamples[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamples[0].ShaderRegister = 0;
	staticSamples[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptorRootSignature.pStaticSamplers = staticSamples;
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamples);


	//シリアライズとしてバイナリにする
	ComPtr<ID3D10Blob> signatureBlob;
	ComPtr<ID3D10Blob> errorBlob;
	result = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(result)) {
		assert(false);
	}

	//バイナリを基に作成
	result = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));

	//InPutLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[2] = {};
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDesc[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDesc[1].SemanticName = "TEXCOORD";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = _countof(inputElementDesc);



	//BlendState
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	//Rasterrizer
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を描画しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//塗りつぶし
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	//読み込み処理
	ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"Resources/shaders/SpriteVS.hlsl",
		L"vs_6_0", dxcUils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(SUCCEEDED(result));

	ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"Resources/shaders/SpritePS.hlsl",
		L"ps_6_0", dxcUils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(SUCCEEDED(result));


	//PipelineState
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	
	result = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));

	
}

void SpriteCommon::SpritePreDraw()
{
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(GetRootSignature());
	dxCommon_->GetCommandList()->SetPipelineState(GetPiplineState());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//DirectX::ScratchImage SpriteCommon::LoadTexture(const std::wstring& filePath)
//{
//	//テクスチャファイルを読み込んでプログラムで扱えるようにする
//	DirectX::ScratchImage image{};
//	HRESULT result = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
//	assert(SUCCEEDED(result));
//
//	DirectX::ScratchImage mipImages{};
//	result = DirectX::GenerateMipMaps(
//		image.GetImages(), image.GetImageCount(), image.GetMetadata(),
//		DirectX::TEX_FILTER_SRGB, 0, mipImages
//	);
//	assert(SUCCEEDED(result));
//
//	return image;
//}


//void SpriteCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
//{
//
//	//Mate情報を取得
//	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
//	//全MipMap
//	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
//		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//
//		//連想（シェーダ）テクスチャの全情報を送る
//		HRESULT result = texture->WriteToSubresource(
//			UINT(mipLevel),
//			nullptr,
//			img->pixels,
//			UINT(img->rowPitch),
//			UINT(img->slicePitch)
//		);
//		assert(SUCCEEDED(result));
//	}
//	
//}

IDxcBlob* SpriteCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler)
{
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT result = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(result));

	//読み込んだファイル
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;


	LPCWSTR arguments[] = {
		//コンパイル対象のhlslファイル名
		filePath.c_str(),
		//エントリーポイントの指定、基本的にmain以外にはしない
		L"-E", L"main",
		//shaderProfileの設定
		L"-T", profile,
		//デバッグ用の情報を埋め込む
		L"-Zi", L"-Qembed_debug",
		//最適化を外しておく
		L"-Od",
		//メモリアウトは行優先
		L"-Zpr"
	};

	//実際にshaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	result = dxcCompiler->Compile(
		//読み込んだファイル
		&shaderSourceBuffer,
		//コンパイルオプション
		arguments,
		//コンパイルオプションの数
		_countof(arguments),
		//includeが含まれた諸々
		includeHandler,
		//コンパイル結果
		IID_PPV_ARGS(&shaderResult)
	);
	assert(SUCCEEDED(result));



	//けいこく・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		
		//警告・エラーダメゼッタイ
		assert(false);
	}


	//4. Compile結果を受け取って返す

	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	result = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(result));
	//成功したログを出す
	//もう使わないログを出す
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;

	//return nullptr;
}
