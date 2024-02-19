#pragma once
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <vector>
#include "WinApp.h"
#include <chrono>

class DirectXCommon
{
private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
		void Initialize(WinApp* winApp);

		void PreDraw();
		void PostDraw();

		ID3D12Device* GetDevice() const { return device.Get(); }
		ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }



		//スワップチェーン
		DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc(){ return swapChainDesc; }

		//RTVディスク
		D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() { return rtvDesc; }
		//SRVディスクリプターヒープ
		ID3D12DescriptorHeap* GetSrvDescriptorHeap() { return srvDescriptorHeap.Get(); }

private:
	void DeviceInitialize();

	void CommandInitialize();

	void SwapChainInitialize();

	void RenderTargetInitialize();

	void DepthBufferInitialize();

	void FenceInitialize();


	//ディスクリプタヒープ作成
	ID3D12DescriptorHeap* CreareDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescripots, bool shaderVisible);


	void InitializeFixFPS();
	void UpdateFixFPS();
	
public:
	static const uint32_t kMaxSRVCount;	


private:

	//WinApp* winApp_ = nullptr;

	//Microsoft::WRL:: ComPtr<IDXGIFactory7> dxgiFactory;
	//Microsoft::WRL:: ComPtr<ID3D12Device> device;

	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

	//Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	//DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	//std::vector<ComPtr<ID3D12Resource>> backBuffers;
	////Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers;

	//Microsoft::WRL::ComPtr<ID3D12Resource> depthBuff;
	//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;

	//Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	//UINT64 fenceVal = 0;

	//D3D12_RESOURCE_BARRIER barrierDesc{};

	//std::chrono::steady_clock::time_point reference_;



	WinApp* winApp_ = nullptr;

	ComPtr<IDXGIFactory7> dxgiFactory;
	ComPtr<ID3D12Device> device;
	//ID3D12Device* device;

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12CommandQueue> commandQueue;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	ComPtr<IDXGISwapChain4> swapChain;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	//Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers;

	ComPtr<ID3D12Resource> depthBuff;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	ComPtr<ID3D12DescriptorHeap> dsvHeap;

	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;

	D3D12_RESOURCE_BARRIER barrierDesc{};

	std::chrono::steady_clock::time_point reference_;


	// レンダーターゲットビューの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	//ディスクリプターヒープ
	//RTV(ゲーム画面を保存しておく)
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	//SRV(画像などを保存しておくもの)
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

};

