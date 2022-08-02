#pragma region �ǂݍ��ރw�b�_�[

#define DIRECTINPUT_VERSION 0x0800 //DirectInput�̃o�[�W�����w��
#pragma once

#include<dinput.h>
#include<d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#include<Windows.h>
#include <tchar.h>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include <vector>
#include <string>
#include<DirectXMath.h>
#include <math.h>
#include <DirectXTex.h>
#include <wrl.h>


using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

#pragma endregion
const float PI = 3.141592f;


// �萔�o�b�t�@�p�f�[�^�\���́i�}�e���A���j
struct ConstBufferDataMaterial
{
	XMFLOAT4 color; // �F (RGBA)
};
// �萔�o�b�t�@�p�f�[�^�\����Pos�i�}�e���A���j
struct ConstBufferDataMaterialPos
{
	XMFLOAT4 Move; // �ړ�
};
//�萔�o�b�t�@�p�f�[�^�\����(3D�ϊ��s��)
struct ConstBufferDataTransform
{
	XMMATRIX mat;
};

//3D�I�u�W�F�N�g
struct Object3d
{
	//�萔�o�b�t�@(�s��p)
	ComPtr<ID3D12Resource> constBuffTransform;
	//�萔�o�b�t�@�}�b�v
	ConstBufferDataTransform* constMapTransform = nullptr;
	//�A�t�B���ϊ����
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//���[���h�ϊ��s��
	XMMATRIX matWorld{};
	//�e�I�u�W�F�N�g�ւ̃|�C���^
	Object3d* parent = nullptr;
};

//3D�I�u�W�F�N�g������
void InitializeObject3d(Object3d* object, ID3D12Device* device)
{

	HRESULT result;

	//�萔�o�b�t�@�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	//�萔�o�b�t�@�̃��\�[�X�ݒ�
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//�萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result)
	);
	//�萔�o�b�t�@�̃}�b�s���O
	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform);
	assert(SUCCEEDED(result));
}

void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection)
{

	XMMATRIX matScale, matRot, matTrans;

	//�X�P�[���A��]�A���s�ړ��s��̌v�Z
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//���[���h�s��̍���
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//�e�I�u�W�F�N�g�������
	if (object->parent != nullptr)
	{
		//�e�I�u�W�F�N�g�̃��[���h�s���������
		object->matWorld *= object->parent->matWorld;
	}

	//�萔�o�b�t�@�փf�[�^�]��
	object->constMapTransform->mat = object->matWorld * matView * matProjection;
}

void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices)
{
	//���_�o�b�t�@�̐ݒ�
	commandList->IASetVertexBuffers(0, 1, &vbView);
	//�C���f�b�N�X�o�b�t�@�̐ݒ�
	commandList->IASetIndexBuffer(&ibView);
	//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());
	//�`��R�}���h
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}





#pragma region ���܂��Ȃ�
// @brief �R���\�[����ʂɃt�H�[�}�b�g�t��������̕\��
// @param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ���)
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�N�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OS�ɑ΂��āu���̃A�v���͂����I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

#pragma region WindowsAPI����������

	// �E�B���h�E����
	const int window_width = 1280;
	// �E�B���h�E�c��
	const int window_height = 720;


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	//�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = _T("DX12Sample");			//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr);		//�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y{X���W�@Y���W�@�����@�c��}
	RECT wrc = { 0,0,window_width,window_height };
	//�֐����g���ăE�B���h�E�̃T�C�Y�������ŕ␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X���w��
		_T("LE2B_10_�J�~�T�g_�}�T�g�V"),					//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,			//�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,					//�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,					//�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,			//�E�B���h�E��
		wrc.bottom - wrc.top,			//�E�B���h�E��
		nullptr,						//�e�E�B���h�E�n���h��
		nullptr,						//���j���[�n���h��
		w.hInstance,					//�Ăяo���A�v���P�[�V�����n���h��
		nullptr);						//�ǉ��p�����[�^�[(�I�v�V����)

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

#pragma endregion

#pragma region DirectX����������

	////////////////////////////////////////////////////
	//-------------DirectX12������������������-----------//
	//////////////////////////////////////////////////

