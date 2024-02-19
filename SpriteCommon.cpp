#include "SpriteCommon.h"


#include <cassert>


#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	HRESULT result{};
	dxCommon_ = dxCommon;


	//DXC������
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
	//�쐬
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	//�摜�p�̃f�B�X�N���v�^�͈�
	D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



	//RootParameter�쐬
	D3D12_ROOT_PARAMETER rootParamters[3]{};
	//�F
	rootParamters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamters[0].Descriptor.ShaderRegister - 0;

	//�s��
	rootParamters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParamters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParamters[1].Descriptor.ShaderRegister = 0;

	//�摜
	rootParamters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParamters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParamters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParamters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);



	descriptorRootSignature.pParameters = rootParamters;
	descriptorRootSignature.NumParameters = _countof(rootParamters);

	//Sanplen�ݒ�
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


	//�V���A���C�Y�Ƃ��ăo�C�i���ɂ���
	ComPtr<ID3D10Blob> signatureBlob;
	ComPtr<ID3D10Blob> errorBlob;
	result = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(result)) {
		assert(false);
	}

	//�o�C�i������ɍ쐬
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
	//���ʂ�`�悵�Ȃ�
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//�h��Ԃ�
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	//�ǂݍ��ݏ���
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
//	//�e�N�X�`���t�@�C����ǂݍ���Ńv���O�����ň�����悤�ɂ���
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
//	//Mate�����擾
//	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
//	//�SMipMap
//	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
//		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//
//		//�A�z�i�V�F�[�_�j�e�N�X�`���̑S���𑗂�
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

	//�ǂݍ��񂾃t�@�C��
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;


	LPCWSTR arguments[] = {
		//�R���p�C���Ώۂ�hlsl�t�@�C����
		filePath.c_str(),
		//�G���g���[�|�C���g�̎w��A��{�I��main�ȊO�ɂ͂��Ȃ�
		L"-E", L"main",
		//shaderProfile�̐ݒ�
		L"-T", profile,
		//�f�o�b�O�p�̏��𖄂ߍ���
		L"-Zi", L"-Qembed_debug",
		//�œK�����O���Ă���
		L"-Od",
		//�������A�E�g�͍s�D��
		L"-Zpr"
	};

	//���ۂ�shader���R���p�C������
	IDxcResult* shaderResult = nullptr;
	result = dxcCompiler->Compile(
		//�ǂݍ��񂾃t�@�C��
		&shaderSourceBuffer,
		//�R���p�C���I�v�V����
		arguments,
		//�R���p�C���I�v�V�����̐�
		_countof(arguments),
		//include���܂܂ꂽ���X
		includeHandler,
		//�R���p�C������
		IID_PPV_ARGS(&shaderResult)
	);
	assert(SUCCEEDED(result));



	//���������E�G���[���o�Ă��烍�O�ɏo���Ď~�߂�
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		
		//�x���E�G���[�_���[�b�^�C
		assert(false);
	}


	//4. Compile���ʂ��󂯎���ĕԂ�

	//�R���p�C�����ʂ�����s�p�̃o�C�i���������擾
	IDxcBlob* shaderBlob = nullptr;
	result = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(result));
	//�����������O���o��
	//�����g��Ȃ����O���o��
	shaderSource->Release();
	shaderResult->Release();
	//���s�p�̃o�C�i����ԋp
	return shaderBlob;

	//return nullptr;
}