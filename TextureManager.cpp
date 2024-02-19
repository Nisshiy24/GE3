#include "TextureManager.h"
#include "BufferResource.h"

TextureManager* TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new TextureManager();
	}
	return instance;
}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::wstring& filePath)
{

	//読み込みが済みか確認する処理
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		return;
	}

	//最大数を超えないかチェックする
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);


	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	HRESULT result = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(result));

	DirectX::ScratchImage mipImages{};
	result = DirectX::GenerateMipMaps(
		image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB, 0, mipImages
	);
	assert(SUCCEEDED(result));

	//画像データ
	textureDatas.resize(textureDatas.size() + 1);
	//追加したデータ
	TextureData& data = textureDatas.back();

	data.filePath = filePath;
	data.metaData = mipImages.GetMetadata();
	data.resouce = CreateTextureResource(dxCommon_->GetDevice(), data.metaData);
	UploadTextureData(data.resouce.Get(), mipImages);

	//画像が保存されているメモリ
	uint32_t srvIndex = static_cast<uint32_t> (textureDatas.size() - 1) + kSRVIndexTop;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = dxCommon_->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = dxCommon_->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	handleCPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * srvIndex;
	handleGPU.ptr += dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * srvIndex;

	data.sevHandleCPU = handleCPU;
	data.srvHandleGPU = handleGPU;

	//ShaderResouceView作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = data.metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(data.metaData.mipLevels);

	//読み込んだ情報をSrvDesc（枠）とHandle（位置）を使って保存する
	dxCommon_->GetDevice()->CreateShaderResourceView(data.resouce.Get(), &srvDesc, data.sevHandleCPU);


}

uint32_t TextureManager::GetTextureIndexFilePath(const std::wstring& filePath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	//太陽の画像がなかった場合、停止する
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	//対象の要旨番号がメモリの範囲外を選択制てないか確認
	assert(textureIndex < DirectXCommon::kMaxSRVCount);

	//要素番号のTextureDataを受け取る
	TextureData& data = textureDatas[textureIndex];

	return data.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	//対象の要旨番号がメモリの範囲外を選択制てないか確認
	assert(textureIndex < DirectXCommon::kMaxSRVCount);

	//要素番号のTextureDataを受け取る
	TextureData& data = textureDatas[textureIndex];

	return data.metaData;
}

void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
	//Mate情報を取得
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	//全MipMap
	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		//連想（シェーダ）テクスチャの全情報を送る
		HRESULT result = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(result));
	}
}