#ifdef _DEBUG
//�f�o�b�O���C���[���I����
	ComPtr < ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ComPtr <ID3D12Device> device;
	ComPtr <IDXGIFactory6> dxgiFactory;
	ComPtr <IDXGISwapChain4> swapChain;
	ComPtr <ID3D12CommandAllocator> commandAllocator;
	ComPtr <ID3D12GraphicsCommandList> commandList;
	ComPtr <ID3D12CommandQueue> commandQueue;
	ComPtr <ID3D12DescriptorHeap> rtvHeap;

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[�̗񋓗p
	std::vector< ComPtr<IDXGIAdapter4>> adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr < IDXGIAdapter4> tmpAdapter;

	//�p�t�H�[�}���X�����������珇�ɁA�S�ẴA�_�v�^�[��񋓂���
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}
	//�Ó��ȃA�_�v�^��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i].Get();
			break;
		}
	}
	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++)
	{
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK)
		{
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	//�R�}���h�A���[�P�[�^�𐶐�
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//�X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//�F���̏���
	swapChainDesc.SampleDesc.Count = 1;							//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;			//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;								//�o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//IDXGISwapChain1��ComPtr��p��
	ComPtr<IDXGISwapChain1> swapchain1;

	//�X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapchain1);

	//��������IDXGISwapChain1�̃I�u�W�F�N�g��IDXGISwapChain4�ɕϊ�����
	swapchain1.As(&swapChain);

	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			//�����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;		//���\��2��
	//�f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�[�x�o�b�t�@�̃��\�[�X�ݒ�
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Width = window_width;
	depthResourceDesc.Height = window_height;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//�[�x�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//�[�x�o�b�t�@�̃��\�[�X����
	ComPtr < ID3D12Resource> depthBuff;
	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);
	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr < ID3D12DescriptorHeap> dsvHeap;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);


	//�o�b�N�o�b�t�@
	std::vector< ComPtr<ID3D12Resource>> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//�X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	//�t�F���X�̐���
	ComPtr < ID3D12Fence> fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//DirectInput�̏�����
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//�L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//�W���`��
	assert(SUCCEEDED(result));

	//3D�I�u�W�F�N�g�̐�
	const size_t kObjectCount = 50;
	//3D�I�u�W�F�N�g�̔z��
	Object3d object3ds[kObjectCount];

	/////////////////////////////////////////////////////////
	//--------------DirectX12�����������@�����܂�-------------//
	///////////////////////////////////////////////////////
#pragma endregion

#pragma region �`�揉��������

	//////////////////////////////////////////////////////
	//---------------�`�揉�������� ��������----------------//
	/////////////////////////////////////////////////////

	TexMetadata metadata{};
	ScratchImage scratchImg{};
	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(
		L"Resources/mario.jpg",   //�uResources�v�t�H���_�́utexture.png�v
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	ScratchImage mipChain{};
	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result))
	{
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}
	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	//2���ڗp�̕ʂ̕ϐ���p��
	TexMetadata metadata2{};
	ScratchImage scratchImg2{};
	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(
		L"Resources/reimu.png",   //�uResources�v�t�H���_�́utexture.png�v
		WIC_FLAGS_NONE,
		&metadata2, scratchImg2);
	ScratchImage mipChain2{};
	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
		scratchImg2.GetImages(), scratchImg2.GetImageCount(), scratchImg2.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain2);
	if (SUCCEEDED(result))
	{
		scratchImg2 = std::move(mipChain2);
		metadata2 = scratchImg2.GetMetadata();
	}
	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata2.format = MakeSRGB(metadata2.format);

	// �q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//// ���\�[�X�ݒ�
	//D3D12_RESOURCE_DESC textureResourceDesc{};
	//textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; textureResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; textureResourceDesc.Width = textureWidth;  // ��
	//textureResourceDesc.Height = textureWidth; // ����
	//textureResourceDesc.DepthOrArraySize = 1;
	//textureResourceDesc.MipLevels = 1;
	//textureResourceDesc.SampleDesc.Count = 1;

	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;

	D3D12_RESOURCE_DESC textureResourceDesc2{};
	textureResourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc2.Format = metadata2.format;
	textureResourceDesc2.Width = metadata2.width;
	textureResourceDesc2.Height = (UINT)metadata2.height;
	textureResourceDesc2.DepthOrArraySize = (UINT16)metadata2.arraySize;
	textureResourceDesc2.MipLevels = (UINT16)metadata2.mipLevels;
	textureResourceDesc2.SampleDesc.Count = 1;

	// �e�N�X�`���o�b�t�@�̐���
	ComPtr < ID3D12Resource> texBuff;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff));

	// �e�N�X�`���o�b�t�@�̐���2
	ComPtr < ID3D12Resource> texBuff2;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff2));

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	//result = texBuff->WriteToSubresource(
	//	0,
	//	nullptr, // �S�̈�փR�s�[
	//	imageData,    // ���f�[�^�A�h���X
	//	sizeof(XMFLOAT4) * textureWidth, // 1���C���T�C�Y
	//	sizeof(XMFLOAT4) * imageDataCount // �S�T�C�Y
	//);
	// �S�~�b�v�}�b�v�ɂ���
	for (size_t i = 0; i < metadata.mipLevels; i++)
	{
		// �~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,              // �S�̈�փR�s�[
			img->pixels,          // ���f�[�^�A�h���X
			(UINT)img->rowPitch,  // 1���C���T�C�Y
			(UINT)img->slicePitch // 1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}
	// �S�~�b�v�}�b�v�ɂ���
	for (size_t i = 0; i < metadata2.mipLevels; i++)
	{
		// �~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img2 = scratchImg2.GetImage(i, 0, 0);
		// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = texBuff2->WriteToSubresource(
			(UINT)i,
			nullptr,              // �S�̈�փR�s�[
			img2->pixels,          // ���f�[�^�A�h���X
			(UINT)img2->rowPitch,  // 1���C���T�C�Y
			(UINT)img2->slicePitch // 1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}


	// SRV�̍ő��
	const size_t kMaxSRVCount = 2056;

	// �f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	// �ݒ������SRV�p�f�X�N���v�^�q�[�v�𐶐�
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));





	//// ���_�f�[�^
//XMFLOAT3 vertices[] = {
//{ -0.5f, -0.5f, 0.0f }, // ���� �C���f�b�N�X0
//{ -0.5f, +0.5f, 0.0f }, // ���� �C���f�b�N�X1
//{ +0.5f, -0.5f, 0.0f }, // �E�� �C���f�b�N�X2
//{ +0.5f, +0.5f, 0.0f }, // �E�� �C���f�b�N�X3
//};

// ���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT3 normal; //�@���x�N�g��
		XMFLOAT2 uv;  // uv���W
	};
	// ���_�f�[�^
	Vertex vertices[] = {
		//  x        y     z       u     v
		 //�O
		{{-5.0f, -5.0f, -5.0f},{} ,{0.0f, 1.0f}}, // ����
		{{-5.0f, 5.0f, -5.0f},{} , {0.0f, 0.0f}}, // ����
		{{5.0f, -5.0f, -5.0f},{} , {1.0f, 1.0f}}, // �E��
		{{5.0f, 5.0f, -5.0f}, {} ,{1.0f, 0.0f}}, // �E��
		//���
		{{-5.0f, -5.0f, 5.0f}, {} ,{0.0f, 1.0f}}, // ����
		{{-5.0f, 5.0f, 5.0f},{} , {0.0f, 0.0f}}, // ����
		{{5.0f, -5.0f, 5.0f}, {} ,{1.0f, 1.0f}}, // �E��
		{{5.0f, 5.0f, 5.0f},{} , {1.0f, 0.0f}}, // �E��
		//��
		{{-5.0f, -5.0f, -5.0f},{} , {0.0f, 1.0f}}, // ����
		{{-5.0f, -5.0f, 5.0f},{} , {0.0f, 0.0f}}, // ����
		{{-5.0f, 5.0f, -5.0f},{} , {1.0f, 1.0f}}, // �E��
		{{-5.0f, 5.0f, 5.0f}, {} ,{1.0f, 0.0f}}, // �E��
		//�E
		{{5.0f, -5.0f, -5.0f},{} , {0.0f, 1.0f}}, // ����
		{{5.0f, -5.0f, 5.0f},{} , {0.0f, 0.0f}}, // ����
		{{5.0f, 5.0f, -5.0f}, {} ,{1.0f, 1.0f}}, // �E��
		{{5.0f, 5.0f, 5.0f},{} , {1.0f, 0.0f}}, // �E��
		//��
		{{-5.0f, -5.0f, -5.0f},{} , {0.0f, 1.0f}}, // ����
		{{-5.0f, -5.0f, 5.0f}, {} ,{0.0f, 0.0f}}, // ����
		{{5.0f, -5.0f, -5.0f}, {} ,{1.0f, 1.0f}}, // �E��
		{{5.0f, -5.0f, 5.0f}, {} ,{1.0f, 0.0f}}, // �E��
		//��
		{{-5.0f, 5.0f, -5.0f}, {} ,{0.0f, 1.0f}}, // ����
		{{-5.0f, 5.0f, 5.0f}, {} ,{0.0f, 0.0f}}, // ����
		{{5.0f, 5.0f, -5.0f}, {} ,{1.0f, 1.0f}}, // �E��
		{{5.0f, 5.0f, 5.0f},{} , {1.0f, 0.0f}}, // �E��
	};

	//// �C���f�b�N�X�f�[�^
	//uint16_t indices[] =
	//{
	//	0, 1, 2, // �O�p�`1��
	//	1, 2, 3, // �O�p�`2��
	//};

	// �C���f�b�N�X�f�[�^
	// �C���f�b�N�X�f�[�^
	unsigned short indices[] = {
		//�O
		0, 1, 2, // �O�p�`1��
		3, 2, 1, // �O�p�`2��
		//��
		6, 5, 4, // �O�p�`3��
		5, 6, 7, // �O�p�`4��
		//��
		8, 9, 10, // �O�p�`5��
		11, 10, 9, // �O�p�`6��
		//�E
		14, 13, 12, // �O�p�`7��
		13, 14, 15, // �O�p�`8��
		//��
		18, 17, 16, // �O�p�`9��
		17, 18, 19, // �O�p�`10��
		//��
		20, 21, 22, // �O�p�`11��
		23, 22, 21, // �O�p�`12��
	};


	/*float transformX = 0.0f;
	float transformY = 0.0f;
	float rotation = 0.0f;
	float scale = 1.0f;
	float affin[3][3] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};*/


	// ���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	/*UINT sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(vertices));*/
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));
	// ���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{}; // �q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU�ւ̓]���p
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB; // ���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// ���_�o�b�t�@�̐���
	ComPtr < ID3D12Resource> vertBuff;
	result = device->CreateCommittedResource(
		&heapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// �C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	// ���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; // �C���f�b�N�X��񂪓��镪�̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// �C���f�b�N�X�o�b�t�@�̐���
	ComPtr < ID3D12Resource> indexBuff;
	result = device->CreateCommittedResource(
		&heapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	// �C���f�b�N�X�o�b�t�@���}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// �S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];   // �C���f�b�N�X���R�s�[
	}
	// �}�b�s���O����
	indexBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	//�@���v�Z
	for (int i = 0; i < _countof(indices) / 3; i++)
	{//�O�p�`1���ƂɌv�Z���Ă���
		//�O�p�`�̃C���f�b�N�X�����o���Ĉꎞ�I�ȕϐ��ɓ����
		unsigned short indices_0 = indices[i * 3 + 0];
		unsigned short indices_1 = indices[i * 3 + 1];
		unsigned short indices_2 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices_0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices_1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices_2].pos);
		//p0��p1�x�N�g��,p0��p2�x�N�g�����v�Z (�x�N�g���̌��Z)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//���K��(������1�ɂ���)
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[indices_0].normal, normal);
		XMStoreFloat3(&vertices[indices_1].normal, normal);
		XMStoreFloat3(&vertices[indices_2].normal, normal);
	}


	// GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));

	// �S���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i]; // ���W���R�s�[
	}
	// �q���������
	vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	// ���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	// ���_1���̃f�[�^�T�C�Y
	/*vbView.StrideInBytes = sizeof(XMFLOAT3);*/
	vbView.StrideInBytes = sizeof(vertices[0]);

	//
	// 
	// 
	// 
	// 
	// 
	// 
	// 
	//SRV�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	// �V�F�[�_���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;

	// �n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�쐬
	device->CreateShaderResourceView(texBuff.Get(), &srvDesc, srvHandle);

	//1�n���h����i�߂�
	UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += incrementSize;

	//�V�F�[�_�[���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = textureResourceDesc2.Format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = textureResourceDesc2.MipLevels;

	// �n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�쐬
	device->CreateShaderResourceView(texBuff2.Get(), &srvDesc2, srvHandle);

	// CBV,SRV,UAV��1���̃T�C�Y���擾
	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// �n���h����1�i�߂�iSRV�̈ʒu�j
	srvHandle.ptr += descriptorSize * 1;

	// CBV(�R���X�^���g�o�b�t�@�r���[)�̐ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	//cbvDesc�̒l�ݒ�i�ȗ��j
	// �萔�o�b�t�@�r���[����
	device->CreateConstantBufferView(&cbvDesc, srvHandle);

	// �f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;         //��x�̕`��Ɏg���e�N�X�`����1���Ȃ̂�1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;     //�e�N�X�`�����W�X�^0��
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;





	//�萔�o�b�t�@(0)
	ComPtr < ID3D12Resource> constBuffTransform0;
	ConstBufferDataTransform* constMapTransform0 = nullptr;
	//�萔�o�b�t�@(1)
	ComPtr < ID3D12Resource> constBuffTransform1;
	ConstBufferDataTransform* constMapTransform1 = nullptr;



	////�X�P�[�����O�{��
	//XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
	////��]�p
	//XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
	////���W
	//XMFLOAT3 position = { 0.0f,0.0f,0.0f };

	////���[���h�ϊ��s��
	//XMMATRIX matWorld;
	////�P�ʍs�����
	//matWorld = XMMatrixIdentity();
	//
	//XMMATRIX matScale; //�X�P�[�����O�s��
	//XMMATRIX matRot;
	//XMMATRIX matTrans;

	//matTrans = XMMatrixTranslation(-50.0f, 0.0f, 0.0f);
	//matWorld *= matTrans;

	{
		// �q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPU�ւ̓]���p
		// ���\�[�X�ݒ�
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256�o�C�g�A���C�������g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//
		// 
		// 
		// 
		//// �萔�o�b�t�@�̐��� (0)
		//result = device->CreateCommittedResource(
		//	&cbHeapProp, // �q�[�v�ݒ�
		//	D3D12_HEAP_FLAG_NONE,
		//	&cbResourceDesc, // ���\�[�X�ݒ�
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	nullptr,
		//	IID_PPV_ARGS(&constBuffTransform0));
		//assert(SUCCEEDED(result));
		//// �萔�o�b�t�@�̃}�b�s���O
		//result = constBuffTransform0->Map(0, nullptr, (void**)&constMapTransform0); // �}�b�s���O
		//assert(SUCCEEDED(result));

		//// �萔�o�b�t�@�̐��� (1)
		//result = device->CreateCommittedResource(
		//	&cbHeapProp, // �q�[�v�ݒ�
		//	D3D12_HEAP_FLAG_NONE,
		//	&cbResourceDesc, // ���\�[�X�ݒ�
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	nullptr,
		//	IID_PPV_ARGS(&constBuffTransform1));
		//assert(SUCCEEDED(result));
		//// �萔�o�b�t�@�̃}�b�s���O
		//result = constBuffTransform1->Map(0, nullptr, (void**)&constMapTransform1); // �}�b�s���O
		//assert(SUCCEEDED(result));

		//�z����̑S�I�u�W�F�N�g�ɑ΂���
		for (int i = 0; i < _countof(object3ds); i++)
		{
			//������
			InitializeObject3d(&object3ds[i], device.Get());

			//�������牺�͐e�q�\���̃T���v��
			//�擪�ȊO�Ȃ�
			if (i > 0)
			{
				//�ЂƂO�̃I�u�W�F�N�g��e�I�u�W�F�N�g�Ƃ���
				object3ds[i].parent = &object3ds[i - 1];
				//�e�I�u�W�F�N�g��9���̑傫��
				object3ds[i].scale = { 0.9f,0.9f,0.9f };
				//�I�u�W�F�N�g�ɑ΂���Z���܂���30�x��]
				object3ds[i].rotation = { 0.0f,0.0f,XMConvertToRadians(30.0f) };

				//�e�I�u�W�F�N�g�ɑ΂���Z����-8.0���炷
				object3ds[i].position = { 0.0f,0.0f,-8.0f };
			}

		}



	}
	//�P�ʍs�����
	/*constMapTransform->mat = XMMatrixIdentity();
	constMapTransform->mat.r[0].m128_f32[0] = 2.0f / window_width;
	constMapTransform->mat.r[1].m128_f32[1] = -2.0f / window_height;
	constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
	constMapTransform->mat.r[3].m128_f32[1] = 1.0f;*/
	//���s
	//constMapTransform0->mat = XMMatrixOrthographicOffCenterLH(
	//	//���[			//�E�[    
	//	0.0f,			 window_width,

	//	//���[			//��[
	//	window_height,  0.0f,

	//	//�O�[			//���[
	//	 0.0f,			1.0f      
	//);
	//�������e�s��̌v�Z
	XMMATRIX matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		(float)window_width / window_height,
		0.1f, 1000.0f
	);



	//�r���[�ϊ��s��
	XMMATRIX matView;
	XMFLOAT3 eye(0, 0, -100);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//constMapTransform0->mat = matWorld * matView * matProjection;




// �q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPU�ւ̓]���p
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;   // 256�o�C�g�A���C�������g
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ComPtr < ID3D12Resource> constBuffMaterial;
	// �萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&cbHeapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial));
	assert(SUCCEEDED(result));

	// �萔�o�b�t�@�̃}�b�s���O
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // �}�b�s���O
	assert(SUCCEEDED(result));

	// �l���������ނƎ����I�ɓ]�������
	constMapMaterial->color = XMFLOAT4(1, 1, 1, 1);              // RGBA

	float RED = 1;
	float GREEN = 0;
	float BLUE = 0;
	constMapMaterial->color = XMFLOAT4(RED, GREEN, BLUE, 0.5f);

	// ���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	// �萔�o�b�t�@0��
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // ���
	rootParams[0].Descriptor.ShaderRegister = 0;                   // �萔�o�b�t�@�ԍ�
	rootParams[0].Descriptor.RegisterSpace = 0;                    // �f�t�H���g�l
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // �S�ẴV�F�[�_���猩����
	// �e�N�X�`�����W�X�^0��
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   //���
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;		  //�f�X�N���v�^�����W
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;              		  //�f�X�N���v�^�����W��
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;               //�S�ẴV�F�[�_���猩����
	// �萔�o�b�t�@1��
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // ���
	rootParams[2].Descriptor.ShaderRegister = 1;                   // �萔�o�b�t�@�ԍ�
	rootParams[2].Descriptor.RegisterSpace = 0;                    // �f�t�H���g�l
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // �S�ẴV�F�[�_���猩����


	// �e�N�X�`���T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���J��Ԃ��i�^�C�����O�j
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //�c�J��Ԃ��i�^�C�����O�j
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���s�J��Ԃ��i�^�C�����O�j
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //�S�ă��j�A���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;                                              //�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //�s�N�Z���V�F�[�_����̂ݎg�p�\

	ID3DBlob* vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
	ID3DBlob* psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ID3DBlob* errorBlob = nullptr; // �G���[�I�u�W�F�N�g

	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicVS.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "vs_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);
	// �G���[�Ȃ�
	if (FAILED(result))
	{
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}
	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicPS.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "ps_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);

	// �G���[�Ȃ�
	if (FAILED(result))
	{
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xyz���W(1�s�ŏ������ق������₷��)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ //�@���x�N�g��(1�s�ŏ������ق������₷��)
		  "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		  D3D12_APPEND_ALIGNED_ELEMENT,
		  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0

		},
		{ // uv���W(1�s�ŏ������ق������₷��)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};


	// �O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable = true;
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// �V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// �V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// �T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�

	//////////////////////////////////////////////////////////////
	//-------------------���X�^���C�U�̐ݒ�------------------------//
	/////////////////////////////////////////////////////////////

	//pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // �J�����O���Ȃ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;


	//�h��Ԃ������C���[�̂ǂ����������g���Ȃ�
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	//pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // ���C���[�t���[��
	pipelineDesc.RasterizerState.DepthClipEnable = true; // �[�x�N���b�s���O��L����

	// �u�����h�X�e�[�g
	//pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask
	//	= D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA�S�Ẵ`�����l����`��


	//�����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //RBGA�S�Ẵ`�����l����`��
	//���ʐݒ�(�A���t�@�l)
	blenddesc.BlendEnable = true; //�u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE; //�\�[�X�̒l��100%�g��
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO; //�f�X�g�̒l��0%�g��
	////���Z����
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD; //���Z
	//blenddesc.SrcBlend = D3D12_BLEND_ONE; //�\�[�X�̒l��100%�g��
	//blenddesc.DestBlend = D3D12_BLEND_ONE; //�f�X�g�̒l��100%�g��
	////���Z����
	//blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; //�f�X�g����\�[�X�����Z
	//blenddesc.SrcBlend = D3D12_BLEND_ONE; //�\�[�X�̒l��100%�g��
	//blenddesc.DestBlend = D3D12_BLEND_ONE;              // �f�X�g�̒l��100% �g��
	//// �F���]
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // ���Z
	//blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;    // 1.0f-�f�X�g�J���[�̒l
	//blenddesc.DestBlend = D3D12_BLEND_ZERO;             // �g��Ȃ�
	// ����������
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // ���Z
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // �\�[�X�̃A���t�@�l
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;    // 1.0f-�\�[�X�̃A���t�@�l

	//���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	//�}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���̑��̐ݒ�
	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	// ���[�g�V�O�l�`��
	ComPtr < ID3D12RootSignature> rootSignature;

	// ���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams);        //���[�g�p�����[�^��
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;


	// ���[�g�V�O�l�`���̃V���A���C�Y
	ComPtr < ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));

	// �p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature.Get();

	//�΃v�����X�e�[�g�̐���
	ComPtr < ID3D12PipelineState> pipelineStage;

	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineStage));
	assert(SUCCEEDED(result));

#pragma endregion

	float angle = 0.0f; //�J�����̉�]
	//�Q�[�����[�v
	while (true)
	{
#pragma region �E�B���h�E���b�Z�[�W����
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���P�[�V�������I��鎞��message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}


#pragma endregion

#pragma region DirectX���t���[������
		/////////////////////////////////////////////////////
		//----------DireceX���t���[�������@��������------------//
		///////////////////////////////////////////////////

		// �L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();
		// �S�L�[�̓��͏�Ԃ��擾����
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);
		// ������0�L�[��������Ă�����
		if (key[DIK_0])
		{
			OutputDebugStringA("Hit 0\n");  // �o�̓E�B���h�E�ɁuHit 0�v�ƕ\��
		}

		//�o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.�`���̕ύX
		//�����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		/*commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);*/
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		//3.��ʃN���A			R	  G	   B	A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//���ۂ��F
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		if (key[DIK_SPACE])     // �X�y�[�X�L�[��������Ă�����
		{
			//��ʃN���A�J���[�̐��l������������
			FLOAT clearColor[] = { 11.1f,0.25f, 0.5f,0.0f }; // �s���N���ۂ��F
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}


		if (key[DIK_D] || key[DIK_A])
		{
			if (key[DIK_D]) { angle += XMConvertToRadians(1.0f); }
			else if (DIK_A) { angle -= XMConvertToRadians(1.0f); }

			//angle���W�A������y���܂��ɉ�]
			eye.x = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}
		if (key[DIK_W] || key[DIK_S])
		{
			if (key[DIK_W]) { angle += XMConvertToRadians(1.0f); }
			else if (DIK_S) { angle -= XMConvertToRadians(1.0f); }

			//angle���W�A������y���܂��ɉ�]
			eye.y = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}


		for (size_t i = 0; i < _countof(object3ds); i++)
		{
			UpdateObject3d(&object3ds[i], matView, matProjection);
		}

		//���W����
		if (key[DIK_UP] || key[DIK_DOWN] || key[DIK_RIGHT] || key[DIK_LEFT])
		{
			if (key[DIK_UP]) { object3ds[0].position.y += 1.0f; }
			else if (key[DIK_DOWN]) { object3ds[0].position.y -= 1.0f; }
			if (key[DIK_RIGHT]) { object3ds[0].position.x += 1.0f; }
			else if (key[DIK_LEFT]) { object3ds[0].position.x -= 1.0f; }
		}

		if (GREEN <= 1.0f)
		{
			RED -= 0.01f;
			GREEN += 0.01f;
			BLUE += 0.01f;
		}
		constMapMaterial->color = XMFLOAT4(RED, GREEN, BLUE, 0.5f);

		if (key[DIK_R])
		{
			RED = 1.0f;
			GREEN = 0.0f;
			BLUE = 0.0f;
		}

		////��]
		//if (key[DIK_UP] || key[DIK_DOWN] || key[DIK_RIGHT] || key[DIK_LEFT]) {
		//	if (key[DIK_UP]) { position.z += 1.0f; }
		//	else if (key[DIK_DOWN]) { position.z -= 1.0f; }
		//	if (key[DIK_RIGHT]) { position.x += 1.0f; }
		//	else if (key[DIK_LEFT]) { position.x -= 1.0f; }
		//}

		//matWorld = XMMatrixIdentity();
		//matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
		//matWorld *= matScale;
		////�P�ʍs�����
		//matRot = XMMatrixIdentity();
		//matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
		//matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
		//matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
		//matWorld *= matRot;


		//matTrans = XMMatrixTranslation(position.x, position.y, position.z);
		//matWorld *= matTrans;

		////�萔�o�b�t�@�ɓ]��
		//constMapTransform0->mat = matWorld * matView * matProjection;



		/////���[���h�s��1�Ԃ̐���///
		////���[���h�ϊ��s��
		//XMMATRIX matWorld1;
		//matWorld1 = XMMatrixIdentity();
		////�e��ό`�s����v�Z
		//XMMATRIX matScale1 = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		//XMMATRIX matRot1 = XMMatrixRotationY(XM_PI / 4.0f);
		//XMMATRIX matTrans1 = XMMatrixTranslation(-20.0f, 0, 0);
		////���[���h�s�������
		//matWorld1 = matScale1 * matRot1 * matTrans1;
		////���[���h�A�r���[�A�ˉe�s����������ăV�F�[�_�[�ɓ]��
		//constMapTransform1->mat = matWorld1 * matView * matProjection;


		////�g���K�[����
		//bool �L�[����������Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂�����Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[���������u�Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂����u�Ԃ�(uint8_t �L�[�ԍ�);

		// �L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();

		// �S�L�[�̓��͏�Ԃ��擾����

		keyboard->GetDeviceState(sizeof(key), key);



		//////////////////////////////////////////////
		//-------DireceX���t���[�������@�����܂�--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region �O���t�B�b�N�X�R�}���h

		//4.�`��R�}���h��������

		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;   //�悱 �ő�1280
		viewport.Height = window_height;  //���� �ő�720
		viewport.TopLeftX = 0;  //����X
		viewport.TopLeftY = 0;  //����Y
		viewport.MinDepth = 0.0f; //�ŏ��p�x
		viewport.MaxDepth = 1.0f; //�ő�p�x
		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);

		//�V�U�[��`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0; // �؂蔲�����W��
		scissorRect.right = scissorRect.left + window_width; // �؂蔲�����W�E
		scissorRect.top = 0; // �؂蔲�����W��
		scissorRect.bottom = scissorRect.top + window_height; // �؂蔲�����W��
		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		//�p�C�v���C�����`�[�g�Ɓ�V�O�l�`���̐ݒ�R�}���h
		commandList->SetPipelineState(pipelineStage.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		// �v���~�e�B�u�`��̐ݒ�R�}���h
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // �O�p�`���X�g

		//���_�o�b�t�@�̐ݒ�R�}���h
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// �萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
		commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
		// SRV�q�[�v�̐ݒ�R�}���h
		commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
		// SRV�q�[�v�̐擪�n���h�����擾�iSRV���w���Ă���͂��j
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		// SRV�q�[�v�̐擪�ɂ���SRV�����[�g�p�����[�^1�Ԃɐݒ�
		commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);


		//2���ڂ��w�������悤�ɂ���SRV�̃n���h�������[�g�p�����[�^1�Ԃɐݒ�
		srvGpuHandle.ptr += incrementSize;
		commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetIndexBuffer(&ibView);

		//// 0�Ԓ萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
		//commandList->SetGraphicsRootConstantBufferView(2, constBuffTransform0->GetGPUVirtualAddress());
		//// �`��R�}���h
		//commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
		//
		//// 1�Ԓ萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
		//commandList->SetGraphicsRootConstantBufferView(2, constBuffTransform1->GetGPUVirtualAddress());
		//// �`��R�}���h
		//commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		//�S�I�u�W�F�N�g�ɂ��ď���
		for (int i = 0; i < _countof(object3ds); i++)
		{
			DrawObject3d(&object3ds[i], commandList.Get(), vbView, ibView, _countof(indices));
		}



		//4.�`��R�}���h�����܂�

#pragma endregion

#pragma region ��ʓ���ւ�


		//5.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//�\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		//���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(1, commandLists);

		//��ʂɕ\������o�b�t�@���t���b�v(���\�̓���ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		//�R�}���h�̎��s������҂�
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			if (event != 0)
			{
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
		}

		//�L���[���N���A
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//�ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));

#pragma endregion
	}
#pragma region  WindowsAPI��n��

	//�����N���X�͎g��Ȃ��̂œo�^����������
	UnregisterClass(w.lpszClassName, w.hInstance);

#pragma endregion
	return 0;
}